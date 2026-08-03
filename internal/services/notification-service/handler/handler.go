package handler

import (
	"fmt"
	"io"
	"net/http"

	"im/internal/services/notification-service/service"
	"im/internal/shared/logger"
	"im/internal/shared/middleware"
	pb "im/internal/shared/protocol/pb"

	"google.golang.org/protobuf/proto"
)

type NotificationHandler struct {
	requestHandler *middleware.RequestHandler
	svc            *service.NotificationService
	logger         *logger.Logger
}

func NewNotificationHandler(svc *service.NotificationService, logger *logger.Logger) *NotificationHandler {
	return &NotificationHandler{
		svc:            svc,
		logger:         logger,
		requestHandler: middleware.NewRequestHandler(logger, svc.GetRedis()),
	}
}

func (h *NotificationHandler) RegisterRoutes(mux *http.ServeMux) {
	mux.HandleFunc("/send_email_code", h.requestHandler.HandleRequest(h.sendEmailCode))
	mux.HandleFunc("/reset_password", h.requestHandler.HandleRequest(h.resetPassword))
	// 新增：注册验证码校验
	mux.HandleFunc("/verify_email_code_register", h.requestHandler.HandleRequest(h.verifyEmailCodeRegister))
}

func (h *NotificationHandler) writeResp(w http.ResponseWriter, code int, msg string, data []byte) {
	w.Header().Set("Content-Type", "application/x-protobuf")
	resp := &pb.APIResp{Code: int32(code), Msg: msg, Data: data}
	b, _ := proto.Marshal(resp)
	w.Write(b)
}

func (h *NotificationHandler) sendEmailCode(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.SendEmailCodeReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if req.Email == "" || req.Purpose == "" {
		h.writeResp(w, 1, "缺少参数", nil)
		return
	}
	if err := h.svc.SendEmailCode(req.Email, req.Purpose); err != nil {
		h.writeResp(w, 1, err.Error(), nil)
		return
	}
	h.writeResp(w, 0, "ok", nil)
}

func (h *NotificationHandler) resetPassword(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.ResetPasswordReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	// 校验验证码
	if ok := h.svc.VerifyEmailCode(req.Email, req.EmailCode, "reset_password"); !ok {
		h.writeResp(w, 1, "验证码无效", nil)
		return
	}
	// 实际重置密码应调用用户服务；此处仅返回成功
	h.writeResp(w, 0, "ok", nil)
}

// verifyEmailCodeRegister 校验注册验证码（protobuf-only）
func (h *NotificationHandler) verifyEmailCodeRegister(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		h.writeResp(w, 1, "请求体读取失败", nil)
		return
	}
	var req pb.ResetPasswordReq
	if err := proto.Unmarshal(body, &req); err != nil {
		h.writeResp(w, 1, "请求格式错误", nil)
		return
	}
	if ok := h.svc.VerifyEmailCode(req.Email, req.EmailCode, "register"); !ok {
		h.writeResp(w, 1, "验证码无效", nil)
		return
	}
	h.writeResp(w, 0, "ok", nil)
}

// Start 启动HTTP服务器
func (h *NotificationHandler) Start(port int) error {
	mux := http.NewServeMux()
	h.RegisterRoutes(mux)

	addr := fmt.Sprintf(":%d", port)
	h.logger.Infof("服务启动在端口 %d", port)

	return http.ListenAndServe(addr, mux)
}
