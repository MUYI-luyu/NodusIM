package service

import (
	"time"

	"im/internal/services/message-service/model"
	"im/internal/services/message-service/storage"
	"im/internal/services/message-service/utils"
	"im/internal/shared/logger"
)

// MessageService 消息服务
type MessageService struct {
	storage storage.MessageStorage
	logger  *logger.Logger
}

// NewMessageService 创建消息服务实例
func NewMessageService(storage storage.MessageStorage, logger *logger.Logger) *MessageService {
	return &MessageService{
		storage: storage,
		logger:  logger,
	}
}

// SendPrivateMessage 发送私聊消息
func (s *MessageService) SendPrivateMessage(from, to, messageType, content, extra string) (*model.IMMessage, error) {
	s.logger.Infof("发送私聊消息: %s -> %s, 类型: %s", from, to, messageType)

	// 创建消息
	msg := &model.IMMessage{
		ID:        utils.GenerateMessageID(),
		From:      from,
		To:        to,
		Type:      messageType,
		Content:   content,
		Extra:     extra,
		Timestamp: time.Now().Unix(),
		CreatedAt: time.Now(),
	}

	// 生成会话键
	sessionKey := utils.GenerateSessionKey(from, to)

	// 存储到Redis
	if err := s.storage.StoreMessage(sessionKey, msg); err != nil {
		s.logger.Errorf("存储消息到Redis失败: %v", err)
		return nil, err
	}

	// 存储到MongoDB
	privateMsg := &model.PrivateMessage{
		IMMessage: *msg,
	}
	if err := s.storage.StorePrivateMessage(privateMsg); err != nil {
		s.logger.Errorf("存储私聊消息到MongoDB失败: %v", err)
		return nil, err
	}

	return msg, nil
}

// SendGroupMessage 发送群聊消息
func (s *MessageService) SendGroupMessage(from, groupID, messageType, content, extra string) (*model.IMMessage, error) {
	s.logger.Infof("发送群聊消息: %s -> %s, 类型: %s", from, groupID, messageType)

	// 创建消息
	msg := &model.IMMessage{
		ID:        utils.GenerateMessageID(),
		From:      from,
		To:        groupID, // 群聊消息使用To字段存储群组ID
		Type:      messageType,
		Content:   content,
		Extra:     extra,
		Timestamp: time.Now().Unix(),
		CreatedAt: time.Now(),
	}

	// 生成会话键
	sessionKey := utils.GenerateGroupSessionKey(groupID)

	// 存储到Redis
	if err := s.storage.StoreMessage(sessionKey, msg); err != nil {
		s.logger.Errorf("存储消息到Redis失败: %v", err)
		return nil, err
	}

	// 存储到MongoDB
	groupMsg := &model.GroupMessage{
		IMMessage: *msg,
	}
	if err := s.storage.StoreGroupMessage(groupMsg); err != nil {
		s.logger.Errorf("存储群聊消息到MongoDB失败: %v", err)
		return nil, err
	}

	return msg, nil
}

// GetRecentPrivateMessages 获取最近私聊消息
func (s *MessageService) GetRecentPrivateMessages(from, to string, count int) ([]*model.IMMessage, error) {
	s.logger.Debugf("获取最近私聊消息: %s <-> %s, 数量: %d", from, to, count)

	// 生成会话键
	sessionKey := utils.GenerateSessionKey(from, to)

	// 先从Redis获取
	messages, err := s.storage.GetRecentMessages(sessionKey, count)
	if err != nil || len(messages) == 0 {
		// Redis没有数据，从MongoDB获取
		s.logger.Debugf("Redis中没有消息，从MongoDB获取")
		return s.storage.GetPrivateMessages(from, to, count)
	}

	// 解析Redis中的消息
	var result []*model.IMMessage
	for _, msgData := range messages {
		var msg model.IMMessage
		if err := utils.UnmarshalMessage([]byte(msgData), &msg); err == nil {
			result = append(result, &msg)
		}
	}

	return result, nil
}

// GetRecentGroupMessages 获取最近群聊消息
func (s *MessageService) GetRecentGroupMessages(groupID string, count int) ([]*model.IMMessage, error) {
	s.logger.Debugf("获取最近群聊消息: %s, 数量: %d", groupID, count)

	// 生成会话键
	sessionKey := utils.GenerateGroupSessionKey(groupID)

	// 先从Redis获取
	messages, err := s.storage.GetRecentMessages(sessionKey, count)
	if err != nil || len(messages) == 0 {
		// Redis没有数据，从MongoDB获取
		s.logger.Debugf("Redis中没有消息，从MongoDB获取")
		return s.storage.GetGroupMessages(groupID, count)
	}

	// 解析Redis中的消息
	var result []*model.IMMessage
	for _, msgData := range messages {
		var msg model.IMMessage
		if err := utils.UnmarshalMessage([]byte(msgData), &msg); err == nil {
			result = append(result, &msg)
		}
	}

	return result, nil
}

// StoreOfflineMessage 存储离线消息
func (s *MessageService) StoreOfflineMessage(userID string, message *model.IMMessage) error {
	return s.storage.StoreOfflineMessage(userID, message)
}

// GetOfflineMessages 获取离线消息
func (s *MessageService) GetOfflineMessages(userID string) ([]*model.IMMessage, error) {
	return s.storage.GetOfflineMessages(userID)
}

// ClearOfflineMessages 清除离线消息
func (s *MessageService) ClearOfflineMessages(userID string) error {
	return s.storage.ClearOfflineMessages(userID)
}

// StorePrivateMessage 存储私聊消息到MongoDB
func (s *MessageService) StorePrivateMessage(message *model.PrivateMessage) error {
	return s.storage.StorePrivateMessage(message)
}

// StoreGroupMessage 存储群聊消息到MongoDB
func (s *MessageService) StoreGroupMessage(message *model.GroupMessage) error {
	return s.storage.StoreGroupMessage(message)
}
