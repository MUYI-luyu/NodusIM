package service

import (
	"fmt"

	"im/internal/services/group-service/model"
	"im/internal/services/group-service/storage"
	"im/internal/shared/logger"
)

// GroupService 群组服务
type GroupService struct {
	storage storage.GroupStorage
	logger  *logger.Logger
}

// NewGroupService 创建群组服务实例
func NewGroupService(storage storage.GroupStorage, logger *logger.Logger) *GroupService {
	return &GroupService{
		storage: storage,
		logger:  logger,
	}
}

// CreateGroup 创建群组
func (s *GroupService) CreateGroup(name, ownerUID string) (string, error) {
	s.logger.Infof("创建群组: %s, 群主: %s", name, ownerUID)

	if name == "" {
		return "", fmt.Errorf("群组名称不能为空")
	}
	if ownerUID == "" {
		return "", fmt.Errorf("群主UID不能为空")
	}

	return s.storage.CreateGroup(name, ownerUID)
}

// GetGroup 获取群组信息
func (s *GroupService) GetGroup(groupID string) (*model.Group, error) {
	s.logger.Debugf("获取群组信息: %s", groupID)
	return s.storage.GetGroup(groupID)
}

// UpdateGroupName 更新群组名称
func (s *GroupService) UpdateGroupName(groupID, newName string) error {
	s.logger.Infof("更新群组名称: %s -> %s", groupID, newName)

	if newName == "" {
		return fmt.Errorf("群组名称不能为空")
	}

	return s.storage.UpdateGroupName(groupID, newName)
}

// DisbandGroup 解散群组
func (s *GroupService) DisbandGroup(groupID string) error {
	s.logger.Infof("解散群组: %s", groupID)
	return s.storage.DisbandGroup(groupID)
}

// JoinGroup 加入群组
func (s *GroupService) JoinGroup(groupID, uid string) error {
	s.logger.Infof("加入群组: %s -> %s", uid, groupID)
	return s.storage.JoinGroup(groupID, uid)
}

// LeaveGroup 离开群组
func (s *GroupService) LeaveGroup(groupID, uid string) error {
	s.logger.Infof("离开群组: %s -> %s", uid, groupID)
	return s.storage.LeaveGroup(groupID, uid)
}

// GetUserGroups 获取用户群组列表
func (s *GroupService) GetUserGroups(userUID string) ([]*model.Group, error) {
	s.logger.Debugf("获取用户群组列表: %s", userUID)
	return s.storage.GetUserGroups(userUID)
}

// GetGroupMembers 获取群组成员
func (s *GroupService) GetGroupMembers(groupID string) ([]*model.GroupMember, error) {
	s.logger.Debugf("获取群组成员: %s", groupID)
	return s.storage.GetGroupMembers(groupID)
}

// GetUserRoleInGroup 获取用户在群组中的角色
func (s *GroupService) GetUserRoleInGroup(groupID, userUID string) (string, error) {
	s.logger.Debugf("获取用户角色: %s in %s", userUID, groupID)
	return s.storage.GetUserRoleInGroup(groupID, userUID)
}

// SetGroupMemberRole 设置群组成员角色
func (s *GroupService) SetGroupMemberRole(groupID, userUID, role string) error {
	s.logger.Infof("设置群组成员角色: %s -> %s, 角色: %s", userUID, groupID, role)

	if role != "owner" && role != "admin" && role != "member" {
		return fmt.Errorf("无效的角色: %s", role)
	}

	return s.storage.SetGroupMemberRole(groupID, userUID, role)
}

// SetGroupMemberNickname 设置群成员昵称
func (s *GroupService) SetGroupMemberNickname(groupID, userUID, nickname string) error {
	s.logger.Infof("设置群成员昵称: %s -> %s, 昵称: %s", userUID, groupID, nickname)
	return s.storage.SetGroupMemberNickname(groupID, userUID, nickname)
}

// SetGroupMemberRemark 设置群成员备注
func (s *GroupService) SetGroupMemberRemark(groupID, userUID, remark string) error {
	s.logger.Infof("设置群成员备注: %s -> %s, 备注: %s", userUID, groupID, remark)
	return s.storage.SetGroupMemberRemark(groupID, userUID, remark)
}

// SetGroupMemberDND 设置群成员免打扰
func (s *GroupService) SetGroupMemberDND(groupID, userUID string, dnd bool) error {
	s.logger.Infof("设置群成员免打扰: %s -> %s, 免打扰: %v", userUID, groupID, dnd)
	return s.storage.SetGroupMemberDND(groupID, userUID, dnd)
}

// GetGroupMemberDND 获取群成员免打扰状态
func (s *GroupService) GetGroupMemberDND(groupID, userUID string) (bool, error) {
	s.logger.Debugf("获取群成员免打扰状态: %s -> %s", userUID, groupID)
	return s.storage.GetGroupMemberDND(groupID, userUID)
}

// SetGroupMemberMute 设置群成员禁言
func (s *GroupService) SetGroupMemberMute(groupID, userUID string, muted bool) error {
	s.logger.Infof("设置群成员禁言: %s -> %s, 禁言: %v", userUID, groupID, muted)
	return s.storage.SetGroupMemberMute(groupID, userUID, muted)
}

// InsertInviteRequest 插入邀请请求
func (s *GroupService) InsertInviteRequest(groupID, inviterUID, inviteeUID string) error {
	s.logger.Infof("插入邀请请求: %s -> %s, 邀请者: %s", inviteeUID, groupID, inviterUID)
	return s.storage.InsertInviteRequest(groupID, inviterUID, inviteeUID)
}

// GetPendingInviteRequests 获取待处理的邀请请求
func (s *GroupService) GetPendingInviteRequests(groupID string) ([]*model.GroupInviteRequest, error) {
	s.logger.Debugf("获取待处理邀请请求: %s", groupID)
	return s.storage.GetPendingInviteRequests(groupID)
}

// ApproveInviteRequest 批准邀请请求
func (s *GroupService) ApproveInviteRequest(requestID string) error {
	s.logger.Infof("批准邀请请求: %s", requestID)
	return s.storage.ApproveInviteRequest(requestID)
}

// RejectInviteRequest 拒绝邀请请求
func (s *GroupService) RejectInviteRequest(requestID string) error {
	s.logger.Infof("拒绝邀请请求: %s", requestID)
	return s.storage.RejectInviteRequest(requestID)
}

// GetGroupAdminsAndOwner 获取群组管理员和群主
func (s *GroupService) GetGroupAdminsAndOwner(groupID string) ([]string, error) {
	s.logger.Debugf("获取群组管理员和群主: %s", groupID)
	return s.storage.GetGroupAdminsAndOwner(groupID)
}

// HasPendingInvite 检查是否有待处理的邀请
func (s *GroupService) HasPendingInvite(groupID, userUID string) (bool, error) {
	s.logger.Debugf("检查待处理邀请: %s -> %s", userUID, groupID)
	return s.storage.HasPendingInvite(groupID, userUID)
}
