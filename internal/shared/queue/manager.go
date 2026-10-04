package queue

import (
	"context"
	"time"

	"im/internal/shared/database"
	"im/internal/shared/logger"

	"github.com/redis/go-redis/v9"
)

type Manager struct {
	queue *MessageQueue
}

func NewManager(dbManager *database.Manager, logger *logger.Logger) *Manager {
	return &Manager{queue: NewMessageQueue(dbManager.GetRedis(), logger)}
}

func (m *Manager) PublishMessage(ctx context.Context, streamName string, message *QueueMessage) (string, error) {
	return m.queue.Publish(ctx, streamName, message)
}

func (m *Manager) EnsureConsumerGroup(ctx context.Context, streamName, consumerGroup string) error {
	return m.queue.EnsureConsumerGroup(ctx, streamName, consumerGroup)
}

func (m *Manager) ConsumeMessages(ctx context.Context, streamName, consumerGroup, consumerName string, block time.Duration) ([]redis.XStream, error) {
	return m.queue.ReadNew(ctx, streamName, consumerGroup, consumerName, block)
}

func (m *Manager) ClaimPendingMessages(ctx context.Context, streamName, consumerGroup, consumerName string, minIdle time.Duration, start string) ([]redis.XMessage, string, error) {
	return m.queue.ClaimPending(ctx, streamName, consumerGroup, consumerName, minIdle, start)
}

func (m *Manager) AcknowledgeMessage(ctx context.Context, streamName, consumerGroup, messageID string) error {
	return m.queue.Acknowledge(ctx, streamName, consumerGroup, messageID)
}

func (m *Manager) ParseMessage(values map[string]interface{}) (*QueueMessage, error) {
	return m.queue.ParseMessage(values)
}

const (
	StreamMessageProcessing       = "message_processing"
	MessageTypePrivateMessage     = "private_message"
	MessageTypeGroupMessage       = "group_message"
	ConsumerGroupMessageProcessor = "message_processor"
)
