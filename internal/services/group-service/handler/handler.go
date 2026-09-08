package handler

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"time"

	"im/internal/services/group-service/service"
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

// GroupHandler 群组处理器
type GroupHandler struct {
	requestHandler *middleware.RequestHandler
	service        *service.GroupService
	logger         *logger.Logger
	rpcManager     *rpc.Manager
}

// NewGroupHandler 创建群组处理器
func NewGroupHandler(service *service.GroupService, logger *logger.Logger, dbManager *database.Manager) *GroupHandler {
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

	return &GroupHandler{
		service:        service,
		logger:         logger,
		requestHandler: middleware.NewRequestHandler(logger, dbManager.GetRedis()),
		rpcManager:     rpcManager,
	}
}

// RegisterRoutes 注册路由
func (h *GroupHandler) RegisterRoutes(mux *http.ServeMux) {
	mux.HandleFunc("/create_group", h.requestHandler.HandleRequest(h.createGroup))
	mux.HandleFunc("/join_group", h.requestHandler.HandleRequest(h.joinGroup))
	mux.HandleFunc("/group_list", h.requestHandler.HandleRequest(h.getGroupList))
	mux.HandleFunc("/group_info", h.requestHandler.HandleRequest(h.getGroupInfo))
	mux.HandleFunc("/group_members", h.requestHandler.HandleRequest(h.getGroupMembers))
	mux.HandleFunc("/leave_group", h.requestHandler.HandleRequest(h.leaveGroup))
	mux.HandleFunc("/group_member_info", h.requestHandler.HandleRequest(h.getGroupMemberInfo))
	mux.HandleFunc("/invite_to_group", h.requestHandler.HandleRequest(h.inviteToGroup))
	mux.HandleFunc("/set_group_nickname", h.requestHandler.HandleRequest(h.setGroupNickname))
	mux.HandleFunc("/set_group_remark", h.requestHandler.HandleRequest(h.setGroupRemark))
	mux.HandleFunc("/set_group_dnd", h.requestHandler.HandleRequest(h.setGroupDND))
	mux.HandleFunc("/get_group_dnd", h.requestHandler.HandleRequest(h.getGroupDND))
	mux.HandleFunc("/set_group_mute", h.requestHandler.HandleRequest(h.setGroupMute))
	mux.HandleFunc("/kick_from_group", h.requestHandler.HandleRequest(h.kickFromGroup))
	mux.HandleFunc("/update_group_name", h.requestHandler.HandleRequest(h.updateGroupName))
	mux.HandleFunc("/set_group_admin", h.requestHandler.HandleRequest(h.setGroupAdmin))
	mux.HandleFunc("/dismiss_group", h.requestHandler.HandleRequest(h.dismissGroup))
	mux.HandleFunc("/group_request_list", h.requestHandler.HandleRequest(h.getGroupRequestList))
	mux.HandleFunc("/handle_group_request", h.requestHandler.HandleRequest(h.handleGroupRequest))
}

// writeResp 写入响应
func (h *GroupHandler) writeResp(w http.ResponseWriter, code int, msg string, data []byte) {
	w.Header().Set("Content-Type", "application/x-protobuf")
	resp := &pb.APIResp{Code: int32(code), Msg: msg, Data: data}
	b, _ := proto.Marshal(resp)
	w.Write(b)
}

// 内部: 推送通知到消息服务
func (h *GroupHandler) notify(to string, n *pb.Notification) {
	ctx := context.Background()

	_, err := h.rpcManager.CallWithRetry(ctx, "message-service", "/notify", n, 3)
	if err != nil {
		h.logger.Errorf("发送通知失败: %v", err)
	}
}

