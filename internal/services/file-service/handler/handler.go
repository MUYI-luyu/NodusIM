package handler

import (
	"fmt"
	"net/http"
	"strings"

	"im/internal/services/file-service/service"
	"im/internal/shared/database"
	"im/internal/shared/logger"
	"im/internal/shared/middleware"
	pb "im/internal/shared/protocol/pb"

	"google.golang.org/protobuf/proto"
)

// FileHandler 文件处理器
type FileHandler struct {
	requestHandler *middleware.RequestHandler
	service        *service.FileService
	logger         *logger.Logger
}

// NewFileHandler 创建文件处理器
func NewFileHandler(service *service.FileService, logger *logger.Logger, dbManager *database.Manager) *FileHandler {
	return &FileHandler{
		service:        service,
		logger:         logger,
		requestHandler: middleware.NewRequestHandler(logger, dbManager.GetRedis()),
	}
}

// RegisterRoutes 注册路由
func (h *FileHandler) RegisterRoutes(mux *http.ServeMux) {
	mux.HandleFunc("/upload", h.requestHandler.HandleRequest(h.uploadFile))
	mux.HandleFunc("/uploads/", h.requestHandler.HandleRequest(h.downloadFile))
}

// writeResp 写入响应
func (h *FileHandler) writeResp(w http.ResponseWriter, code int, msg string, data []byte) {
	w.Header().Set("Content-Type", "application/x-protobuf")
	resp := &pb.APIResp{Code: int32(code), Msg: msg, Data: data}
	b, _ := proto.Marshal(resp)
	w.Write(b)
}

// uploadFile 文件上传处理器
func (h *FileHandler) uploadFile(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		h.writeResp(w, 4001, "只支持POST方法", nil)
		return
	}

	// 解析multipart表单
	if err := r.ParseMultipartForm(50 * 1024 * 1024); err != nil {
		h.writeResp(w, 4002, "解析表单失败", nil)
		return
	}

	// 获取文件
	file, header, err := r.FormFile("file")
	if err != nil {
		h.writeResp(w, 4003, "获取文件失败", nil)
		return
	}
	defer file.Close()

	// 调用业务层处理文件上传
	fileInfo, err := h.service.UploadFile(file, header.Filename, header.Size)
	if err != nil {
		h.writeResp(w, 4004, err.Error(), nil)
		return
	}

	// 返回文件信息
	data, _ := proto.Marshal(fileInfo)
	h.writeResp(w, 0, "上传成功", data)
}

// downloadFile 文件下载处理器
func (h *FileHandler) downloadFile(w http.ResponseWriter, r *http.Request) {
	filename := strings.TrimPrefix(r.URL.Path, "/uploads/")
	if filename == "" {
		http.NotFound(w, r)
		return
	}

	// 调用业务层获取文件路径
	filePath, err := h.service.GetFilePath(filename)
	if err != nil {
		http.NotFound(w, r)
		return
	}

	// 设置响应头
	w.Header().Set("Content-Type", h.service.GetMimeType(filename))
	w.Header().Set("Content-Disposition", fmt.Sprintf("attachment; filename=%s", filename))

	// 发送文件
	http.ServeFile(w, r, filePath)
}

// Start 启动HTTP服务器
func (h *FileHandler) Start(port int) error {
	mux := http.NewServeMux()
	h.RegisterRoutes(mux)

	addr := fmt.Sprintf(":%d", port)
	h.logger.Infof("服务启动在端口 %d", port)

	return http.ListenAndServe(addr, mux)
}
