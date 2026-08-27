package storage

import (
	"database/sql"
	"fmt"
	"time"

	"im/internal/services/group-service/model"
	"im/internal/shared/config"

	_ "github.com/go-sql-driver/mysql"
)

// GroupStorage 群组存储接口
type GroupStorage interface {
	CreateGroup(name, ownerUID string) (string, error)
	GetGroup(groupID string) (*model.Group, error)
	UpdateGroupName(groupID, newName string) error
	DisbandGroup(groupID string) error
	JoinGroup(groupID, uid string) error
	LeaveGroup(groupID, uid string) error
	GetUserGroups(userUID string) ([]*model.Group, error)
	GetGroupMembers(groupID string) ([]*model.GroupMember, error)
	GetUserRoleInGroup(groupID, userUID string) (string, error)
	SetGroupMemberRole(groupID, userUID, role string) error
	SetGroupMemberNickname(groupID, userUID, nickname string) error
	SetGroupMemberRemark(groupID, userUID, remark string) error
	SetGroupMemberDND(groupID, userUID string, dnd bool) error
	GetGroupMemberDND(groupID, userUID string) (bool, error)
	SetGroupMemberMute(groupID, userUID string, muted bool) error
	InsertInviteRequest(groupID, inviterUID, inviteeUID string) error
	GetPendingInviteRequests(groupID string) ([]*model.GroupInviteRequest, error)
	ApproveInviteRequest(requestID string) error
	RejectInviteRequest(requestID string) error
	HasPendingInvite(groupID, userUID string) (bool, error)
	GetGroupAdminsAndOwner(groupID string) ([]string, error)
	Close() error
}

// MySQLGroupStorage MySQL群组存储实现
type MySQLGroupStorage struct {
	db *sql.DB
}

// NewGroupStorage 创建群组存储实例
func NewGroupStorage(cfg config.DatabaseConfig) (GroupStorage, error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s?charset=utf8mb4&parseTime=True&loc=Local",
		cfg.Username, cfg.Password, cfg.Host, cfg.Port, cfg.Database)

	db, err := sql.Open("mysql", dsn)
	if err != nil {
		return nil, fmt.Errorf("连接数据库失败: %v", err)
	}

	if err := db.Ping(); err != nil {
		return nil, fmt.Errorf("数据库连接测试失败: %v", err)
	}

	storage := &MySQLGroupStorage{db: db}
	if err := storage.initTables(); err != nil {
		return nil, fmt.Errorf("初始化表失败: %v", err)
	}

	return storage, nil
}

// initTables 初始化表
func (s *MySQLGroupStorage) initTables() error {
	// 创建群组表
	createGroupsTableSQL := `
	CREATE TABLE IF NOT EXISTS ` + "`groups`" + ` (
		id BIGINT AUTO_INCREMENT PRIMARY KEY,
		group_id VARCHAR(50) UNIQUE NOT NULL,
		name VARCHAR(100) NOT NULL,
		description TEXT,
		owner_uid VARCHAR(50) NOT NULL,
		created_at BIGINT NOT NULL,
		updated_at BIGINT NOT NULL,
		INDEX idx_group_id (group_id),
		INDEX idx_owner (owner_uid),
		INDEX idx_created (created_at)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;`

	// 创建群组成员表
	createGroupMembersTableSQL := `
	CREATE TABLE IF NOT EXISTS group_members (
		group_id VARCHAR(50) NOT NULL,
		uid VARCHAR(50) NOT NULL,
		nickname VARCHAR(100),
		role VARCHAR(20) DEFAULT 'member',
		join_time BIGINT NOT NULL,
		remark VARCHAR(255) DEFAULT NULL,
		dnd TINYINT(1) DEFAULT 0,
		mute_until BIGINT DEFAULT 0,
		PRIMARY KEY (group_id, uid),
		INDEX idx_group (group_id),
		INDEX idx_uid (uid),
		FOREIGN KEY (group_id) REFERENCES ` + "`groups`" + `(group_id) ON DELETE CASCADE
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;`

	// 创建群组邀请请求表
	createGroupInviteRequestsTableSQL := `
	CREATE TABLE IF NOT EXISTS group_invite_requests (
		id BIGINT AUTO_INCREMENT PRIMARY KEY,
		group_id VARCHAR(50) NOT NULL,
		inviter_uid VARCHAR(50) NOT NULL,
		invitee_uid VARCHAR(50) NOT NULL,
		status ENUM('pending','approved','rejected') DEFAULT 'pending',
		created_at BIGINT NOT NULL,
		updated_at BIGINT NOT NULL,
		INDEX idx_group (group_id),
		INDEX idx_invitee (invitee_uid)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;`

	if _, err := s.db.Exec(createGroupsTableSQL); err != nil {
		return fmt.Errorf("创建群组表失败: %v", err)
	}

	if _, err := s.db.Exec(createGroupMembersTableSQL); err != nil {
		return fmt.Errorf("创建群组成员表失败: %v", err)
	}

	if _, err := s.db.Exec(createGroupInviteRequestsTableSQL); err != nil {
		return fmt.Errorf("创建群组邀请请求表失败: %v", err)
	}

	return nil
}

