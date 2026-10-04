package handler

import (
	"context"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"hash/fnv"
	"io"
	"net/http"
	"os"
	"time"

	"im/internal/services/message-service/model"
	msgqueue "im/internal/services/message-service/queue"
	"im/internal/services/message-service/service"
	"im/internal/services/message-service/utils"
	ws "im/internal/services/message-service/websocket"
	"im/internal/shared/auth"
	"im/internal/shared/database"
	"im/internal/shared/discovery"
	"im/internal/shared/logger"
	"im/internal/shared/middleware"
	pb "im/internal/shared/protocol/pb"
	sharedqueue "im/internal/shared/queue"
	"im/internal/shared/rpc"

	"github.com/gorilla/websocket"

	"strings"

	"google.golang.org/protobuf/proto"
)

// MessageHandler 消息处理器
type MessageHandler struct {
	chatService     *service.MessageService
	logger          *logger.Logger
	requestHandler  *middleware.RequestHandler
	chatConnManager *ws.ChatConnectionManager
	rpcManager      *rpc.Manager
	queueManager    *sharedqueue.Manager
}

// WebSocket升级器
var upgrader = websocket.Upgrader{
	CheckOrigin:     func(r *http.Request) bool { return true },
	ReadBufferSize:  4096,
	WriteBufferSize: 4096,
}

// NewMessageHandler 创建消息处理器
func NewMessageHandler(chatService *service.MessageService, logger *logger.Logger, dbManager *database.Manager) *MessageHandler {
	chatConnManager := ws.NewChatConnectionManager(logger, dbManager.GetRedis())

	// 启动心跳检测
	go chatConnManager.StartHeartbeat(context.Background())

	// 启动Redis订阅，处理跨实例消息
	go chatConnManager.StartRedisSubscriber(context.Background())

	// 定期清理死连接
	go func() {
		ticker := time.NewTicker(5 * time.Minute)
		defer ticker.Stop()
		for range ticker.C {
			chatConnManager.CleanupDeadConnections()
		}
	}()

	// 创建RPC管理器
	rpcManager := rpc.NewManager(logger)

	// 启用etcd服务发现（如果提供了ETCD_ENDPOINTS，则优先使用）
	endpoints := os.Getenv("ETCD_ENDPOINTS")
	if endpoints == "" {
		endpoints = "localhost:2379"
	}
	disc, err := discovery.New(discovery.Config{Endpoints: strings.Split(endpoints, ",")})
	if err != nil {
		logger.Fatalf("etcd 连接失败，无法启动服务发现: %v", err)
	}
	rpcManager.UseEtcd(disc, "/im/services")
	_ = rpcManager.WatchService("group-service")
	_ = rpcManager.WatchService("friend-service")
	_ = rpcManager.WatchService("user-service")
	queueManager := sharedqueue.NewManager(dbManager, logger)

	handler := &MessageHandler{
		chatService:     chatService,
		logger:          logger,
		requestHandler:  middleware.NewRequestHandler(logger, dbManager.GetRedis()),
		chatConnManager: chatConnManager,
		rpcManager:      rpcManager,
		queueManager:    queueManager,
	}

	go handler.startQueueConsumer()

	return handler
}

// computeMsgFingerprint 为消息生成稳定的短指纹，帮助跨日志关联
func computeMsgFingerprint(msg *pb.IMMessage) string {
	hasher := fnv.New64a()
	// 使用关键字段构造指纹：from,to,groupId,type,timestamp,content前64字节
	io.WriteString(hasher, msg.From)
	io.WriteString(hasher, "|")
	io.WriteString(hasher, msg.To)
	io.WriteString(hasher, "|")
	io.WriteString(hasher, msg.GroupId)
	io.WriteString(hasher, "|")
	io.WriteString(hasher, msg.Type)
	io.WriteString(hasher, "|")
	var tsBuf [8]byte
	binary.BigEndian.PutUint64(tsBuf[:], uint64(msg.Timestamp))
	hasher.Write(tsBuf[:])
	io.WriteString(hasher, "|")
	content := msg.Content
	if len(content) > 64 {
		content = content[:64]
	}
	io.WriteString(hasher, content)
	sum := hasher.Sum64()
	// 返回16进制短字符串
	return fmt.Sprintf("%016x", sum)
}

