package service

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	"im/internal/shared/logger"
	pb "im/internal/shared/protocol/pb"
)

const (
	UploadDir   = "./uploads"
	MaxFileSize = 50 * 1024 * 1024 // 50MB
)

// FileService 文件服务
type FileService struct {
	logger *logger.Logger
}

// NewFileService 创建文件服务实例
func NewFileService(logger *logger.Logger) *FileService {
	// 确保上传目录存在
	if err := os.MkdirAll(UploadDir, 0755); err != nil {
		logger.Errorf("创建上传目录失败: %v", err)
		os.Exit(1)
	}
	return &FileService{
		logger: logger,
	}
}

// UploadFile 上传文件
func (fs *FileService) UploadFile(file io.Reader, filename string, size int64) (*pb.FileInfo, error) {
	// 检查文件大小
	if size > MaxFileSize {
		return nil, fmt.Errorf("文件太大，最大支持50MB")
	}

	// 检查文件类型
	fileType := fs.getFileType(filename)
	if fileType == "unknown" {
		return nil, fmt.Errorf("不支持的文件类型")
	}

	// 生成唯一文件名
	ext := filepath.Ext(filename)
	uniqueFilename := fmt.Sprintf("%d_%s%s", time.Now().UnixNano(), fs.generateRandomString(8), ext)
	filePath := filepath.Join(UploadDir, uniqueFilename)

	// 创建文件
	dst, err := os.Create(filePath)
	if err != nil {
		return nil, fmt.Errorf("创建文件失败: %v", err)
	}
	defer dst.Close()

	// 复制文件内容
	if _, err := io.Copy(dst, file); err != nil {
		return nil, fmt.Errorf("保存文件失败: %v", err)
	}

	fs.logger.Info("文件上传成功: " + filename + " -> " + uniqueFilename)

	// 返回文件信息
	fileInfo := &pb.FileInfo{
		Filename:     uniqueFilename,
		OriginalName: filename,
		Size:         size,
		Type:         fileType,
		Url:          fmt.Sprintf("/uploads/%s", uniqueFilename),
	}

	return fileInfo, nil
}

// GetFilePath 获取文件路径
func (fs *FileService) GetFilePath(filename string) (string, error) {
	filePath := filepath.Join(UploadDir, filename)

	// 检查文件是否存在
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		return "", fmt.Errorf("文件不存在")
	}

	return filePath, nil
}

// GetMimeType 获取MIME类型
func (fs *FileService) GetMimeType(filename string) string {
	ext := strings.ToLower(filepath.Ext(filename))
	switch ext {
	case ".jpg", ".jpeg":
		return "image/jpeg"
	case ".png":
		return "image/png"
	case ".gif":
		return "image/gif"
	case ".pdf":
		return "application/pdf"
	case ".txt":
		return "text/plain"
	case ".md", ".markdown":
		return "text/markdown"
	case ".zip":
		return "application/zip"
	default:
		return "application/octet-stream"
	}
}

// getFileType 获取文件类型
func (fs *FileService) getFileType(filename string) string {
	ext := strings.ToLower(filepath.Ext(filename))
	switch ext {
	case ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp":
		return "image"
	case ".txt", ".doc", ".docx", ".pdf", ".xls", ".xlsx", ".ppt", ".pptx", ".md", ".markdown":
		return "document"
	case ".mp3", ".wav", ".flac", ".aac":
		return "audio"
	case ".mp4", ".avi", ".mov", ".wmv":
		return "video"
	case ".zip", ".rar", ".7z", ".tar", ".gz":
		return "archive"
	default:
		return "unknown"
	}
}

// generateRandomString 生成随机字符串
func (fs *FileService) generateRandomString(length int) string {
	const charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	b := make([]byte, length)
	for i := range b {
		b[i] = charset[time.Now().UnixNano()%int64(len(charset))]
	}
	return string(b)
}
