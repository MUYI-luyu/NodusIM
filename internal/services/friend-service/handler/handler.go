package handler

import (
	"context"
	"io"
	"net/http"
	"sync"
	"time"

	"im/internal/services/friend-service/service"
	"im/internal/shared/auth"
	"im/internal/shared/database"
	"im/internal/shared/discovery"
	"im/internal/shared/logger"
	"im/internal/shared/middleware"
	pb "im/internal/shared/protocol/pb"
	"im/internal/shared/rpc"
	"os"
	"strings"

	"google.golang.org/protobuf/proto"
)

// FriendHandler 好友处理器
type FriendHandler struct {
	service        *service.FriendService
	logger         *logger.Logger
	requestHandler *middleware.RequestHandler
	rpcManager     *rpc.Manager
}

// NewFriendHandler 创建好友处理器
func NewFriendHandler(service *service.FriendService, logger *logger.Logger, dbManager *database.Manager) *FriendHandler {
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
	_ = rpcManager.WatchService("user-service")
	_ = rpcManager.WatchService("notification-service")

	return &FriendHandler{
		service:        service,
		logger:         logger,
		requestHandler: middleware.NewRequestHandler(logger, dbManager.GetRedis()),
		rpcManager:     rpcManager,
	}
}

// RegisterRoutes 注册路由
func (h *FriendHandler) RegisterRoutes(mux *http.ServeMux) {
	mux.HandleFunc("/add_friend", h.requestHandler.HandleRequest(h.addFriend))
	mux.HandleFunc("/friend_list", h.requestHandler.HandleRequest(h.getFriendList))
	mux.HandleFunc("/friend_info", h.requestHandler.HandleRequest(h.getFriendInfo))
	mux.HandleFunc("/update_friend_remark", h.requestHandler.HandleRequest(h.updateFriendRemark))
	mux.HandleFunc("/set_friend_dnd", h.requestHandler.HandleRequest(h.setFriendDND))
	mux.HandleFunc("/delete_friend", h.requestHandler.HandleRequest(h.deleteFriend))
	mux.HandleFunc("/friend_request_list", h.requestHandler.HandleRequest(h.getFriendRequestList))
	mux.HandleFunc("/handle_friend_request", h.requestHandler.HandleRequest(h.handleFriendRequest))
}

// writeResp 写入响应
func (h *FriendHandler) writeResp(w http.ResponseWriter, code int, msg string, data []byte) {
	w.Header().Set("Content-Type", "application/x-protobuf")
	resp := &pb.APIResp{Code: int32(code), Msg: msg, Data: data}
	b, _ := proto.Marshal(resp)
	w.Write(b)
}

// helper: 调用用户服务获取单个用户名
func (h *FriendHandler) fetchUsernameByUID(uid string) string {
	ctx := context.Background()

	// 生成正确的 JWT token 来获取用户信息
	token, err := auth.GenerateToken(uid)
	if err != nil {
		h.logger.Errorf("生成token失败: %v", err)
		return ""
	}

	req := &pb.UserInfoReq{Token: token}
	resp, err := h.rpcManager.CallWithRetry(ctx, "user-service", "/user_info", req, 3)
	if err != nil {
		h.logger.Errorf("获取用户信息失败: %v", err)
		return ""
	}

	var api pb.APIResp
	if err := proto.Unmarshal(resp, &api); err != nil || api.Code != 0 {
		h.logger.Errorf("解析用户信息响应失败: %v", err)
		return ""
	}

	info := &pb.UserInfoResp{}
	if err := proto.Unmarshal(api.Data, info); err != nil {
		h.logger.Errorf("解析用户信息数据失败: %v", err)
		return ""
	}

	return info.Username
}

// fetchUserInfo 从用户服务获取用户名与邮箱（支持外部超时控制）
func (h *FriendHandler) fetchUserInfo(ctx context.Context, uid string) (string, string) {
	// 生成 JWT token
	token, err := auth.GenerateToken(uid)
	if err != nil {
		h.logger.Errorf("生成token失败: %v", err)
		return "", ""
	}
	req := &pb.UserInfoReq{Token: token}
	resp, err := h.rpcManager.CallWithRetry(ctx, "user-service", "/user_info", req, 2)
	if err != nil {
		h.logger.Errorf("获取用户信息失败: %v", err)
		return "", ""
	}
	var api pb.APIResp
	if err := proto.Unmarshal(resp, &api); err != nil || api.Code != 0 {
		h.logger.Errorf("解析用户信息响应失败: %v", err)
		return "", ""
	}
	info := &pb.UserInfoResp{}
	if err := proto.Unmarshal(api.Data, info); err != nil {
		h.logger.Errorf("解析用户信息数据失败: %v", err)
		return "", ""
	}
	return info.Username, info.Email
}

// helper: 调用消息服务推送通知
func (h *FriendHandler) notify(notif *pb.Notification) {
	ctx := context.Background()

	_, err := h.rpcManager.CallWithRetry(ctx, "message-service", "/notify", notif, 3)
	if err != nil {
		h.logger.Errorf("发送通知失败: %v", err)
	}
}

