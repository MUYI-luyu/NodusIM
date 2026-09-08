package websocket

import (
	"context"
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"sync"
	"time"

	"im/internal/shared/logger"
	pb "im/internal/shared/protocol/pb"

	"github.com/gorilla/websocket"
	"github.com/redis/go-redis/v9"
	"google.golang.org/protobuf/proto"
)

// ChatConnectionInfo 聊天连接信息
type ChatConnectionInfo struct {
	UserID      string    `json:"user_id"`
	InstanceID  string    `json:"instance_id"`
	ConnectedAt time.Time `json:"connected_at"`
}

// ChatConnectionManager 聊天室 WebSocket 连接管理器
type ChatConnectionManager struct {
	// 本地连接存储
	uidToConn map[string]*websocket.Conn
	mu        sync.RWMutex
	logger    *logger.Logger

	// Redis客户端用于分布式连接管理
	redis *redis.Client

	// 实例ID，用于标识当前实例
	instanceID string

	// 连接池配置
	maxConnections    int
	heartbeatInterval time.Duration
	writeTimeout      time.Duration
	readTimeout       time.Duration
}

// NewChatConnectionManager 创建聊天室连接管理器
func NewChatConnectionManager(logger *logger.Logger, redis *redis.Client) *ChatConnectionManager {
	// 生成实例ID
	instanceID := fmt.Sprintf("message-service-%d", os.Getpid())

	return &ChatConnectionManager{
		uidToConn:         make(map[string]*websocket.Conn),
		logger:            logger,
		redis:             redis,
		instanceID:        instanceID,
		maxConnections:    10000,
		heartbeatInterval: 30 * time.Second,
		writeTimeout:      10 * time.Second,
		readTimeout:       60 * time.Second,
	}
}

// AddConnection 添加连接
func (cm *ChatConnectionManager) AddConnection(userID string, conn *websocket.Conn) error {
	cm.mu.Lock()
	defer cm.mu.Unlock()

	// 检查连接数限制
	if len(cm.uidToConn) >= cm.maxConnections {
		return ErrMaxConnectionsReached
	}

	// 设置连接参数
	conn.SetReadDeadline(time.Now().Add(cm.readTimeout))
	conn.SetWriteDeadline(time.Now().Add(cm.writeTimeout))
	conn.SetPongHandler(func(string) error {
		conn.SetReadDeadline(time.Now().Add(cm.readTimeout))
		return nil
	})

	// 关闭旧连接
	if oldConn, exists := cm.uidToConn[userID]; exists {
		oldConn.Close()
	}

	// 添加到本地连接
	cm.uidToConn[userID] = conn

	// 注册到Redis（分布式连接管理）
	ctx := context.Background()
	connInfo := ChatConnectionInfo{
		UserID:      userID,
		InstanceID:  cm.instanceID,
		ConnectedAt: time.Now(),
	}

	data, err := json.Marshal(connInfo)
	if err != nil {
		cm.logger.Errorf("序列化连接信息失败: %v", err)
	} else {
		// 存储连接信息到Redis，设置过期时间
		key := fmt.Sprintf("ws_connection:%s", userID)
		cm.redis.Set(ctx, key, data, 24*time.Hour)

		// 添加到实例连接集合
		instanceKey := fmt.Sprintf("ws_instance:%s", cm.instanceID)
		cm.redis.SAdd(ctx, instanceKey, userID)
		cm.redis.Expire(ctx, instanceKey, 24*time.Hour)
	}

	cm.logger.Infof("用户 %s 连接已添加，当前连接数: %d", userID, len(cm.uidToConn))

	// 发送登录就绪确认(login_ack)为protobuf二进制
	ack := &pb.IMMessage{Type: "login_ack", Content: "ok", Timestamp: time.Now().Unix()}
	b, _ := proto.Marshal(ack)
	_ = conn.WriteMessage(websocket.BinaryMessage, b)

	return nil
}

// RemoveConnection 移除连接
func (cm *ChatConnectionManager) RemoveConnection(userID string) {
	cm.mu.Lock()
	defer cm.mu.Unlock()

	if conn, exists := cm.uidToConn[userID]; exists {
		conn.Close()
		delete(cm.uidToConn, userID)

		// 从Redis中移除连接信息
		ctx := context.Background()
		key := fmt.Sprintf("ws_connection:%s", userID)
		cm.redis.Del(ctx, key)

		// 从实例连接集合中移除
		instanceKey := fmt.Sprintf("ws_instance:%s", cm.instanceID)
		cm.redis.SRem(ctx, instanceKey, userID)

		cm.logger.Infof("用户 %s 连接已移除，当前连接数: %d", userID, len(cm.uidToConn))
	}
}