// CreateGroup 创建群组
func (s *MySQLGroupStorage) CreateGroup(name, ownerUID string) (string, error) {
	tx, err := s.db.Begin()
	if err != nil {
		return "", fmt.Errorf("开始事务失败: %v", err)
	}
	defer tx.Rollback()

	now := time.Now().Unix()

	// 获取当前最大群组ID
	var maxGroupID int64
	err = tx.QueryRow("SELECT COALESCE(MAX(CAST(group_id AS UNSIGNED)), 0) FROM `groups` WHERE group_id REGEXP '^[0-9]+$'").Scan(&maxGroupID)
	if err != nil {
		return "", fmt.Errorf("获取最大群组ID失败: %v", err)
	}

	// 生成新的群组ID
	newGroupID := maxGroupID + 1
	groupID := fmt.Sprintf("%d", newGroupID)

	// 插入群组记录
	_, err = tx.Exec("INSERT INTO `groups` (group_id, name, description, owner_uid, created_at, updated_at) VALUES (?, ?, '', ?, ?, ?)",
		groupID, name, ownerUID, now, now)
	if err != nil {
		return "", fmt.Errorf("创建群组失败: %v", err)
	}
	// 添加群主为成员
	_, err = tx.Exec(`
		INSERT INTO group_members (group_id, uid, nickname, role, join_time)
		VALUES (?, ?, ?, ?, ?)
	`, groupID, ownerUID, "", "owner", now)
	if err != nil {
		return "", fmt.Errorf("添加群主失败: %v", err)
	}

	if err := tx.Commit(); err != nil {
		return "", fmt.Errorf("提交事务失败: %v", err)
	}

	return groupID, nil
}

// GetGroup 获取群组信息
func (s *MySQLGroupStorage) GetGroup(groupID string) (*model.Group, error) {
	query := "SELECT group_id, name, owner_uid, created_at, updated_at FROM `groups` WHERE group_id = ?"
	row := s.db.QueryRow(query, groupID)

	group := &model.Group{}
	err := row.Scan(&group.GroupID, &group.Name, &group.OwnerUID, &group.CreatedAt, &group.UpdatedAt)
	if err != nil {
		if err == sql.ErrNoRows {
			return nil, fmt.Errorf("群组不存在")
		}
		return nil, fmt.Errorf("查询群组失败: %v", err)
	}

	return group, nil
}

// UpdateGroupName 更新群组名称
func (s *MySQLGroupStorage) UpdateGroupName(groupID, newName string) error {
	query := "UPDATE `groups` SET name = ? WHERE group_id = ?"
	_, err := s.db.Exec(query, newName, groupID)
	if err != nil {
		return fmt.Errorf("更新群组名称失败: %v", err)
	}
	return nil
}