// RegisterRoutes 注册路由
func (h *MessageHandler) RegisterRoutes(mux *http.ServeMux) {
	mux.HandleFunc("/ws", h.ws)
	mux.HandleFunc("/notify", h.requestHandler.HandleRequest(h.notify))
	mux.HandleFunc("/get_recent_private_messages", h.requestHandler.HandleRequest(h.getRecentPrivateMessages))
	mux.HandleFunc("/get_recent_group_messages", h.requestHandler.HandleRequest(h.getRecentGroupMessages))
}

// writeResp 写入响应
func (h *MessageHandler) writeResp(w http.ResponseWriter, code int, msg string, data []byte) {
	w.Header().Set("Content-Type", "application/x-protobuf")
	resp := &pb.APIResp{Code: int32(code), Msg: msg, Data: data}
	b, _ := proto.Marshal(resp)
	w.Write(b)
}

// 内部: 发送通知给在线用户
func (h *MessageHandler) sendNotification(to string, notif *pb.Notification) {
	b, _ := proto.Marshal(notif)
	if err := h.chatConnManager.BroadcastToUser(to, b); err != nil {
		// 用户离线，存储离线消息
		h.storeOfflineNotification(to, notif)
	}
}

// 内部: 存储离线通知
func (h *MessageHandler) storeOfflineNotification(userID string, notif *pb.Notification) {
	// 将通知转换为消息格式存储
	msg := &model.IMMessage{
		From:      notif.From,
		To:        notif.To,
		Content:   notif.Content,
		Type:      notif.Type,
		Extra:     notif.GroupId + "|" + notif.GroupName, // 将群组信息存储在Extra字段
		Timestamp: time.Now().Unix(),
	}

	// 存储离线消息
	if err := h.chatService.StoreOfflineMessage(userID, msg); err != nil {
		h.logger.Errorf("存储离线消息失败: %v", err)
	}
}

// 内部: 推送离线消息
func (h *MessageHandler) pushOfflineMessages(userID string, conn *websocket.Conn) {
	// 获取离线消息
	messages, err := h.chatService.GetOfflineMessages(userID)
	if err != nil {
		h.logger.Errorf("获取离线消息失败: %v", err)
		return
	}

	if len(messages) == 0 {
		return
	}

	h.logger.Infof("用户 %s 有 %d 条离线消息", userID, len(messages))

	// 推送离线消息通知
	notif := &pb.Notification{
		Type:      "offline_messages",
		Content:   fmt.Sprintf("您有 %d 条离线消息", len(messages)),
		Timestamp: time.Now().Unix(),
	}

	b, _ := proto.Marshal(notif)
	_ = conn.WriteMessage(websocket.BinaryMessage, b)

	// 清除离线消息
	if err := h.chatService.ClearOfflineMessages(userID); err != nil {
		h.logger.Errorf("清除离线消息失败: %v", err)
	}
}

// 内部: 获取群组成员列表
func (h *MessageHandler) getGroupMembers(groupID string) []string {
	ctx := context.Background()
	req := &pb.GroupMembersReq{GroupId: groupID}

	resp, err := h.rpcManager.CallWithRetry(ctx, "group-service", "/group_members", req, 3)
	if err != nil {
		h.logger.Errorf("获取群组成员失败: %v", err)
		return nil
	}

	var api pb.APIResp
	if err := proto.Unmarshal(resp, &api); err != nil || api.Code != 0 {
		h.logger.Errorf("解析群组成员响应失败: %v", err)
		return nil
	}

	membersResp := &pb.GroupMembersResp{}
	if err := proto.Unmarshal(api.Data, membersResp); err != nil {
		h.logger.Errorf("解析群组成员数据失败: %v", err)
		return nil
	}

	var uids []string
	for _, m := range membersResp.Members {
		uids = append(uids, m.Uid)
	}
	return uids
}