// GetConnection 获取连接
func (cm *ChatConnectionManager) GetConnection(userID string) (*websocket.Conn, bool) {
	cm.mu.RLock()
	defer cm.mu.RUnlock()

	conn, exists := cm.uidToConn[userID]
	return conn, exists
}

// IsUserOnline 分布式在线检查（本地或其他实例）
func (cm *ChatConnectionManager) IsUserOnline(userID string) bool {
	// 本地先查
	cm.mu.RLock()
	if _, exists := cm.uidToConn[userID]; exists {
		cm.mu.RUnlock()
		return true
	}
	cm.mu.RUnlock()

	// 再查Redis中的连接键
	ctx := context.Background()
	key := fmt.Sprintf("ws_connection:%s", userID)
	n, err := cm.redis.Exists(ctx, key).Result()
	if err != nil {
		return false
	}
	return n > 0
}

// BroadcastToGroup 群组广播
func (cm *ChatConnectionManager) BroadcastToGroup(userIDs []string, message []byte) {
	cm.mu.RLock()
	defer cm.mu.RUnlock()

	var wg sync.WaitGroup
	for _, userID := range userIDs {
		if conn, exists := cm.uidToConn[userID]; exists {
			wg.Add(1)
			go func(conn *websocket.Conn) {
				defer wg.Done()
				conn.SetWriteDeadline(time.Now().Add(cm.writeTimeout))
				if err := conn.WriteMessage(websocket.BinaryMessage, message); err != nil {
					cm.logger.Errorf("发送消息失败: %v", err)
				}
			}(conn)
		}
	}
	wg.Wait()
}

// BroadcastToUser 单用户发送（支持跨实例）
func (cm *ChatConnectionManager) BroadcastToUser(userID string, message []byte) error {
	// 首先检查本地连接
	cm.mu.RLock()
	conn, exists := cm.uidToConn[userID]
	cm.mu.RUnlock()

	if exists {
		// 用户在当前实例，直接发送
		conn.SetWriteDeadline(time.Now().Add(cm.writeTimeout))
		return conn.WriteMessage(websocket.BinaryMessage, message)
	}

	// 用户不在当前实例，通过Redis Pub/Sub发送到其他实例
	ctx := context.Background()
	channel := fmt.Sprintf("ws_broadcast:%s", userID)

	// 构造消息
	encoded := base64.StdEncoding.EncodeToString(message)
	broadcastMsg := map[string]interface{}{
		"user_id": userID,
		"message": encoded,
		"from":    cm.instanceID,
	}

	data, err := json.Marshal(broadcastMsg)
	if err != nil {
		return fmt.Errorf("序列化广播消息失败: %v", err)
	}

	// 发布消息到Redis
	err = cm.redis.Publish(ctx, channel, data).Err()
	if err != nil {
		return fmt.Errorf("发布广播消息失败: %v", err)
	}

	// debug log removed
	return nil
}

// StartHeartbeat 启动心跳检测
func (cm *ChatConnectionManager) StartHeartbeat(ctx context.Context) {
	ticker := time.NewTicker(cm.heartbeatInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			cm.sendHeartbeat()
		}
	}
}

// StartRedisSubscriber 启动Redis订阅，处理跨实例消息
func (cm *ChatConnectionManager) StartRedisSubscriber(ctx context.Context) {
	// 订阅所有WebSocket广播频道
	pubsub := cm.redis.PSubscribe(ctx, "ws_broadcast:*")
	defer pubsub.Close()

	cm.logger.Info("启动Redis订阅，监听跨实例消息")

	for {
		select {
		case <-ctx.Done():
			return
		default:
			msg, err := pubsub.ReceiveMessage(ctx)
			if err != nil {
				cm.logger.Errorf("接收Redis消息失败: %v", err)
				continue
			}

			// 处理接收到的消息
			cm.handleBroadcastMessage(msg.Payload)
		}
	}
}