// DisbandGroup 解散群组
func (s *MySQLGroupStorage) DisbandGroup(groupID string) error {
	// 开始事务
	tx, err := s.db.Begin()
	if err != nil {
		return fmt.Errorf("开始事务失败: %v", err)
	}
	defer tx.Rollback()

	// 删除群组成员
	_, err = tx.Exec(`DELETE FROM group_members WHERE group_id = ?`, groupID)
	if err != nil {
		return fmt.Errorf("删除群组成员失败: %v", err)
	}

	// 删除群组邀请请求
	_, err = tx.Exec(`DELETE FROM group_invite_requests WHERE group_id = ?`, groupID)
	if err != nil {
		return fmt.Errorf("删除群组邀请请求失败: %v", err)
	}

	// 删除群组
	_, err = tx.Exec("DELETE FROM `groups` WHERE group_id = ?", groupID)
	if err != nil {
		return fmt.Errorf("删除群组失败: %v", err)
	}

	// 提交事务
	if err := tx.Commit(); err != nil {
		return fmt.Errorf("提交事务失败: %v", err)
	}

	return nil
}

// JoinGroup 加入群组
func (s *MySQLGroupStorage) JoinGroup(groupID, uid string) error {
	// 检查是否已经是成员
	var count int
	err := s.db.QueryRow(`
		SELECT COUNT(*) FROM group_members WHERE group_id = ? AND uid = ?
	`, groupID, uid).Scan(&count)
	if err != nil {
		return fmt.Errorf("检查成员状态失败: %v", err)
	}
	if count > 0 {
		return fmt.Errorf("已经是群组成员")
	}

	// 检查群组是否存在
	err = s.db.QueryRow("SELECT COUNT(*) FROM `groups` WHERE group_id = ?", groupID).Scan(&count)
	if err != nil {
		return fmt.Errorf("检查群组存在失败: %v", err)
	}
	if count == 0 {
		return fmt.Errorf("群组不存在")
	}

	// 添加成员
	_, err = s.db.Exec(`
		INSERT INTO group_members (group_id, uid, nickname, role, join_time)
		VALUES (?, ?, ?, ?, ?)
	`, groupID, uid, "", "member", time.Now().Unix())
	if err != nil {
		return fmt.Errorf("加入群组失败: %v", err)
	}

	// 更新群组更新时间
	_, err = s.db.Exec("UPDATE `groups` SET updated_at = ? WHERE group_id = ?", time.Now().Unix(), groupID)
	if err != nil {
		return fmt.Errorf("更新群组时间失败: %v", err)
	}

	return nil
}

// LeaveGroup 离开群组
func (s *MySQLGroupStorage) LeaveGroup(groupID, uid string) error {
	query := `DELETE FROM group_members WHERE group_id = ? AND uid = ?`
	_, err := s.db.Exec(query, groupID, uid)
	if err != nil {
		return fmt.Errorf("离开群组失败: %v", err)
	}
	return nil
}

// GetUserGroups 获取用户加入的群组
func (s *MySQLGroupStorage) GetUserGroups(userUID string) ([]*model.Group, error) {
	query := `
		SELECT g.group_id, g.name, g.owner_uid, g.created_at, g.updated_at, gm.remark 
		FROM ` + "`groups`" + ` g 
		INNER JOIN group_members gm ON g.group_id = gm.group_id 
		WHERE gm.uid = ? 
		ORDER BY gm.join_time DESC`

	rows, err := s.db.Query(query, userUID)
	if err != nil {
		return nil, fmt.Errorf("查询用户群组失败: %v", err)
	}
	defer rows.Close()

	var groups []*model.Group
	for rows.Next() {
		group := &model.Group{}
		var remark sql.NullString
		err := rows.Scan(&group.GroupID, &group.Name, &group.OwnerUID, &group.CreatedAt, &group.UpdatedAt, &remark)
		if err != nil {
			return nil, fmt.Errorf("扫描群组信息失败: %v", err)
		}
		// 设置群组备注
		if remark.Valid {
			group.Remark = remark.String
		} else {
			group.Remark = ""
		}
		groups = append(groups, group)
	}

	return groups, nil
}