// 内部: 获取群组信息
func (h *MessageHandler) getGroupInfo(groupID string) (string, error) {
	ctx := context.Background()
	req := &pb.GroupInfoReq{GroupId: groupID}

	resp, err := h.rpcManager.CallWithRetry(ctx, "group-service", "/group_info", req, 3)
	if err != nil {
		return "", fmt.Errorf("获取群组信息失败: %v", err)
	}

	var api pb.APIResp
	if err := proto.Unmarshal(resp, &api); err != nil || api.Code != 0 {
		return "", fmt.Errorf("解析群组信息响应失败: %v", err)
	}

	infoResp := &pb.GroupInfoResp{}
	if err := proto.Unmarshal(api.Data, infoResp); err != nil {
		return "", fmt.Errorf("解析群组信息数据失败: %v", err)
	}

	return infoResp.Group.Name, nil
}

// 内部: 检查好友免打扰状态
func (h *MessageHandler) isFriendDND(fromUID, toUID string) bool {
	ctx := context.Background()
	req := &pb.FriendInfoReq{Uid: toUID, FriendUid: fromUID}

	resp, err := h.rpcManager.CallWithRetry(ctx, "friend-service", "/friend_info", req, 3)
	if err != nil {
		h.logger.Errorf("检查好友免打扰状态失败: %v", err)
		return false // 出错时不阻止通知
	}

	var api pb.APIResp
	if err := proto.Unmarshal(resp, &api); err != nil || api.Code != 0 {
		h.logger.Errorf("解析好友信息响应失败: %v", err)
		return false
	}

	infoResp := &pb.FriendInfoResp{}
	if err := proto.Unmarshal(api.Data, infoResp); err != nil {
		h.logger.Errorf("解析好友信息数据失败: %v", err)
		return false
	}

	return infoResp.Dnd
}

// 内部: 检查群组免打扰状态
func (h *MessageHandler) isGroupDND(groupID, userUID string) bool {
	ctx := context.Background()
	req := &pb.SetGroupDNDReq{GroupId: groupID, Uid: userUID}

	resp, err := h.rpcManager.CallWithRetry(ctx, "group-service", "/get_group_dnd", req, 3)
	if err != nil {
		h.logger.Errorf("检查群组免打扰状态失败: %v", err)
		return false // 出错时不阻止通知
	}

	var api pb.APIResp
	if err := proto.Unmarshal(resp, &api); err != nil || api.Code != 0 {
		h.logger.Errorf("解析群组免打扰响应失败: %v", err)
		return false
	}

	dndResp := &pb.SetGroupDNDResp{}
	if err := proto.Unmarshal(api.Data, dndResp); err != nil {
		h.logger.Errorf("解析群组免打扰数据失败: %v", err)
		return false
	}

	return dndResp.Dnd
}

// 内部: 检查群成员是否被禁言
func (h *MessageHandler) isGroupMemberMuted(groupID, userUID string) bool {
	ctx := context.Background()
	req := &pb.GroupMembersReq{GroupId: groupID}

	resp, err := h.rpcManager.CallWithRetry(ctx, "group-service", "/group_members", req, 3)
	if err != nil {
		h.logger.Errorf("调用群组服务获取成员列表失败: %v", err)
		return false // 出错时不阻止发送
	}

	var api pb.APIResp
	if err := proto.Unmarshal(resp, &api); err != nil || api.Code != 0 {
		h.logger.Errorf("群组服务返回错误: code=%d, msg=%s", api.Code, api.Msg)
		return false
	}

	membersResp := &pb.GroupMembersResp{}
	if err := proto.Unmarshal(api.Data, membersResp); err != nil {
		h.logger.Errorf("解析群成员列表失败: %v", err)
		return false
	}

	// 查找指定用户并检查禁言状态
	h.logger.Infof("获取到群组 %s 的成员列表，共 %d 个成员", groupID, len(membersResp.Members))
	for _, member := range membersResp.Members {
		h.logger.Infof("检查成员: UID=%s, Muted=%v", member.Uid, member.GetMuted())
		if member.Uid == userUID {
			muted := member.GetMuted()
			h.logger.Infof("找到用户 %s，禁言状态: %v", userUID, muted)
			return muted
		}
	}
	h.logger.Warnf("未找到用户 %s 在群组 %s 中的成员信息", userUID, groupID)
	return false
}

