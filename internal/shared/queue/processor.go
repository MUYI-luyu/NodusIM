package queue

import (
	"context"
	"fmt"
	"time"

	"im/internal/shared/logger"

	"github.com/redis/go-redis/v9"
)

type MessageProcessor interface {
	ProcessMessage(ctx context.Context, message *QueueMessage) error
	GetMessageType() string
}

type ProcessorManager struct {
	manager    *Manager
	processors map[string]MessageProcessor
	logger     *logger.Logger
}

func NewProcessorManager(manager *Manager, logger *logger.Logger) *ProcessorManager {
	return &ProcessorManager{manager: manager, processors: make(map[string]MessageProcessor), logger: logger}
}

func (pm *ProcessorManager) RegisterProcessor(processor MessageProcessor) {
	pm.processors[processor.GetMessageType()] = processor
}

func (pm *ProcessorManager) StartConsumer(ctx context.Context, streamName, consumerGroup, consumerName string) error {
	if err := pm.manager.EnsureConsumerGroup(ctx, streamName, consumerGroup); err != nil {
		return err
	}

	claimTicker := time.NewTicker(30 * time.Second)
	defer claimTicker.Stop()
	if err := pm.claimAndProcess(ctx, streamName, consumerGroup, consumerName); err != nil {
		pm.logger.Errorf("恢复 Pending 消息失败: %v", err)
	}

	for {
		streams, err := pm.manager.ConsumeMessages(ctx, streamName, consumerGroup, consumerName, 5*time.Second)
		if err != nil {
			pm.logger.Errorf("消费 Stream 消息失败: %v", err)
			time.Sleep(time.Second)
		} else {
			for _, stream := range streams {
				for _, message := range stream.Messages {
					if err := pm.processMessage(ctx, streamName, consumerGroup, message); err != nil {
						pm.logger.Errorf("处理 Stream 消息 %s 失败，将保留 Pending: %v", message.ID, err)
					}
				}
			}
		}

		select {
		case <-ctx.Done():
			return nil
		case <-claimTicker.C:
			if err := pm.claimAndProcess(ctx, streamName, consumerGroup, consumerName); err != nil {
				pm.logger.Errorf("恢复 Pending 消息失败: %v", err)
			}
		default:
		}
	}
}

func (pm *ProcessorManager) claimAndProcess(ctx context.Context, streamName, consumerGroup, consumerName string) error {
	start := "0-0"
	for {
		messages, next, err := pm.manager.ClaimPendingMessages(ctx, streamName, consumerGroup, consumerName, time.Minute, start)
		if err != nil {
			return err
		}
		for _, message := range messages {
			if err := pm.processMessage(ctx, streamName, consumerGroup, message); err != nil {
				pm.logger.Errorf("恢复 Pending 消息 %s 失败，将继续保留: %v", message.ID, err)
			}
		}
		if next == "0-0" || len(messages) == 0 {
			return nil
		}
		start = next
	}
}

func (pm *ProcessorManager) processMessage(ctx context.Context, streamName, consumerGroup string, message redis.XMessage) error {
	queueMessage, err := pm.manager.ParseMessage(message.Values)
	if err != nil {
		return err
	}
	processor, ok := pm.processors[queueMessage.Type]
	if !ok {
		return fmt.Errorf("未找到消息处理器: %s", queueMessage.Type)
	}
	if err := processor.ProcessMessage(ctx, queueMessage); err != nil {
		return err
	}
	return pm.manager.AcknowledgeMessage(ctx, streamName, consumerGroup, message.ID)
}
