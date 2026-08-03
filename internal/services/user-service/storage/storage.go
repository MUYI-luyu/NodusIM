package storage

import (
	"database/sql"
	"fmt"
	"time"

	"im/internal/services/user-service/model"
	"im/internal/services/user-service/utils"
	"im/internal/shared/database"

	_ "github.com/go-sql-driver/mysql"
)

// UserStorage 用户存储接口
type UserStorage interface {
	CreateUser(username, password, email string) (string, error)
	GetUserByUID(uid string) (*model.User, error)
	GetUserByEmail(email string) (*model.User, error)
	CheckPassword(uid, password string) (bool, error)
	UpdatePassword(uid, password string) error
	UpdateUsername(uid, username string) error
	DeleteUser(uid string) error
	Close() error
}

// MySQLUserStorage MySQL用户存储实现
type MySQLUserStorage struct {
	dbManager *database.Manager
}

// NewUserStorage 创建用户存储实例
func NewUserStorage(dbManager *database.Manager) (UserStorage, error) {
	storage := &MySQLUserStorage{dbManager: dbManager}
	if err := storage.initTables(); err != nil {
		return nil, fmt.Errorf("初始化表失败: %v", err)
	}

	return storage, nil
}

// initTables 初始化表
func (s *MySQLUserStorage) initTables() error {
	createTableSQL := `
	CREATE TABLE IF NOT EXISTS users (
		id BIGINT AUTO_INCREMENT PRIMARY KEY,
		uid VARCHAR(64) UNIQUE NOT NULL,
		username VARCHAR(64) NOT NULL,
		password VARCHAR(255) NOT NULL,
		email VARCHAR(128) UNIQUE NOT NULL,
		created_at BIGINT NOT NULL,
		updated_at BIGINT NOT NULL,
		INDEX idx_uid (uid),
		INDEX idx_email (email)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;`

	_, err := s.dbManager.GetMySQL().Exec(createTableSQL)
	return err
}

// CreateUser 创建用户
func (s *MySQLUserStorage) CreateUser(username, password, email string) (string, error) {
	// 先加密密码
	hash, err := utils.HashPassword(password)
	if err != nil {
		return "", fmt.Errorf("密码加密失败: %v", err)
	}

	// 使用事务确保原子性
	tx, err := s.dbManager.GetMySQL().Begin()
	if err != nil {
		return "", fmt.Errorf("开启事务失败: %v", err)
	}
	defer tx.Rollback() // 如果提交失败，回滚事务

	// 获取当前最大UID
	var maxUID int64
	err = tx.QueryRow("SELECT COALESCE(MAX(CAST(uid AS UNSIGNED)), 0) FROM users WHERE uid REGEXP '^[0-9]+$'").Scan(&maxUID)
	if err != nil {
		return "", fmt.Errorf("获取最大UID失败: %v", err)
	}

	// 生成新的UID
	newUID := maxUID + 1
	uid := fmt.Sprintf("%d", newUID)

	now := time.Now().Unix()

	// 插入用户记录
	query := `INSERT INTO users (uid, username, password, email, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?)`
	_, err = tx.Exec(query, uid, username, string(hash), email, now, now)
	if err != nil {
		return "", fmt.Errorf("插入用户失败: %v", err)
	}

	// 提交事务
	if err := tx.Commit(); err != nil {
		return "", fmt.Errorf("提交事务失败: %v", err)
	}

	return uid, nil
}

// GetUserByUID 根据UID获取用户
func (s *MySQLUserStorage) GetUserByUID(uid string) (*model.User, error) {
	query := `SELECT uid, username, password, email, created_at, updated_at FROM users WHERE uid = ?`
	row := s.dbManager.GetMySQL().QueryRow(query, uid)

	user := &model.User{}
	err := row.Scan(&user.UID, &user.Username, &user.Password, &user.Email, &user.CreatedAt, &user.UpdatedAt)
	if err != nil {
		if err == sql.ErrNoRows {
			return nil, fmt.Errorf("用户不存在")
		}
		return nil, fmt.Errorf("查询用户失败: %v", err)
	}

	return user, nil
}

// GetUserByEmail 根据邮箱获取用户
func (s *MySQLUserStorage) GetUserByEmail(email string) (*model.User, error) {
	query := `SELECT uid, username, password, email, created_at, updated_at FROM users WHERE email = ?`
	row := s.dbManager.GetMySQL().QueryRow(query, email)

	user := &model.User{}
	err := row.Scan(&user.UID, &user.Username, &user.Password, &user.Email, &user.CreatedAt, &user.UpdatedAt)
	if err != nil {
		if err == sql.ErrNoRows {
			return nil, fmt.Errorf("用户不存在")
		}
		return nil, fmt.Errorf("查询用户失败: %v", err)
	}

	return user, nil
}

// CheckPassword 检查密码
func (s *MySQLUserStorage) CheckPassword(uid, password string) (bool, error) {
	user, err := s.GetUserByUID(uid)
	if err != nil {
		return false, err
	}

	return utils.CheckPassword(password, user.Password), nil
}

// UpdatePassword 更新密码
func (s *MySQLUserStorage) UpdatePassword(uid, password string) error {
	hashedPassword, err := utils.HashPassword(password)
	if err != nil {
		return fmt.Errorf("密码加密失败: %v", err)
	}

	query := `UPDATE users SET password = ? WHERE uid = ?`
	_, err = s.dbManager.GetMySQL().Exec(query, hashedPassword, uid)
	if err != nil {
		return fmt.Errorf("更新密码失败: %v", err)
	}

	return nil
}

// UpdateUsername 更新用户名
func (s *MySQLUserStorage) UpdateUsername(uid, username string) error {
	query := `UPDATE users SET username = ? WHERE uid = ?`
	_, err := s.dbManager.GetMySQL().Exec(query, username, uid)
	if err != nil {
		return fmt.Errorf("更新用户名失败: %v", err)
	}

	return nil
}

// DeleteUser 删除用户
func (s *MySQLUserStorage) DeleteUser(uid string) error {
	query := `DELETE FROM users WHERE uid = ?`
	_, err := s.dbManager.GetMySQL().Exec(query, uid)
	if err != nil {
		return fmt.Errorf("删除用户失败: %v", err)
	}

	return nil
}

// Close 关闭数据库连接
func (s *MySQLUserStorage) Close() error {
	// 数据库连接由管理器统一管理，这里不需要关闭
	return nil
}