// ws WebSocket 握手与消息循环（protobuf IMMessage）
func (h *MessageHandler) ws(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		return
	}
	defer conn.Close()

	var uid string
	for {
		_, data, err := conn.ReadMessage()
		if err != nil {
			if uid != "" {
				h.chatConnManager.RemoveConnection(uid)
			}
			return
		}
		var msg pb.IMMessage
		if err := proto.Unmarshal(data, &msg); err != nil {
			// 返回错误
			errMsg := &pb.IMMessage{Type: "error", Content: "消息格式错误"}
			b, _ := proto.Marshal(errMsg)
			_ = conn.WriteMessage(websocket.BinaryMessage, b)
			continue
		}
		if uid == "" {
			if msg.Type != "login" || msg.Token == "" {
				errMsg := &pb.IMMessage{Type: "error", Content: "请先登录"}
				b, _ := proto.Marshal(errMsg)
				_ = conn.WriteMessage(websocket.BinaryMessage, b)
				return
			}
			parsed, err := auth.ParseToken(msg.Token)
			if err != nil {
				errMsg := &pb.IMMessage{Type: "error", Content: "token无效"}
				b, _ := proto.Marshal(errMsg)
				_ = conn.WriteMessage(websocket.BinaryMessage, b)
				return
			}
			uid = parsed

			// 添加到连接管理器
			if err := h.chatConnManager.AddConnection(uid, conn); err != nil {
				h.logger.Errorf("添加连接失败: %v", err)
				return
			}

			// 推送离线消息
			h.pushOfflineMessages(uid, conn)

			ok := &pb.IMMessage{Type: "login", Content: "登录成功"}
			b, _ := proto.Marshal(ok)
			_ = conn.WriteMessage(websocket.BinaryMessage, b)
			continue
		}
		// 群聊消息处理
		if msg.GroupId != "" {
			finger := computeMsgFingerprint(&msg)
			h.logger.Debugf("[GROUP] recv: group=%s from=%s type=%s ts=%d fp=%s len=%d", msg.GroupId, uid, msg.Type, msg.Timestamp, finger, len(msg.Content))
			// 检查发送者是否被禁言
			muted := h.isGroupMemberMuted(msg.GroupId, uid)
			h.logger.Debugf("[GROUP] muted-check: group=%s uid=%s muted=%v", msg.GroupId, uid, muted)
			if muted {
				h.logger.Infof("[GROUP] muted-block: group=%s uid=%s", msg.GroupId, uid)
				// 发送禁言提示消息给发送者
				errorMsg := &pb.IMMessage{
					Type:      "error",
					From:      "system",
					To:        uid,
					Content:   "您已被禁言，无法发送消息",
					Timestamp: time.Now().Unix(),
				}
				bError, _ := proto.Marshal(errorMsg)
				_ = conn.WriteMessage(websocket.BinaryMessage, bError)
				continue
			}
			h.logger.Debugf("[GROUP] muted-pass: group=%s uid=%s", msg.GroupId, uid)

			msg.From = uid
			msg.Timestamp = time.Now().Unix() // 设置正确的时间戳

			// 获取群成员
			members := h.getGroupMembers(msg.GroupId)
			h.logger.Debugf("[GROUP] members: group=%s count=%d fp=%s", msg.GroupId, len(members), finger)

			if err := h.publishGroupMessage(uid, msg.GroupId, msg.Type, msg.Content, msg.Extra, msg.Timestamp, members); err != nil {
				h.logger.Errorf("群聊消息写入 Stream 失败: %v", err)
				errMsg := &pb.IMMessage{Type: "error", Content: "消息接收失败，请重试"}
				b, _ := proto.Marshal(errMsg)
				_ = conn.WriteMessage(websocket.BinaryMessage, b)
				continue
			}
			continue
		}
		// 私聊转发与存储
		if msg.To != "" {
			msg.From = uid
			msg.Timestamp = time.Now().Unix() // 设置正确的时间戳

			if err := h.publishPrivateMessage(uid, msg.To, msg.Type, msg.Content, msg.Extra, msg.Timestamp); err != nil {
				h.logger.Errorf("私聊消息写入 Stream 失败: %v", err)
				errMsg := &pb.IMMessage{Type: "error", Content: "消息接收失败，请重试"}
				b, _ := proto.Marshal(errMsg)
				_ = conn.WriteMessage(websocket.BinaryMessage, b)
				continue
			}
			continue
		}
	}
}

