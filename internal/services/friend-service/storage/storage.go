package storage

import (
	"database/sql"
	"fmt"
	"time"

	"im/internal/shared/config"

	_ "github.com/go-sql-driver/mysql"
)

// FriendStorage 好友存储接口
type FriendStorage interface {
	AddFriend(userUID, friendUID string) error
	DeleteFriend(userUID, friendUID string) error
	GetFriends(userUID string) ([]string, error)
	SetFriendRemark(userUID, friendUID, remark string) error
	GetFriendRemark(userUID, friendUID string) (string, error)
	SetFriendDND(userUID, friendUID string, dnd bool) error
	GetFriendDND(userUID, friendUID string) (bool, error)
	AddFriendRequest(fromUID, toUID, message string) error
	GetFriendRequests(userUID string) (map[string]string, error)
	HandleFriendRequest(fromUID, toUID string, accept bool) error
	Close() error
}

// MySQLFriendStorage MySQL好友存储实现
type MySQLFriendStorage struct {
	db *sql.DB
}

// NewFriendStorage 创建好友存储实例
func NewFriendStorage(cfg config.DatabaseConfig) (FriendStorage, error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s?charset=utf8mb4&parseTime=True&loc=Local",
		cfg.Username, cfg.Password, cfg.Host, cfg.Port, cfg.Database)

	db, err := sql.Open("mysql", dsn)
	if err != nil {
		return nil, fmt.Errorf("连接数据库失败: %v", err)
	}

	if err := db.Ping(); err != nil {
		return nil, fmt.Errorf("数据库连接测试失败: %v", err)
	}

	storage := &MySQLFriendStorage{db: db}
	if err := storage.initTables(); err != nil {
		return nil, fmt.Errorf("初始化表失败: %v", err)
	}

	return storage, nil
}

// initTables 初始化表
func (s *MySQLFriendStorage) initTables() error {
	// 旧实现：friendships 与 friend_requests
	createFriendshipsTableSQL := `
	CREATE TABLE IF NOT EXISTS friendships (
		id BIGINT AUTO_INCREMENT PRIMARY KEY,
		user_id VARCHAR(64) NOT NULL,
		friend_id VARCHAR(64) NOT NULL,
		remark VARCHAR(128) DEFAULT '',
		dnd BOOLEAN DEFAULT FALSE,
		created_at BIGINT NOT NULL,
		updated_at BIGINT NOT NULL,
		UNIQUE KEY unique_friendship (user_id, friend_id),
		INDEX idx_user_id (user_id),
		INDEX idx_friend_id (friend_id)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;`

	createFriendRequestsTableSQL := `
	CREATE TABLE IF NOT EXISTS friend_requests (
		id BIGINT AUTO_INCREMENT PRIMARY KEY,
		from_user_id VARCHAR(64) NOT NULL,
		to_user_id VARCHAR(64) NOT NULL,
		verify_msg VARCHAR(255) DEFAULT '',
		status ENUM('pending', 'accepted', 'rejected') DEFAULT 'pending',
		created_at BIGINT NOT NULL,
		updated_at BIGINT NOT NULL,
		UNIQUE KEY unique_request (from_user_id, to_user_id),
		INDEX idx_from_user_id (from_user_id),
		INDEX idx_to_user_id (to_user_id),
		INDEX idx_status (status)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;`

	if _, err := s.db.Exec(createFriendshipsTableSQL); err != nil {
		return fmt.Errorf("创建好友关系表失败: %v", err)
	}

	if _, err := s.db.Exec(createFriendRequestsTableSQL); err != nil {
		return fmt.Errorf("创建好友请求表失败: %v", err)
	}

	return nil
}

// AddFriend 添加好友
func (s *MySQLFriendStorage) AddFriend(userUID, friendUID string) error {
	now := time.Now().Unix()
	// 添加双向好友关系
	query := `INSERT INTO friendships (user_id, friend_id, created_at, updated_at) VALUES (?, ?, ?, ?), (?, ?, ?, ?)`
	_, err := s.db.Exec(query, userUID, friendUID, now, now, friendUID, userUID, now, now)
	if err != nil {
		return fmt.Errorf("添加好友关系失败: %v", err)
	}
	return nil
}

// DeleteFriend 删除好友
func (s *MySQLFriendStorage) DeleteFriend(userUID, friendUID string) error {
	query := `DELETE FROM friendships WHERE (user_id = ? AND friend_id = ?) OR (user_id = ? AND friend_id = ?)`
	_, err := s.db.Exec(query, userUID, friendUID, friendUID, userUID)
	if err != nil {
		return fmt.Errorf("删除好友关系失败: %v", err)
	}
	return nil
}