// createGroup 创建群组
func (h *GroupHandler) createGroup(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.CreateGroupReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Name == "" || req.OwnerUid == "" {
		h.writeResp(w, 1, "群组名称和群主UID不能为空", nil)
		return
	}
	groupID, err := h.service.CreateGroup(req.Name, req.OwnerUid)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.CreateGroupResp{Code: 0, Msg: "群组创建成功", GroupId: groupID}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// joinGroup 加入群组
func (h *GroupHandler) joinGroup(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.JoinGroupReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Uid == "" {
		h.writeResp(w, 1, "群组ID和UID不能为空", nil)
		return
	}
	// 检查群组是否存在
	g, err := h.service.GetGroup(req.GroupId)
	if err != nil {
		h.writeResp(w, 1, "群组不存在", nil)
		return
	}
	// 检查是否已有待审批的请求
	hasPending, _ := h.service.HasPendingInvite(req.GroupId, req.Uid)
	if hasPending {
		h.writeResp(w, 1, "你已提交过申请，请耐心等待审批", nil)
		return
	}
	// 插入入群申请
	err = h.service.InsertInviteRequest(req.GroupId, req.Uid, req.Uid)
	if err != nil {
		h.writeResp(w, 1, "申请入群失败: "+err.Error(), nil)
		return
	}
	// 通知所有管理员和群主有新的审批
	adminsAndOwner, _ := h.service.GetGroupAdminsAndOwner(req.GroupId)
	for _, adminUID := range adminsAndOwner {
		n := &pb.Notification{
			Type:      "group_application_pending",
			From:      req.Uid,
			To:        adminUID,
			GroupId:   g.GroupID,
			GroupName: g.Name,
			Content:   "",
			Extra:     "", // 只放特殊参数
		}
		h.notify(adminUID, n)
	}
	h.writeResp(w, 0, "入群申请已发送，请等待管理员审批", nil)
}

// getGroupList 获取群组列表
func (h *GroupHandler) getGroupList(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.GroupListReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Uid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}
	groups, err := h.service.GetUserGroups(req.Uid)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 转换为protobuf格式
	var pbGroups []*pb.Group
	for _, group := range groups {
		pbGroup := &pb.Group{
			GroupId:   group.GroupID,
			Name:      group.Name,
			OwnerUid:  group.OwnerUID,
			Remark:    group.Remark,    // 包含用户对该群组的备注
			CreatedAt: group.CreatedAt, // 已经是Unix时间戳
			UpdatedAt: group.UpdatedAt, // 已经是Unix时间戳
		}
		pbGroups = append(pbGroups, pbGroup)
	}
	resp := &pb.GroupListResp{Code: 0, Msg: "ok", Groups: pbGroups}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getGroupInfo 获取群组信息
func (h *GroupHandler) getGroupInfo(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.GroupInfoReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" {
		h.writeResp(w, 1, "缺少群组ID", nil)
		return
	}
	group, err := h.service.GetGroup(req.GroupId)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 获取群组成员列表
	members, err := h.service.GetGroupMembers(req.GroupId)
	if err != nil {
		h.writeResp(w, 1, "获取群组成员失败: "+err.Error(), nil)
		return
	}
	// 提取成员UID列表
	var memberUIDs []string
	for _, member := range members {
		memberUIDs = append(memberUIDs, member.UID)
	}
	pbGroup := &pb.Group{
		GroupId:    group.GroupID,
		Name:       group.Name,
		OwnerUid:   group.OwnerUID,
		MemberUids: memberUIDs,      // 填充成员UID列表
		CreatedAt:  group.CreatedAt, // 已经是Unix时间戳
		UpdatedAt:  group.UpdatedAt, // 已经是Unix时间戳
	}
	resp := &pb.GroupInfoResp{Code: 0, Msg: "ok", Group: pbGroup}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getGroupMembers 获取群组成员
func (h *GroupHandler) getGroupMembers(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.GroupMembersReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" {
		h.writeResp(w, 1, "缺少群组ID", nil)
		return
	}
	members, err := h.service.GetGroupMembers(req.GroupId)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 转换为protobuf格式
	var pbMembers []*pb.GroupMember
	for _, member := range members {
		// 获取用户名
		username := h.fetchUsernameByUID(member.UID)
		pbMember := &pb.GroupMember{
			Uid:      member.UID,
			Username: username,
			Nickname: member.Nickname,
			Role:     member.Role,
			JoinTime: member.JoinTime, // 已经是Unix时间戳
			Muted:    member.Muted,    // 禁言状态
		}
		pbMembers = append(pbMembers, pbMember)
	}
	resp := &pb.GroupMembersResp{Code: 0, Msg: "ok", Members: pbMembers}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// leaveGroup 离开群组
func (h *GroupHandler) leaveGroup(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.LeaveGroupReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Uid == "" {
		h.writeResp(w, 1, "群组ID和UID不能为空", nil)
		return
	}
	err = h.service.LeaveGroup(req.GroupId, req.Uid)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 通知离开（旧类型可选: group_member_left）
	g, _ := h.service.GetGroup(req.GroupId)
	n := &pb.Notification{Type: "group_member_left", From: req.Uid, GroupId: req.GroupId, GroupName: g.Name}
	// 通知群主
	h.notify(g.OwnerUID, n)
	resp := &pb.LeaveGroupResp{Code: 0, Msg: "退出群组成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getGroupMemberInfo 获取群成员信息
func (h *GroupHandler) getGroupMemberInfo(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.GroupMemberInfoReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Uid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	members, err := h.service.GetGroupMembers(req.GroupId)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 查找指定成员
	for _, member := range members {
		if member.UID == req.Uid {
			resp := &pb.GroupMemberInfoResp{
				Code:     0,
				Msg:      "ok",
				Uid:      member.UID,
				Username: "",
				Nickname: member.Nickname,
				Role:     member.Role,
				JoinTime: member.JoinTime, // 已经是Unix时间戳
			}
			data, _ := proto.Marshal(resp)
			h.writeResp(w, 0, "ok", data)
			return
		}
	}
	h.writeResp(w, 1, "成员不存在", nil)
}

// inviteToGroup 邀请入群
func (h *GroupHandler) inviteToGroup(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.InviteToGroupReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.InviterUid == "" || len(req.InviteeUids) == 0 {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 获取邀请者在群组中的角色
	role, err := h.service.GetUserRoleInGroup(req.GroupId, req.InviterUid)
	if err != nil {
		h.writeResp(w, 1, "无法获取邀请者身份", nil)
		return
	}
	g, _ := h.service.GetGroup(req.GroupId)
	for _, invitee := range req.InviteeUids {
		if role == "owner" || role == "admin" {
			// 管理员/群主直接添加成员
			err := h.service.JoinGroup(req.GroupId, invitee)
			if err != nil {
				h.writeResp(w, 1, "添加成员失败: "+invitee+":"+err.Error(), nil)
				return
			}
		} else {
			// 普通成员邀请需审批
			hasPending, _ := h.service.HasPendingInvite(req.GroupId, invitee)
			if hasPending {
				continue
			}
			err := h.service.InsertInviteRequest(req.GroupId, req.InviterUid, invitee)
			if err != nil {
				h.writeResp(w, 1, "邀请请求失败: "+invitee+":"+err.Error(), nil)
				return
			}
			// 通知群主/管理员待审批（旧类型: group_application_pending）
			n := &pb.Notification{Type: "group_application_pending", From: req.InviterUid, To: g.OwnerUID, GroupId: req.GroupId, GroupName: g.Name, Content: invitee}
			h.notify(g.OwnerUID, n)
		}
	}
	h.writeResp(w, 0, "邀请操作已处理", nil)
}

// setGroupNickname 设置群昵称
func (h *GroupHandler) setGroupNickname(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SetGroupNicknameReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Uid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 权限校验
	role, err := h.service.GetUserRoleInGroup(req.GroupId, req.Uid)
	if err != nil || role != "owner" {
		h.writeResp(w, 1, "只有群主可以设置群昵称", nil)
		return
	}
	err = h.service.SetGroupMemberNickname(req.GroupId, req.Uid, req.Nickname)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.SetGroupNicknameResp{Code: 0, Msg: "群昵称设置成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// setGroupRemark 设置群备注
func (h *GroupHandler) setGroupRemark(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SetGroupRemarkReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Uid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 检查用户是否是群组成员
	_, err = h.service.GetUserRoleInGroup(req.GroupId, req.Uid)
	if err != nil {
		h.writeResp(w, 1, "您不是该群组成员", nil)
		return
	}
	err = h.service.SetGroupMemberRemark(req.GroupId, req.Uid, req.Remark)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.SetGroupRemarkResp{Code: 0, Msg: "群备注设置成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// setGroupDND 设置群免打扰
func (h *GroupHandler) setGroupDND(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SetGroupDNDReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Uid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 权限校验：检查用户是否是群成员
	_, err = h.service.GetUserRoleInGroup(req.GroupId, req.Uid)
	if err != nil {
		h.writeResp(w, 1, "用户不是群成员", nil)
		return
	}
	err = h.service.SetGroupMemberDND(req.GroupId, req.Uid, req.Dnd)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.SetGroupDNDResp{Code: 0, Msg: "群免打扰设置成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getGroupDND 获取群免打扰状态
func (h *GroupHandler) getGroupDND(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SetGroupDNDReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.Uid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 权限校验：检查用户是否是群成员
	_, err = h.service.GetUserRoleInGroup(req.GroupId, req.Uid)
	if err != nil {
		h.writeResp(w, 1, "用户不是群成员", nil)
		return
	}
	dnd, err := h.service.GetGroupMemberDND(req.GroupId, req.Uid)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	resp := &pb.SetGroupDNDResp{Code: 0, Msg: "ok", Dnd: dnd}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// setGroupMute 设置群禁言
func (h *GroupHandler) setGroupMute(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SetGroupMuteReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.TargetUid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 权限校验
	operatorRole, err := h.service.GetUserRoleInGroup(req.GroupId, req.OperatorUid)
	if err != nil {
		h.writeResp(w, 1, "无法获取操作者身份", nil)
		return
	}
	targetRole, err := h.service.GetUserRoleInGroup(req.GroupId, req.TargetUid)
	if err != nil {
		h.writeResp(w, 1, "无法获取目标身份", nil)
		return
	}
	if req.TargetUid == req.OperatorUid {
		h.writeResp(w, 1, "不能禁言自己", nil)
		return
	}
	if operatorRole == "owner" {
		// 群主可以禁言任何人（除了自己）
	} else if operatorRole == "admin" {
		if targetRole != "member" {
			h.writeResp(w, 1, "管理员只能禁言普通成员，不能禁言群主或其他管理员", nil)
			return
		}
	} else {
		h.writeResp(w, 1, "无权限操作", nil)
		return
	}
	err = h.service.SetGroupMemberMute(req.GroupId, req.TargetUid, req.Mute)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 通知被处理者（旧类型: group_mute_change）
	g, _ := h.service.GetGroup(req.GroupId)
	n := &pb.Notification{Type: "group_mute_change", From: req.OperatorUid, To: req.TargetUid, GroupId: req.GroupId, GroupName: g.Name, Content: fmt.Sprintf("mute=%v", req.Mute)}
	h.notify(req.TargetUid, n)
	resp := &pb.SetGroupMuteResp{Code: 0, Msg: "群禁言设置成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// kickFromGroup 踢出群组
func (h *GroupHandler) kickFromGroup(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.KickFromGroupReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.OperatorUid == "" || req.TargetUid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 权限校验
	operatorRole, err := h.service.GetUserRoleInGroup(req.GroupId, req.OperatorUid)
	if err != nil {
		h.writeResp(w, 1, "无法获取操作者身份", nil)
		return
	}
	targetRole, err := h.service.GetUserRoleInGroup(req.GroupId, req.TargetUid)
	if err != nil {
		h.writeResp(w, 1, "无法获取目标身份", nil)
		return
	}
	if operatorRole == "admin" && targetRole != "member" {
		h.writeResp(w, 1, "管理员只能移除普通成员", nil)
		return
	}
	if operatorRole == "owner" && req.TargetUid == req.OperatorUid {
		h.writeResp(w, 1, "群主不能移除自己", nil)
		return
	}
	if operatorRole != "owner" && operatorRole != "admin" {
		h.writeResp(w, 1, "无权限操作", nil)
		return
	}
	// 先移除管理员权限（如果有）
	if targetRole == "admin" && operatorRole == "owner" {
		err := h.service.SetGroupMemberRole(req.GroupId, req.TargetUid, "member")
		if err != nil {
			h.writeResp(w, 1, "移除管理员权限失败: "+err.Error(), nil)
			return
		}
	}
	err = h.service.LeaveGroup(req.GroupId, req.TargetUid)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 通知被踢者（旧类型: group_kicked）
	g, _ := h.service.GetGroup(req.GroupId)
	n := &pb.Notification{Type: "group_kicked", From: req.OperatorUid, To: req.TargetUid, GroupId: req.GroupId, GroupName: g.Name}
	h.notify(req.TargetUid, n)
	h.writeResp(w, 0, "已移除成员", nil)
}

// updateGroupName 更新群名
func (h *GroupHandler) updateGroupName(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.UpdateGroupNameReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.NewName == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 权限校验
	role, err := h.service.GetUserRoleInGroup(req.GroupId, req.OperatorUid)
	if err != nil || role != "owner" {
		h.writeResp(w, 1, "只有群主可以修改群名", nil)
		return
	}
	err = h.service.UpdateGroupName(req.GroupId, req.NewName)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	// 通知所有成员（旧类型: group_name_updated）
	members, _ := h.service.GetGroupMembers(req.GroupId)
	for _, m := range members {
		n := &pb.Notification{Type: "group_name_updated", From: req.OperatorUid, To: m.UID, GroupId: req.GroupId, GroupName: req.NewName}
		h.notify(m.UID, n)
	}
	resp := &pb.UpdateGroupNameResp{Code: 0, Msg: "群名修改成功"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// setGroupAdmin 设置群管理员
func (h *GroupHandler) setGroupAdmin(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SetGroupAdminReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.OperatorUid == "" || req.TargetUid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	if req.OperatorUid == req.TargetUid {
		h.writeResp(w, 1, "不能设置自己为管理员", nil)
		return
	}
	role, err := h.service.GetUserRoleInGroup(req.GroupId, req.OperatorUid)
	if err != nil || role != "owner" {
		h.writeResp(w, 1, "只有群主可以设置管理员", nil)
		return
	}
	// 设置角色
	newRole := "member"
	if req.SetAdmin {
		newRole = "admin"
	}
	err = h.service.SetGroupMemberRole(req.GroupId, req.TargetUid, newRole)
	if err != nil {
		h.writeResp(w, 1, "设置失败: "+err.Error(), nil)
		return
	}
	msg := "已取消管理员"
	if req.SetAdmin {
		msg = "已设置为管理员"
	}
	// 通知被设置人（旧类型: group_admin_change）
	g, _ := h.service.GetGroup(req.GroupId)
	content := "已取消管理员权限"
	if req.SetAdmin {
		content = "已被设置为管理员"
	}
	n := &pb.Notification{Type: "group_admin_change", From: req.OperatorUid, To: req.TargetUid, GroupId: req.GroupId, GroupName: g.Name, Content: content}
	h.notify(req.TargetUid, n)
	resp := &pb.SetGroupAdminResp{Code: 0, Msg: msg}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// dismissGroup 解散群组
func (h *GroupHandler) dismissGroup(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.DismissGroupReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.GroupId == "" || req.OperatorUid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	// 权限校验：只有群主可以解散群组
	role, err := h.service.GetUserRoleInGroup(req.GroupId, req.OperatorUid)
	if err != nil || role != "owner" {
		h.writeResp(w, 1, "只有群主可以解散群组", nil)
		return
	}
	err = h.service.DisbandGroup(req.GroupId)
	if err != nil {
		h.writeResp(w, 1, "解散群组失败: "+err.Error(), nil)
		return
	}
	// 通知所有成员（旧类型: group_dismissed）
	members, _ := h.service.GetGroupMembers(req.GroupId)
	g, _ := h.service.GetGroup(req.GroupId)
	for _, m := range members {
		n := &pb.Notification{Type: "group_dismissed", From: req.OperatorUid, To: m.UID, GroupId: req.GroupId, GroupName: g.Name}
		h.notify(m.UID, n)
	}
	resp := &pb.DismissGroupResp{Code: 0, Msg: "群组已解散"}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// getGroupRequestList 获取群组请求列表
func (h *GroupHandler) getGroupRequestList(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.GroupRequestListReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Uid == "" {
		h.writeResp(w, 1, "缺少UID", nil)
		return
	}
	// 获取用户管理的群
	groups, err := h.service.GetUserGroups(req.Uid)
	if err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	var items []*pb.GroupRequestItem
	for _, group := range groups {
		// 检查用户是否是群主或管理员
		role, err := h.service.GetUserRoleInGroup(group.GroupID, req.Uid)
		if err != nil || (role != "owner" && role != "admin") {
			continue
		}
		reqs, err := h.service.GetPendingInviteRequests(group.GroupID)
		if err != nil {
			continue
		}
		for _, r := range reqs {
			item := &pb.GroupRequestItem{
				Id:              fmt.Sprintf("%d", r.ID),
				GroupId:         r.GroupID,
				GroupName:       group.Name,
				InviterUid:      r.InviterUID,
				InviteeUid:      r.InviteeUID,
				Status:          r.Status,
				CreatedAt:       time.Unix(r.CreatedAt, 0).Format("2006-01-02 15:04:05"),
				InviterUsername: "",
				InviteeUsername: "",
			}
			items = append(items, item)
		}
	}
	resp := &pb.GroupRequestListResp{
		Code:  0,
		Msg:   "ok",
		Items: items,
	}
	data, _ := proto.Marshal(resp)
	h.writeResp(w, 0, "ok", data)
}

// handleGroupRequest 处理群组请求
func (h *GroupHandler) handleGroupRequest(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.HandleGroupRequestReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Id == "" || req.GroupId == "" || req.InviteeUid == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	if req.Approve {
		err := h.service.ApproveInviteRequest(req.Id)
		if err != nil {
			h.writeResp(w, 1, err.Error(), nil)
			return
		}
		// 审批通过，添加成员
		err = h.service.JoinGroup(req.GroupId, req.InviteeUid)
		if err != nil {
			h.writeResp(w, 1, err.Error(), nil)
			return
		}
		// 通知被邀请者（旧类型: group_application_approved）
		g, _ := h.service.GetGroup(req.GroupId)
		n := &pb.Notification{Type: "group_application_approved", From: "", To: req.InviteeUid, GroupId: req.GroupId, GroupName: g.Name}
		h.notify(req.InviteeUid, n)
		h.writeResp(w, 0, "已同意并添加成员", nil)
		return
	} else {
		err := h.service.RejectInviteRequest(req.Id)
		if err != nil {
			h.writeResp(w, 1, err.Error(), nil)
			return
		}
		// 通知被邀请者（旧类型: group_application_rejected）
		g, _ := h.service.GetGroup(req.GroupId)
		n := &pb.Notification{Type: "group_application_rejected", From: "", To: req.InviteeUid, GroupId: req.GroupId, GroupName: g.Name}
		h.notify(req.InviteeUid, n)
		h.writeResp(w, 0, "已拒绝邀请", nil)
		return
	}
}

// fetchUsernameByUID 从用户服务获取用户名
func (h *GroupHandler) fetchUsernameByUID(uid string) string {
	ctx := context.Background()

	// 生成临时token用于内部调用
	tempToken, err := auth.GenerateToken(uid)
	if err != nil {
		h.logger.Errorf("生成token失败: %v", err)
		return ""
	}

	req := &pb.UserInfoReq{Token: tempToken}
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

	userResp := &pb.UserInfoResp{}
	if err := proto.Unmarshal(api.Data, userResp); err != nil {
		h.logger.Errorf("解析用户信息数据失败: %v", err)
		return ""
	}

	return userResp.Username
}

// Start 启动HTTP服务器
func (h *GroupHandler) Start(port int) error {
	mux := http.NewServeMux()
	h.RegisterRoutes(mux)

	addr := fmt.Sprintf(":%d", port)
	h.logger.Infof("服务启动在端口 %d", port)

	return http.ListenAndServe(addr, mux)
}
