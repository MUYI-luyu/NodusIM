package storage

import (
	"context"
	"encoding/json"
	"fmt"
	"time"

	"im/internal/services/message-service/model"
	"im/internal/shared/database"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"

	_ "github.com/go-sql-driver/mysql"
)

// MessageStorage 消息存储接口
type MessageStorage interface {
	// Redis操作
	StoreMessage(sessionKey string, message *model.IMMessage) error
	GetRecentMessages(sessionKey string, count int) ([]string, error)

	// MongoDB操作
	StorePrivateMessage(message *model.PrivateMessage) error
	StoreGroupMessage(message *model.GroupMessage) error
	GetPrivateMessages(from, to string, count int) ([]*model.IMMessage, error)
	GetGroupMessages(groupID string, count int) ([]*model.IMMessage, error)

	// 离线消息操作
	StoreOfflineMessage(userID string, message *model.IMMessage) error
	GetOfflineMessages(userID string) ([]*model.IMMessage, error)
	ClearOfflineMessages(userID string) error

	// 关闭连接
	Close() error
}

// HybridMessageStorage 混合存储实现（Redis + MongoDB）
type HybridMessageStorage struct {
	dbManager *database.Manager
}

// NewMessageStorage 创建消息存储实例
func NewMessageStorage(dbManager *database.Manager) (MessageStorage, error) {
	storage := &HybridMessageStorage{
		dbManager: dbManager,
	}

	// 初始化MongoDB集合
	if err := storage.initCollections(); err != nil {
		return nil, fmt.Errorf("初始化MongoDB集合失败: %v", err)
	}

	return storage, nil
}

// initCollections 初始化MongoDB集合
func (s *HybridMessageStorage) initCollections() error {
	ctx := context.Background()

	// 创建私聊消息集合
	privateCollection := s.dbManager.GetMongoDB().Database("im_messages").Collection("private_messages")
	_, err := privateCollection.Indexes().CreateOne(ctx, mongo.IndexModel{
		Keys: bson.D{
			{"session_key", 1},
			{"timestamp", -1},
		},
	})
	if err != nil {
		return fmt.Errorf("创建私聊消息索引失败: %v", err)
	}

	// 创建群聊消息集合
	groupCollection := s.dbManager.GetMongoDB().Database("im_messages").Collection("group_messages")
	_, err = groupCollection.Indexes().CreateOne(ctx, mongo.IndexModel{
		Keys: bson.D{
			{"group_id", 1},
			{"timestamp", -1},
		},
	})
	if err != nil {
		return fmt.Errorf("创建群聊消息索引失败: %v", err)
	}

	return nil
}

// StoreMessage 存储消息到Redis
func (s *HybridMessageStorage) StoreMessage(sessionKey string, message *model.IMMessage) error {
	ctx := context.Background()

	// 序列化消息
	messageData, err := bson.Marshal(message)
	if err != nil {
		return fmt.Errorf("序列化消息失败: %v", err)
	}

	// 存储到Redis列表
	key := fmt.Sprintf("messages:%s", sessionKey)
	err = s.dbManager.GetRedis().LPush(ctx, key, messageData).Err()
	if err != nil {
		return fmt.Errorf("存储消息到Redis失败: %v", err)
	}

	// 限制列表长度（保留最近1000条消息）
	s.dbManager.GetRedis().LTrim(ctx, key, 0, 999)

	// 设置过期时间（7天）
	s.dbManager.GetRedis().Expire(ctx, key, 7*24*time.Hour)

	return nil
}

// GetRecentMessages 从Redis获取最近消息
func (s *HybridMessageStorage) GetRecentMessages(sessionKey string, count int) ([]string, error) {
	ctx := context.Background()

	key := fmt.Sprintf("messages:%s", sessionKey)
	messages, err := s.dbManager.GetRedis().LRange(ctx, key, 0, int64(count-1)).Result()
	if err != nil {
		return nil, fmt.Errorf("从Redis获取消息失败: %v", err)
	}

	return messages, nil
}

// StorePrivateMessage 存储私聊消息到MongoDB
func (s *HybridMessageStorage) StorePrivateMessage(message *model.PrivateMessage) error {
	ctx := context.Background()

	collection := s.dbManager.GetMongoDB().Database("im_messages").Collection("private_messages")
	_, err := collection.InsertOne(ctx, message)
	if err != nil {
		return fmt.Errorf("存储私聊消息到MongoDB失败: %v", err)
	}

	return nil
}

