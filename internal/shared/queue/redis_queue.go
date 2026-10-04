package queue

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"time"

	"im/internal/shared/logger"

	"github.com/redis/go-redis/v9"
)

// QueueMessage is the durable event written to a Redis Stream.
type QueueMessage struct {
	ID        string          `json:"id"`
	Type      string          `json:"type"`
	Data      json.RawMessage `json:"data"`
	Timestamp int64           `json:"timestamp"`
}

type MessageQueue struct {
	client *redis.Client
	logger *logger.Logger
}

func NewMessageQueue(client *redis.Client, logger *logger.Logger) *MessageQueue {
	return &MessageQueue{client: client, logger: logger}
}

func (q *MessageQueue) Publish(ctx context.Context, streamName string, message *QueueMessage) (string, error) {
	data, err := json.Marshal(message)
	if err != nil {
		return "", fmt.Errorf("序列化队列消息失败: %w", err)
	}

	id, err := q.client.XAdd(ctx, &redis.XAddArgs{
		Stream: streamName,
		Values: map[string]interface{}{
			"type": message.Type,
			"data": string(data),
		},
	}).Result()
	if err != nil {
		return "", fmt.Errorf("发布消息到 Redis Stream 失败: %w", err)
	}

	q.logger.Debugf("消息已发布到 Stream %s: %s", streamName, id)
	return id, nil
}

func (q *MessageQueue) EnsureConsumerGroup(ctx context.Context, streamName, consumerGroup string) error {
	err := q.client.XGroupCreateMkStream(ctx, streamName, consumerGroup, "0").Err()
	if err != nil && !strings.Contains(err.Error(), "BUSYGROUP") {
		return fmt.Errorf("创建 Stream 消费者组失败: %w", err)
	}
	return nil
}

func (q *MessageQueue) ReadNew(ctx context.Context, streamName, consumerGroup, consumerName string, block time.Duration) ([]redis.XStream, error) {
	streams, err := q.client.XReadGroup(ctx, &redis.XReadGroupArgs{
		Group:    consumerGroup,
		Consumer: consumerName,
		Streams:  []string{streamName, ">"},
		Block:    block,
		Count:    100,
	}).Result()
	if err == redis.Nil {
		return nil, nil
	}
	if err != nil {
		return nil, fmt.Errorf("读取 Stream 消息失败: %w", err)
	}
	return streams, nil
}

func (q *MessageQueue) ClaimPending(ctx context.Context, streamName, consumerGroup, consumerName string, minIdle time.Duration, start string) ([]redis.XMessage, string, error) {
	messages, next, err := q.client.XAutoClaim(ctx, &redis.XAutoClaimArgs{
		Stream:   streamName,
		Group:    consumerGroup,
		Consumer: consumerName,
		MinIdle:  minIdle,
		Start:    start,
		Count:    100,
	}).Result()
	if err == redis.Nil {
		return nil, "0-0", nil
	}
	if err != nil {
		return nil, "", fmt.Errorf("认领 Pending 消息失败: %w", err)
	}
	return messages, next, nil
}

func (q *MessageQueue) Acknowledge(ctx context.Context, streamName, consumerGroup, messageID string) error {
	if err := q.client.XAck(ctx, streamName, consumerGroup, messageID).Err(); err != nil {
		return fmt.Errorf("确认 Stream 消息失败: %w", err)
	}
	return nil
}

func (q *MessageQueue) ParseMessage(values map[string]interface{}) (*QueueMessage, error) {
	data, ok := values["data"].(string)
	if !ok {
		return nil, fmt.Errorf("Stream 消息缺少 data 字段")
	}

	var message QueueMessage
	if err := json.Unmarshal([]byte(data), &message); err != nil {
		return nil, fmt.Errorf("解析 Stream 消息失败: %w", err)
	}
	return &message, nil
}