// GetGroupMembers 获取群组成员
func (s *MySQLGroupStorage) GetGroupMembers(groupID string) ([]*model.GroupMember, error) {
	query := `
		SELECT group_id, uid, role, nickname, remark, dnd, mute_until, join_time 
		FROM group_members 
		WHERE group_id = ? 
		ORDER BY 
			CASE role 
				WHEN 'owner' THEN 1 
				WHEN 'admin' THEN 2 
				WHEN 'member' THEN 3 
			END, join_time ASC`

	rows, err := s.db.Query(query, groupID)
	if err != nil {
		return nil, fmt.Errorf("查询群组成员失败: %v", err)
	}
	defer rows.Close()

	var members []*model.GroupMember
	for rows.Next() {
		member := &model.GroupMember{}
		var muteUntil int64
		var remark sql.NullString
		err := rows.Scan(&member.GroupID, &member.UID, &member.Role,
			&member.Nickname, &remark, &member.DND, &muteUntil, &member.JoinTime)
		if err != nil {
			return nil, fmt.Errorf("扫描群组成员失败: %v", err)
		}
		// 处理可能为NULL的remark字段
		if remark.Valid {
			member.Remark = remark.String
		} else {
			member.Remark = ""
		}
		// 设置禁言状态：如果mute_until大于当前时间，说明用户被禁言
		currentTime := time.Now().Unix()
		member.Muted = muteUntil > currentTime
		fmt.Printf("DEBUG: 用户 %s 的禁言状态计算: mute_until=%d, current_time=%d, muted=%t\n",
			member.UID, muteUntil, currentTime, member.Muted)
		members = append(members, member)
	}

	return members, nil
}

// GetUserRoleInGroup 获取用户在群组中的角色
func (s *MySQLGroupStorage) GetUserRoleInGroup(groupID, userUID string) (string, error) {
	query := `SELECT role FROM group_members WHERE group_id = ? AND uid = ?`
	row := s.db.QueryRow(query, groupID, userUID)

	var role string
	err := row.Scan(&role)
	if err != nil {
		if err == sql.ErrNoRows {
			return "", fmt.Errorf("用户不是群组成员")
		}
		return "", fmt.Errorf("查询用户角色失败: %v", err)
	}

	return role, nil
}

// SetGroupMemberRole 设置群组成员角色
func (s *MySQLGroupStorage) SetGroupMemberRole(groupID, userUID, role string) error {
	query := `UPDATE group_members SET role = ? WHERE group_id = ? AND uid = ?`
	_, err := s.db.Exec(query, role, groupID, userUID)
	if err != nil {
		return fmt.Errorf("设置群组成员角色失败: %v", err)
	}
	return nil
}

// SetGroupMemberNickname 设置群成员昵称
func (s *MySQLGroupStorage) SetGroupMemberNickname(groupID, userUID, nickname string) error {
	query := `UPDATE group_members SET nickname = ? WHERE group_id = ? AND uid = ?`
	_, err := s.db.Exec(query, nickname, groupID, userUID)
	if err != nil {
		return fmt.Errorf("设置群成员昵称失败: %v", err)
	}
	return nil
}

// SetGroupMemberRemark 设置群成员备注
func (s *MySQLGroupStorage) SetGroupMemberRemark(groupID, userUID, remark string) error {
	query := `UPDATE group_members SET remark = ? WHERE group_id = ? AND uid = ?`
	_, err := s.db.Exec(query, remark, groupID, userUID)
	if err != nil {
		return fmt.Errorf("设置群成员备注失败: %v", err)
	}
	return nil
}

// SetGroupMemberDND 设置群成员免打扰
func (s *MySQLGroupStorage) SetGroupMemberDND(groupID, userUID string, dnd bool) error {
	query := `UPDATE group_members SET dnd = ? WHERE group_id = ? AND uid = ?`
	_, err := s.db.Exec(query, dnd, groupID, userUID)
	if err != nil {
		return fmt.Errorf("设置群成员免打扰失败: %v", err)
	}
	return nil
}

// GetGroupMemberDND 获取群成员免打扰状态
func (s *MySQLGroupStorage) GetGroupMemberDND(groupID, userUID string) (bool, error) {
	query := `SELECT dnd FROM group_members WHERE group_id = ? AND uid = ?`
	var dnd bool
	err := s.db.QueryRow(query, groupID, userUID).Scan(&dnd)
	if err != nil {
		return false, fmt.Errorf("获取群成员免打扰状态失败: %v", err)
	}
	return dnd, nil
}