// handleBroadcastMessage 处理广播消息
func (cm *ChatConnectionManager) handleBroadcastMessage(payload string) {
	cm.logger.Debugf("[WS] cross-instance received bytes=%d", len(payload))

	var broadcastMsg map[string]interface{}
	err := json.Unmarshal([]byte(payload), &broadcastMsg)
	if err != nil {
		cm.logger.Errorf("解析广播消息失败: %v", err)
		return
	}

	userID, ok := broadcastMsg["user_id"].(string)
	if !ok {
		cm.logger.Errorf("广播消息缺少user_id")
		return
	}

	messageData, ok := broadcastMsg["message"].(string)
	if !ok {
		cm.logger.Errorf("广播消息缺少message")
		return
	}
	// base64解码
	raw, err := base64.StdEncoding.DecodeString(messageData)
	if err != nil {
		cm.logger.Errorf("广播消息解码失败: %v", err)
		return
	}

	from, ok := broadcastMsg["from"].(string)
	if !ok {
		cm.logger.Errorf("广播消息缺少from")
		return
	}

	// 忽略自己发送的消息
	if from == cm.instanceID {
		return
	}

	// 试探解析payload类型，便于更有意义的日志
	decodedAs := "unknown"
	{
		var im pb.IMMessage
		if err := proto.Unmarshal(raw, &im); err == nil && im.GetType() != "" {
			decodedAs = "im"
			cm.logger.Debugf("[WS][im] recv: to=%s type=%s group=%s bytes=%d", userID, im.GetType(), im.GetGroupId(), len(raw))
		}
	}
	if decodedAs == "unknown" {
		var nf pb.Notification
		if err := proto.Unmarshal(raw, &nf); err == nil && nf.GetType() != "" {
			decodedAs = "notif"
			cm.logger.Debugf("[WS][notif] recv: to=%s type=%s group=%s bytes=%d", userID, nf.GetType(), nf.GetGroupId(), len(raw))
		}
	}
	if decodedAs == "unknown" {
		cm.logger.Warnf("[WS] recv undecodable payload: to=%s bytes=%d", userID, len(raw))
	}

	// 检查用户是否在当前实例
	cm.mu.RLock()
	conn, exists := cm.uidToConn[userID]
	cm.mu.RUnlock()

	if exists {
		// 用户在当前实例，发送消息
		conn.SetWriteDeadline(time.Now().Add(cm.writeTimeout))
		err := conn.WriteMessage(websocket.BinaryMessage, raw)
		if err != nil {
			cm.logger.Errorf("发送跨实例消息失败(%s): %v", decodedAs, err)
		} else {
			cm.logger.Debugf("成功发送跨实例消息给用户(%s): %s", decodedAs, userID)
		}
		return
	}

	// 可能存在刚登录与订阅竞态：稍作重试
	for i := 0; i < 3; i++ {
		time.Sleep(100 * time.Millisecond)
		cm.mu.RLock()
		conn, exists = cm.uidToConn[userID]
		cm.mu.RUnlock()
		if exists {
			conn.SetWriteDeadline(time.Now().Add(cm.writeTimeout))
			err := conn.WriteMessage(websocket.BinaryMessage, raw)
			if err != nil {
				cm.logger.Errorf("发送跨实例消息失败(重试,%s): %v", decodedAs, err)
			} else {
				cm.logger.Debugf("成功发送跨实例消息给用户(重试,%s): %s", decodedAs, userID)
			}
			return
		}
	}
}

// sendHeartbeat 发送心跳
func (cm *ChatConnectionManager) sendHeartbeat() {
	cm.mu.RLock()
	defer cm.mu.RUnlock()

	var wg sync.WaitGroup

	for userID, conn := range cm.uidToConn {
		wg.Add(1)
		go func(userID string, conn *websocket.Conn) {
			defer wg.Done()
			conn.SetWriteDeadline(time.Now().Add(cm.writeTimeout))
			// 发送protobuf心跳
			hb := &pb.IMMessage{Type: "ping", Timestamp: time.Now().Unix()}
			data, _ := proto.Marshal(hb)
			if err := conn.WriteMessage(websocket.BinaryMessage, data); err != nil {
				cm.logger.Errorf("发送心跳失败，用户: %s, 错误: %v", userID, err)
				// 连接可能已断开，将在下次清理时移除
			}
		}(userID, conn)
	}
	wg.Wait()
}

// GetConnectionCount 获取连接数
func (cm *ChatConnectionManager) GetConnectionCount() int {
	cm.mu.RLock()
	defer cm.mu.RUnlock()
	return len(cm.uidToConn)
}

// CleanupDeadConnections 清理死连接
func (cm *ChatConnectionManager) CleanupDeadConnections() {
	cm.mu.Lock()
	defer cm.mu.Unlock()

	for userID, conn := range cm.uidToConn {
		// 尝试发送protobuf心跳检测连接状态
		conn.SetWriteDeadline(time.Now().Add(1 * time.Second))
		hb := &pb.IMMessage{Type: "ping", Timestamp: time.Now().Unix()}
		data, _ := proto.Marshal(hb)
		if err := conn.WriteMessage(websocket.BinaryMessage, data); err != nil {
			conn.Close()
			delete(cm.uidToConn, userID)
			cm.logger.Infof("清理死连接: %s", userID)
		}
	}
}

// 错误定义
var (
	ErrMaxConnectionsReached = errors.New("max connections reached")
	ErrUserNotConnected      = errors.New("user not connected")
)