// addFriend 添加好友
func (h *FriendHandler) addFriend(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.AddFriendReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.FromUid == "" || req.ToUid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}

	// 新增：判断被添加用户是否存在
	username := h.fetchUsernameByUID(req.ToUid)
	if username == "" {
		h.writeResp(w, 1, "被添加用户不存在", nil)
		return
	}

	err = h.service.AddFriendRequest(req.FromUid, req.ToUid, req.VerifyMsg)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 推送好友请求通知（旧类型）
	fromUsername := h.fetchUsernameByUID(req.FromUid)
	n := &pb.Notification{Type: "friend_request", From: req.FromUid, FromUsername: fromUsername, To: req.ToUid, Content: req.VerifyMsg}
	h.notify(n)

	resp := &pb.AddFriendResp{Code: 0, Msg: "好友请求已发送"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getFriendList 获取好友列表
func (h *FriendHandler) getFriendList(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.FriendListReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Uid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}
	friends, err := h.service.GetFriends(req.Uid)
	if err != nil {
		h.writeResp(w, 1, "获取好友列表失败", nil)
		return
	}
	friendUsernames := make([]string, len(friends))
	remarks := make([]string, len(friends))
	ctx, cancel := context.WithTimeout(context.Background(), 1500*time.Millisecond)
	defer cancel()
	var wg sync.WaitGroup
	for i, fuid := range friends {
		idx := i
		uidCopy := fuid
		wg.Add(1)
		go func() {
			defer wg.Done()
			select {
			case <-ctx.Done():
				return
			default:
			}
			friendUsernames[idx] = h.fetchUsernameByUID(uidCopy)
		}()
		wg.Add(1)
		go func() {
			defer wg.Done()
			select {
			case <-ctx.Done():
				return
			default:
			}
			remark, _ := h.service.GetFriendRemark(req.Uid, uidCopy)
			remarks[idx] = remark
		}()
	}
	wg.Wait()
	resp := &pb.FriendListResp{FriendUids: friends, FriendUsernames: friendUsernames, Remarks: remarks, Code: 0, Msg: "ok"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getFriendInfo 获取好友信息
func (h *FriendHandler) getFriendInfo(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.FriendInfoReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Uid == "" || req.FriendUid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}

	// 并行获取 备注/DND 与 用户信息，并设置超时
	ctx, cancel := context.WithTimeout(context.Background(), 1200*time.Millisecond)
	defer cancel()
	var wg sync.WaitGroup
	var remark string
	var dnd bool
	var username string
	var email string

	wg.Add(1)
	go func() {
		defer wg.Done()
		select {
		case <-ctx.Done():
			return
		default:
		}
		remark, _ = h.service.GetFriendRemark(req.Uid, req.FriendUid)
		dnd, _ = h.service.GetFriendDND(req.Uid, req.FriendUid)
	}()

	wg.Add(1)
	go func() {
		defer wg.Done()
		select {
		case <-ctx.Done():
			return
		default:
		}
		username, email = h.fetchUserInfo(ctx, req.FriendUid)
	}()

	wg.Wait()

	resp := &pb.FriendInfoResp{
		Uid:      req.FriendUid,
		Username: username,
		Email:    email,
		Remark:   remark,
		Code:     0,
		Msg:      "ok",
		Dnd:      dnd,
	}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// updateFriendRemark 更新好友备注
func (h *FriendHandler) updateFriendRemark(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.UpdateFriendRemarkReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Uid == "" || req.FriendUid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}
	err = h.service.SetFriendRemark(req.Uid, req.FriendUid, req.Remark)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.UpdateFriendRemarkResp{Code: 0, Msg: "备注设置成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// setFriendDND 设置好友免打扰
func (h *FriendHandler) setFriendDND(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SetFriendDNDReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Uid == "" || req.FriendUid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}
	err = h.service.SetFriendDND(req.Uid, req.FriendUid, req.Dnd)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.SetFriendDNDResp{Code: 0, Msg: "设置成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// deleteFriend 删除好友
func (h *FriendHandler) deleteFriend(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.DeleteFriendReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Uid == "" || req.FriendUid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}
	err = h.service.DeleteFriend(req.Uid, req.FriendUid)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.DeleteFriendResp{Code: 0, Msg: "已删除"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getFriendRequestList 获取收到的好友请求列表
func (h *FriendHandler) getFriendRequestList(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.FriendListReq // 复用已有结构
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	// TODO: 校验token（如需）
	reqs, err := h.service.GetFriendRequests(req.Uid)
	if err != nil {
		h.writeResp(w, 1, "获取好友请求失败", nil)
		return
	}
	var fromUids, fromUsernames, msgs []string
	for from, msg := range reqs {
		fromUids = append(fromUids, from)
		msgs = append(msgs, msg)
		fromUsernames = append(fromUsernames, h.fetchUsernameByUID(from))
	}
	resp := &pb.FriendRequestListResp{FromUids: fromUids, FromUsernames: fromUsernames, VerifyMsgs: msgs, Code: 0, Msg: "ok"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// handleFriendRequest 处理好友请求
func (h *FriendHandler) handleFriendRequest(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.HandleFriendRequestReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.FromUid == "" || req.ToUid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}
	err = h.service.HandleFriendRequest(req.FromUid, req.ToUid, req.Accept)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.HandleFriendRequestResp{Code: 0, Msg: "处理成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}
