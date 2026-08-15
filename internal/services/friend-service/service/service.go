package service

import (
	"fmt"

	"im/internal/services/friend-service/storage"
	"im/internal/shared/logger"
)

// FriendService 好友服务
type FriendService struct {
	storage storage.FriendStorage
	logger  *logger.Logger
}

// NewFriendService 创建好友服务实例
func NewFriendService(storage storage.FriendStorage, logger *logger.Logger) *FriendService {
	return &FriendService{
		storage: storage,
		logger:  logger,
	}
}

// AddFriendRequest 添加好友请求
func (s *FriendService) AddFriendRequest(fromUID, toUID, message string) error {
	s.logger.Infof("添加好友请求: %s -> %s, 消息: %s", fromUID, toUID, message)

	if fromUID == toUID {
		return fmt.Errorf("不能向自己发送好友请求")
	}

	return s.storage.AddFriendRequest(fromUID, toUID, message)
}

// GetFriendRequests 获取好友请求列表
func (s *FriendService) GetFriendRequests(userUID string) (map[string]string, error) {
	s.logger.Debugf("获取好友请求列表: %s", userUID)
	return s.storage.GetFriendRequests(userUID)
}

// HandleFriendRequest 处理好友请求
func (s *FriendService) HandleFriendRequest(fromUID, toUID string, accept bool) error {
	action := "拒绝"
	if accept {
		action = "接受"
	}
	s.logger.Infof("%s好友请求: %s -> %s", action, fromUID, toUID)
	return s.storage.HandleFriendRequest(fromUID, toUID, accept)
}

// DeleteFriend 删除好友
func (s *FriendService) DeleteFriend(userUID, friendUID string) error {
	s.logger.Infof("删除好友: %s -> %s", userUID, friendUID)
	return s.storage.DeleteFriend(userUID, friendUID)
}

// GetFriends 获取好友列表
func (s *FriendService) GetFriends(userUID string) ([]string, error) {
	s.logger.Debugf("获取好友列表: %s", userUID)
	return s.storage.GetFriends(userUID)
}

// SetFriendRemark 设置好友备注
func (s *FriendService) SetFriendRemark(userUID, friendUID, remark string) error {
	s.logger.Infof("设置好友备注: %s -> %s, 备注: %s", userUID, friendUID, remark)
	return s.storage.SetFriendRemark(userUID, friendUID, remark)
}

// GetFriendRemark 获取好友备注
func (s *FriendService) GetFriendRemark(userUID, friendUID string) (string, error) {
	s.logger.Debugf("获取好友备注: %s -> %s", userUID, friendUID)
	return s.storage.GetFriendRemark(userUID, friendUID)
}

// SetFriendDND 设置好友免打扰
func (s *FriendService) SetFriendDND(userUID, friendUID string, dnd bool) error {
	s.logger.Infof("设置好友免打扰: %s -> %s, 免打扰: %v", userUID, friendUID, dnd)
	return s.storage.SetFriendDND(userUID, friendUID, dnd)
}

// GetFriendDND 获取好友免打扰状态
func (s *FriendService) GetFriendDND(userUID, friendUID string) (bool, error) {
	s.logger.Debugf("获取好友免打扰状态: %s -> %s", userUID, friendUID)
	return s.storage.GetFriendDND(userUID, friendUID)
}