// SetGroupMemberMute 设置群成员禁言
func (s *MySQLGroupStorage) SetGroupMemberMute(groupID, userUID string, muted bool) error {
	var muteUntil int64
	if muted {
		// 如果设置为禁言，设置一个很远的未来时间（比如一年后）
		muteUntil = time.Now().Unix() + 365*24*60*60
	} else {
		// 如果取消禁言，设置为0
		muteUntil = 0
	}
	query := `UPDATE group_members SET mute_until = ? WHERE group_id = ? AND uid = ?`
	_, err := s.db.Exec(query, muteUntil, groupID, userUID)
	if err != nil {
		return fmt.Errorf("设置群成员禁言失败: %v", err)
	}
	return nil
}

// InsertInviteRequest 插入邀请请求
func (s *MySQLGroupStorage) InsertInviteRequest(groupID, inviterUID, inviteeUID string) error {
	now := time.Now().Unix()
	_, err := s.db.Exec(`INSERT INTO group_invite_requests (group_id, inviter_uid, invitee_uid, status, created_at, updated_at) VALUES (?, ?, ?, 'pending', ?, ?)`,
		groupID, inviterUID, inviteeUID, now, now)
	return err
}

// GetPendingInviteRequests 获取待处理的邀请请求
func (s *MySQLGroupStorage) GetPendingInviteRequests(groupID string) ([]*model.GroupInviteRequest, error) {
	query := `
		SELECT id, group_id, inviter_uid, invitee_uid, status, created_at, updated_at 
		FROM group_invite_requests 
		WHERE group_id = ? AND status = 'pending' 
		ORDER BY created_at DESC`

	rows, err := s.db.Query(query, groupID)
	if err != nil {
		return nil, fmt.Errorf("查询邀请请求失败: %v", err)
	}
	defer rows.Close()

	var requests []*model.GroupInviteRequest
	for rows.Next() {
		request := &model.GroupInviteRequest{}
		err := rows.Scan(&request.ID, &request.GroupID, &request.InviterUID, &request.InviteeUID,
			&request.Status, &request.CreatedAt, &request.UpdatedAt)
		if err != nil {
			return nil, fmt.Errorf("扫描邀请请求失败: %v", err)
		}
		requests = append(requests, request)
	}

	return requests, nil
}

// ApproveInviteRequest 批准邀请请求
func (s *MySQLGroupStorage) ApproveInviteRequest(requestID string) error {
	query := `UPDATE group_invite_requests SET status = 'approved' WHERE id = ?`
	_, err := s.db.Exec(query, requestID)
	if err != nil {
		return fmt.Errorf("批准邀请请求失败: %v", err)
	}
	return nil
}

// RejectInviteRequest 拒绝邀请请求
func (s *MySQLGroupStorage) RejectInviteRequest(requestID string) error {
	query := `UPDATE group_invite_requests SET status = 'rejected' WHERE id = ?`
	_, err := s.db.Exec(query, requestID)
	if err != nil {
		return fmt.Errorf("拒绝邀请请求失败: %v", err)
	}
	return nil
}

// HasPendingInvite 检查是否有待处理的邀请
func (s *MySQLGroupStorage) HasPendingInvite(groupID, userUID string) (bool, error) {
	query := `SELECT COUNT(*) FROM group_invite_requests WHERE group_id = ? AND invitee_uid = ? AND status = 'pending'`
	row := s.db.QueryRow(query, groupID, userUID)

	var count int
	err := row.Scan(&count)
	if err != nil {
		return false, fmt.Errorf("检查待处理邀请失败: %v", err)
	}

	return count > 0, nil
}

// GetGroupAdminsAndOwner 获取群组管理员和群主
func (s *MySQLGroupStorage) GetGroupAdminsAndOwner(groupID string) ([]string, error) {
	query := `SELECT uid FROM group_members WHERE group_id = ? AND role IN ('owner', 'admin')`
	rows, err := s.db.Query(query, groupID)
	if err != nil {
		return nil, fmt.Errorf("查询群组管理员失败: %v", err)
	}
	defer rows.Close()

	var admins []string
	for rows.Next() {
		var uid string
		if err := rows.Scan(&uid); err != nil {
			return nil, fmt.Errorf("扫描管理员UID失败: %v", err)
		}
		admins = append(admins, uid)
	}

	return admins, nil
}

// Close 关闭数据库连接
func (s *MySQLGroupStorage) Close() error {
	return s.db.Close()
}
