package queue

import (
	"context"
	"encoding/json"
	"fmt"
	"time"

	"im/internal/services/message-service/model"
	"im/internal/services/message-service/service"
	"im/internal/shared/logger"
	sharedqueue "im/internal/shared/queue"
)

// MessagePayload is the message body carried by a Redis Stream event.
// The message ID is generated before XADD so a claimed event can be persisted idempotently.
type MessagePayload struct {
	ID        string   `json:"id"`
	From      string   `json:"from"`
	To        string   `json:"to,omitempty"`
	GroupID   string   `json:"group_id,omitempty"`
	Type      string   `json:"type"`
	Content   string   `json:"content"`
	Extra     string   `json:"extra"`
	Timestamp int64    `json:"timestamp"`
	Members   []string `json:"members,omitempty"`
}

type MessageProcessor struct {
	chatService    *service.MessageService
	deliverPrivate func(*model.IMMessage)
	deliverGroup   func(*model.IMMessage, []string)
	logger         *logger.Logger
}

func NewMessageProcessor(chatService *service.MessageService, deliverPrivate func(*model.IMMessage), deliverGroup func(*model.IMMessage, []string), logger *logger.Logger) *MessageProcessor {
	return &MessageProcessor{
		chatService:    chatService,
		deliverPrivate: deliverPrivate,
		deliverGroup:   deliverGroup,
		logger:         logger,
	}
}

func (mp *MessageProcessor) ProcessMessage(_ context.Context, message *sharedqueue.QueueMessage) error {
	var payload MessagePayload
	if err := json.Unmarshal(message.Data, &payload); err != nil {
		return fmt.Errorf("解析消息事件失败: %w", err)
	}
	if payload.ID == "" || payload.From == "" || payload.Type == "" || payload.Timestamp == 0 {
		return fmt.Errorf("消息事件字段不完整")
	}

	chatMessage := &model.IMMessage{
		ID:        payload.ID,
		From:      payload.From,
		To:        payload.To,
		Type:      payload.Type,
		Content:   payload.Content,
		Extra:     payload.Extra,
		Timestamp: payload.Timestamp,
		CreatedAt: time.Unix(payload.Timestamp, 0),
	}

	if message.Type == sharedqueue.MessageTypePrivateMessage {
		if chatMessage.To == "" {
			return fmt.Errorf("私聊消息缺少接收者")
		}
		inserted, err := mp.chatService.PersistPrivateMessage(chatMessage)
		if err != nil {
			return fmt.Errorf("持久化私聊消息失败: %w", err)
		}
		if inserted && mp.deliverPrivate != nil {
			mp.deliverPrivate(chatMessage)
		}
		return nil
	}

	if message.Type == sharedqueue.MessageTypeGroupMessage {
		if payload.GroupID == "" {
			return fmt.Errorf("群聊消息缺少群组 ID")
		}
		chatMessage.To = payload.GroupID
		inserted, err := mp.chatService.PersistGroupMessage(chatMessage)
		if err != nil {
			return fmt.Errorf("持久化群聊消息失败: %w", err)
		}
		if inserted && mp.deliverGroup != nil {
			mp.deliverGroup(chatMessage, payload.Members)
		}
		return nil
	}

	return fmt.Errorf("未知消息类型: %s", message.Type)
}

func (mp *MessageProcessor) GetMessageType() string {
	return "message_processor"
}