// notify 接收服务间通知并推送给用户
func (h *MessageHandler) notify(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var notif pb.Notification
	if err := proto.Unmarshal(body, &notif); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if notif.To == "" {
		h.writeResp(w, 1, "缺少接收者", nil)
		return
	}
	h.sendNotification(notif.To, &notif)
	h.writeResp(w, 0, "ok", nil)
}

// getRecentPrivateMessages 获取最近私聊消息
func (h *MessageHandler) getRecentPrivateMessages(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.GetRecentPrivateMessagesReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.From == "" || req.To == "" || req.Count <= 0 {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	msgs, err := h.chatService.GetRecentPrivateMessages(req.From, req.To, int(req.Count))
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	var pbMsgs []*pb.IMMessage
	for _, m := range msgs {
		pbMsgs = append(pbMsgs, &pb.IMMessage{
			Type:      m.Type,
			From:      m.From,
			To:        m.To,
			Content:   m.Content,
			Timestamp: m.Timestamp,
			Extra:     m.Extra,
		})
	}
	list := &pb.IMMessageList{Messages: pbMsgs}
	data, _ := proto.Marshal(list)
	h.writeResp(w, 0, "ok", data)
}

// getRecentGroupMessages 获取最近群聊消息
func (h *MessageHandler) getRecentGroupMessages(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.GetRecentGroupMessagesReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Count <= 0 {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	msgs, err := h.chatService.GetRecentGroupMessages(req.GroupId, int(req.Count))
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	var pbMsgs []*pb.IMMessage
	for _, m := range msgs {
		pbMsgs = append(pbMsgs, &pb.IMMessage{
			Type:      m.Type,
			From:      m.From,
			Content:   m.Content,
			Timestamp: m.Timestamp,
			Extra:     m.Extra,
			GroupId:   req.GroupId,
		})
	}
	list := &pb.IMMessageList{Messages: pbMsgs}
	data, _ := proto.Marshal(list)
	h.writeResp(w, 0, "ok", data)
}

// Start 启动HTTP服务器
func (h *MessageHandler) Start(port int) error {
	mux := http.NewServeMux()
	h.RegisterRoutes(mux)

	addr := fmt.Sprintf(":%d", port)
	h.logger.Infof("服务启动在端口 %d", port)

	return http.ListenAndServe(addr, mux)
}

func (h *MessageHandler) startQueueConsumer() {
	port := os.Getenv("PORT")
	consumerName := fmt.Sprintf("message-processor-%s-%d", port, os.Getpid())
	processor := msgqueue.NewMessageProcessor(h.chatService, h.deliverPrivateMessage, h.deliverGroupMessage, h.logger)
	processorManager := sharedqueue.NewProcessorManager(h.queueManager, h.logger)
	processorManager.RegisterProcessor(&privateMessageProcessor{MessageProcessor: processor})
	processorManager.RegisterProcessor(&groupMessageProcessor{MessageProcessor: processor})

	if err := processorManager.StartConsumer(
		context.Background(),
		sharedqueue.StreamMessageProcessing,
		sharedqueue.ConsumerGroupMessageProcessor,
		consumerName,
	); err != nil {
		h.logger.Errorf("消息 Stream 消费者退出: %v", err)
	}
}

type privateMessageProcessor struct {
	*msgqueue.MessageProcessor
}

func (p *privateMessageProcessor) GetMessageType() string {
	return sharedqueue.MessageTypePrivateMessage
}

type groupMessageProcessor struct {
	*msgqueue.MessageProcessor
}

func (p *groupMessageProcessor) GetMessageType() string {
	return sharedqueue.MessageTypeGroupMessage
}

func (h *MessageHandler) publishPrivateMessage(from, to, messageType, content, extra string, timestamp int64) error {
	payload := msgqueue.MessagePayload{
		ID:        utils.GenerateMessageID(),
		From:      from,
		To:        to,
		Type:      messageType,
		Content:   content,
		Extra:     extra,
		Timestamp: timestamp,
	}
	data, err := json.Marshal(payload)
	if err != nil {
		return err
	}
	_, err = h.queueManager.PublishMessage(context.Background(), sharedqueue.StreamMessageProcessing, &sharedqueue.QueueMessage{
		ID:        payload.ID,
		Type:      sharedqueue.MessageTypePrivateMessage,
		Data:      data,
		Timestamp: timestamp,
	})
	return err
}

func (h *MessageHandler) publishGroupMessage(from, groupID, messageType, content, extra string, timestamp int64, members []string) error {
	payload := msgqueue.MessagePayload{
		ID:        utils.GenerateMessageID(),
		From:      from,
		GroupID:   groupID,
		Type:      messageType,
		Content:   content,
		Extra:     extra,
		Timestamp: timestamp,
		Members:   members,
	}
	data, err := json.Marshal(payload)
	if err != nil {
		return err
	}
	_, err = h.queueManager.PublishMessage(context.Background(), sharedqueue.StreamMessageProcessing, &sharedqueue.QueueMessage{
		ID:        payload.ID,
		Type:      sharedqueue.MessageTypeGroupMessage,
		Data:      data,
		Timestamp: timestamp,
	})
	return err
}

// deliverPrivateMessage runs only after the Stream event has been durably stored in MongoDB.
func (h *MessageHandler) deliverPrivateMessage(msg *model.IMMessage) {
	if !h.chatConnManager.IsUserOnline(msg.To) {
		if err := h.chatService.StoreOfflineMessage(msg.To, msg); err != nil {
			h.logger.Errorf("存储离线私聊消息失败: %v", err)
		}
	} else {
		pbMsg := &pb.IMMessage{Type: msg.Type, From: msg.From, To: msg.To, Content: msg.Content, Extra: msg.Extra, Timestamp: msg.Timestamp}
		data, _ := proto.Marshal(pbMsg)
		if err := h.chatConnManager.BroadcastToUser(msg.To, data); err != nil {
			h.logger.Errorf("实时推送私聊消息失败: %v", err)
			if storeErr := h.chatService.StoreOfflineMessage(msg.To, msg); storeErr != nil {
				h.logger.Errorf("存储离线私聊消息失败: %v", storeErr)
			}
		}
	}

	if !h.isFriendDND(msg.From, msg.To) {
		h.sendNotification(msg.To, &pb.Notification{
			Type:      "private_chat_message",
			From:      msg.From,
			To:        msg.To,
			Content:   msg.Content,
			Timestamp: msg.Timestamp,
		})
	}
}

// deliverGroupMessage runs after MongoDB persistence and fans out to the member snapshot in the event.
func (h *MessageHandler) deliverGroupMessage(msg *model.IMMessage, members []string) {
	pbMsg := &pb.IMMessage{Type: msg.Type, From: msg.From, GroupId: msg.To, Content: msg.Content, Extra: msg.Extra, Timestamp: msg.Timestamp}
	data, _ := proto.Marshal(pbMsg)
	seen := make(map[string]struct{}, len(members))
	for _, memberUID := range members {
		if memberUID == msg.From {
			continue
		}
		if _, exists := seen[memberUID]; exists {
			continue
		}
		seen[memberUID] = struct{}{}
		if err := h.chatConnManager.BroadcastToUser(memberUID, data); err != nil {
			h.logger.Warnf("群聊实时推送失败: group=%s to=%s err=%v", msg.To, memberUID, err)
		}
	}

	for memberUID := range seen {
		if h.isGroupDND(msg.To, memberUID) {
			continue
		}
		h.sendNotification(memberUID, &pb.Notification{
			Type:      "group_chat_message",
			From:      msg.From,
			To:        memberUID,
			GroupId:   msg.To,
			Content:   msg.Content,
			Timestamp: msg.Timestamp,
		})
	}
}