// GetFriends 获取好友列表
func (s *MySQLFriendStorage) GetFriends(userUID string) ([]string, error) {
	query := `SELECT friend_id FROM friendships WHERE user_id = ? ORDER BY created_at DESC`
	rows, err := s.db.Query(query, userUID)
	if err != nil {
		return nil, fmt.Errorf("查询好友列表失败: %v", err)
	}
	defer rows.Close()

	var friends []string
	for rows.Next() {
		var friendUID string
		if err := rows.Scan(&friendUID); err != nil {
			return nil, fmt.Errorf("扫描好友UID失败: %v", err)
		}
		friends = append(friends, friendUID)
	}

	return friends, nil
}

// SetFriendRemark 设置好友备注
func (s *MySQLFriendStorage) SetFriendRemark(userUID, friendUID, remark string) error {
	query := `UPDATE friendships SET remark = ? WHERE user_id = ? AND friend_id = ?`
	_, err := s.db.Exec(query, remark, userUID, friendUID)
	if err != nil {
		return fmt.Errorf("设置好友备注失败: %v", err)
	}
	return nil
}

// GetFriendRemark 获取好友备注
func (s *MySQLFriendStorage) GetFriendRemark(userUID, friendUID string) (string, error) {
	query := `SELECT remark FROM friendships WHERE user_id = ? AND friend_id = ?`
	row := s.db.QueryRow(query, userUID, friendUID)

	var remark string
	err := row.Scan(&remark)
	if err != nil {
		if err == sql.ErrNoRows {
			return "", nil
		}
		return "", fmt.Errorf("获取好友备注失败: %v", err)
	}

	return remark, nil
}

// SetFriendDND 设置好友免打扰
func (s *MySQLFriendStorage) SetFriendDND(userUID, friendUID string, dnd bool) error {
	query := `UPDATE friendships SET dnd = ? WHERE user_id = ? AND friend_id = ?`
	_, err := s.db.Exec(query, dnd, userUID, friendUID)
	if err != nil {
		return fmt.Errorf("设置好友免打扰失败: %v", err)
	}
	return nil
}

// GetFriendDND 获取好友免打扰状态
func (s *MySQLFriendStorage) GetFriendDND(userUID, friendUID string) (bool, error) {
	query := `SELECT dnd FROM friendships WHERE user_id = ? AND friend_id = ?`
	row := s.db.QueryRow(query, userUID, friendUID)

	var dnd bool
	err := row.Scan(&dnd)
	if err != nil {
		if err == sql.ErrNoRows {
			return false, nil
		}
		return false, fmt.Errorf("获取好友免打扰状态失败: %v", err)
	}

	return dnd, nil
}

// AddFriendRequest 添加好友请求
func (s *MySQLFriendStorage) AddFriendRequest(fromUID, toUID, message string) error {
	now := time.Now().Unix()
	query := `INSERT INTO friend_requests (from_user_id, to_user_id, verify_msg, created_at, updated_at) VALUES (?, ?, ?, ?, ?)`
	_, err := s.db.Exec(query, fromUID, toUID, message, now, now)
	if err != nil {
		return fmt.Errorf("添加好友请求失败: %v", err)
	}
	return nil
}

// GetFriendRequests 获取好友请求列表
func (s *MySQLFriendStorage) GetFriendRequests(userUID string) (map[string]string, error) {
	query := `SELECT from_user_id, verify_msg FROM friend_requests WHERE to_user_id = ? AND status = 'pending' ORDER BY created_at DESC`
	rows, err := s.db.Query(query, userUID)
	if err != nil {
		return nil, fmt.Errorf("查询好友请求失败: %v", err)
	}
	defer rows.Close()

	requests := make(map[string]string)
	for rows.Next() {
		var fromUID, message string
		if err := rows.Scan(&fromUID, &message); err != nil {
			return nil, fmt.Errorf("扫描好友请求失败: %v", err)
		}
		requests[fromUID] = message
	}

	return requests, nil
}

// HandleFriendRequest 处理好友请求
func (s *MySQLFriendStorage) HandleFriendRequest(fromUID, toUID string, accept bool) error {
	// 更新请求状态
	status := "rejected"
	if accept {
		status = "accepted"
	}

	query := `UPDATE friend_requests SET status = ? WHERE from_user_id = ? AND to_user_id = ?`
	_, err := s.db.Exec(query, status, fromUID, toUID)
	if err != nil {
		return fmt.Errorf("更新好友请求状态失败: %v", err)
	}

	// 如果接受请求，添加好友关系
	if accept {
		if err := s.AddFriend(fromUID, toUID); err != nil {
			return fmt.Errorf("添加好友关系失败: %v", err)
		}
	}

	return nil
}

// Close 关闭数据库连接
func (s *MySQLFriendStorage) Close() error {
	return s.db.Close()
}
