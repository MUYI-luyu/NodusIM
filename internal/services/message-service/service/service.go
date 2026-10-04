package service

import (
	"errors"

	"im/internal/services/message-service/model"
	"im/internal/services/message-service/storage"
	"im/internal/services/message-service/utils"
	"im/internal/shared/logger"

	"go.mongodb.org/mongo-driver/mongo"
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

// PersistPrivateMessage 持久化私聊消息。MongoDB 是可靠消息历史的事实来源，
// Redis 仅保存近期查询缓存，因此缓存失败不会影响消费确认。
func (s *MessageService) PersistPrivateMessage(msg *model.IMMessage) (bool, error) {
	privateMsg := &model.PrivateMessage{
		IMMessage:  *msg,
		SessionKey: utils.GenerateSessionKey(msg.From, msg.To),
	}
	if err := s.storage.StorePrivateMessage(privateMsg); err != nil {
		if isDuplicateKeyError(err) {
			return false, nil
		}
		return false, err
	}

	if err := s.storage.StoreMessage(privateMsg.SessionKey, msg); err != nil {
		s.logger.Errorf("写入私聊近期缓存失败，消息已持久化: %v", err)
	}
	return true, nil
}

// PersistGroupMessage 持久化群聊消息。To 仍承载群组 ID，与既有消息模型保持一致。
func (s *MessageService) PersistGroupMessage(msg *model.IMMessage) (bool, error) {
	groupID := msg.To
	groupMsg := &model.GroupMessage{
		IMMessage: *msg,
		GroupID:   groupID,
	}
	if err := s.storage.StoreGroupMessage(groupMsg); err != nil {
		if isDuplicateKeyError(err) {
			return false, nil
		}
		return false, err
	}

	if err := s.storage.StoreMessage(utils.GenerateGroupSessionKey(groupID), msg); err != nil {
		s.logger.Errorf("写入群聊近期缓存失败，消息已持久化: %v", err)
	}
	return true, nil
}

func isDuplicateKeyError(err error) bool {
	var writeException mongo.WriteException
	if !errors.As(err, &writeException) {
		return false
	}
	for _, writeErr := range writeException.WriteErrors {
		if writeErr.Code == 11000 {
			return true
		}
	}
	return false
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