// StoreGroupMessage 存储群聊消息到MongoDB
func (s *HybridMessageStorage) StoreGroupMessage(message *model.GroupMessage) error {
	ctx := context.Background()

	collection := s.dbManager.GetMongoDB().Database("im_messages").Collection("group_messages")
	_, err := collection.InsertOne(ctx, message)
	if err != nil {
		return fmt.Errorf("存储群聊消息到MongoDB失败: %v", err)
	}

	return nil
}

// GetPrivateMessages 从MongoDB获取私聊消息
func (s *HybridMessageStorage) GetPrivateMessages(from, to string, count int) ([]*model.IMMessage, error) {
	ctx := context.Background()

	collection := s.dbManager.GetMongoDB().Database("im_messages").Collection("private_messages")

	// 构建查询条件
	filter := bson.M{
		"$or": []bson.M{
			{"from": from, "to": to},
			{"from": to, "to": from},
		},
	}

	// 查询选项
	opts := options.Find().
		SetSort(bson.D{{"timestamp", -1}}).
		SetLimit(int64(count))

	cursor, err := collection.Find(ctx, filter, opts)
	if err != nil {
		return nil, fmt.Errorf("查询私聊消息失败: %v", err)
	}
	defer cursor.Close(ctx)

	var messages []*model.IMMessage
	for cursor.Next(ctx) {
		var msg model.PrivateMessage
		if err := cursor.Decode(&msg); err != nil {
			continue
		}
		messages = append(messages, &msg.IMMessage)
	}

	// 反转顺序，让最早的消息在前面
	for i, j := 0, len(messages)-1; i < j; i, j = i+1, j-1 {
		messages[i], messages[j] = messages[j], messages[i]
	}

	return messages, nil
}

// GetGroupMessages 从MongoDB获取群聊消息
func (s *HybridMessageStorage) GetGroupMessages(groupID string, count int) ([]*model.IMMessage, error) {
	ctx := context.Background()

	collection := s.dbManager.GetMongoDB().Database("im_messages").Collection("group_messages")

	// 构建查询条件
	filter := bson.M{"group_id": groupID}

	// 查询选项
	opts := options.Find().
		SetSort(bson.D{{"timestamp", -1}}).
		SetLimit(int64(count))

	cursor, err := collection.Find(ctx, filter, opts)
	if err != nil {
		return nil, fmt.Errorf("查询群聊消息失败: %v", err)
	}
	defer cursor.Close(ctx)

	var messages []*model.IMMessage
	for cursor.Next(ctx) {
		var msg model.GroupMessage
		if err := cursor.Decode(&msg); err != nil {
			continue
		}
		messages = append(messages, &msg.IMMessage)
	}

	// 反转顺序，让最早的消息在前面
	for i, j := 0, len(messages)-1; i < j; i, j = i+1, j-1 {
		messages[i], messages[j] = messages[j], messages[i]
	}

	return messages, nil
}

// Close 关闭连接
func (s *HybridMessageStorage) Close() error {
	// 数据库连接由管理器统一管理，这里不需要关闭
	return nil
}

// StoreOfflineMessage 存储离线消息到Redis
func (s *HybridMessageStorage) StoreOfflineMessage(userID string, message *model.IMMessage) error {
	ctx := context.Background()
	key := fmt.Sprintf("offline_messages:%s", userID)

	// 将消息序列化为JSON存储
	messageData, err := json.Marshal(message)
	if err != nil {
		return fmt.Errorf("序列化消息失败: %v", err)
	}

	// 使用Redis List存储离线消息
	return s.dbManager.GetRedis().LPush(ctx, key, messageData).Err()
}

// GetOfflineMessages 获取用户的离线消息
func (s *HybridMessageStorage) GetOfflineMessages(userID string) ([]*model.IMMessage, error) {
	ctx := context.Background()
	key := fmt.Sprintf("offline_messages:%s", userID)

	// 获取所有离线消息
	messages, err := s.dbManager.GetRedis().LRange(ctx, key, 0, -1).Result()
	if err != nil {
		return nil, fmt.Errorf("获取离线消息失败: %v", err)
	}

	var result []*model.IMMessage
	for _, msgData := range messages {
		var msg model.IMMessage
		if err := json.Unmarshal([]byte(msgData), &msg); err != nil {
			continue // 跳过无效消息
		}
		result = append(result, &msg)
	}

	return result, nil
}

// ClearOfflineMessages 清除用户的离线消息
func (s *HybridMessageStorage) ClearOfflineMessages(userID string) error {
	ctx := context.Background()
	key := fmt.Sprintf("offline_messages:%s", userID)
	return s.dbManager.GetRedis().Del(ctx, key).Err()
}
