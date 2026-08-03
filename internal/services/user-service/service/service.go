package service

import (
	"context"
	"fmt"

	"im/internal/services/user-service/model"
	"im/internal/services/user-service/storage"
	"im/internal/shared/discovery"
	"im/internal/shared/logger"
	"im/internal/shared/protocol/pb"
	"im/internal/shared/rpc"
	"os"
	"strings"

	"google.golang.org/protobuf/proto"
)

// UserService 用户服务
type UserService struct {
	storage    storage.UserStorage
	logger     *logger.Logger
	rpcManager *rpc.Manager
}

// NewUserService 创建用户服务实例
func NewUserService(storage storage.UserStorage, logger *logger.Logger) *UserService {
	// 创建RPC管理器
	rpcManager := rpc.NewManager(logger)
	endpoints := os.Getenv("ETCD_ENDPOINTS")
	if endpoints == "" {
		endpoints = "localhost:2379"
	}
	disc, err := discovery.New(discovery.Config{Endpoints: strings.Split(endpoints, ",")})
	if err != nil {
		logger.Fatalf("etcd 连接失败，无法启动服务发现: %v", err)
	}
	rpcManager.UseEtcd(disc, "/im/services")
	_ = rpcManager.WatchService("notification-service")

	return &UserService{
		storage:    storage,
		logger:     logger,
		rpcManager: rpcManager,
	}
}

// CreateUser 创建用户
func (s *UserService) CreateUser(username, password, email string) (string, error) {
	s.logger.Infof("创建用户: %s, %s", username, email)

	// 验证输入
	if len(username) < 3 {
		return "", fmt.Errorf("用户名长度不能少于3个字符")
	}
	if len(password) < 6 {
		return "", fmt.Errorf("密码长度不能少于6个字符")
	}
	if len(email) < 5 {
		return "", fmt.Errorf("邮箱格式不正确")
	}

	// 检查用户名是否已存在
	_, err := s.storage.GetUserByEmail(email)
	if err == nil {
		return "", fmt.Errorf("该邮箱已被注册")
	}

	// 创建用户
	uid, err := s.storage.CreateUser(username, password, email)
	if err != nil {
		s.logger.Errorf("创建用户失败: %v", err)
		return "", err
	}

	s.logger.Infof("用户创建成功: %s", uid)
	return uid, nil
}

// GetUserByUID 根据UID获取用户
func (s *UserService) GetUserByUID(uid string) (*model.User, error) {
	s.logger.Debugf("获取用户信息: %s", uid)
	return s.storage.GetUserByUID(uid)
}

// GetUserByEmail 根据邮箱获取用户
func (s *UserService) GetUserByEmail(email string) (*model.User, error) {
	s.logger.Debugf("根据邮箱获取用户: %s", email)
	return s.storage.GetUserByEmail(email)
}

// CheckPassword 检查密码
func (s *UserService) CheckPassword(uid, password string) (bool, error) {
	s.logger.Debugf("检查用户密码: %s", uid)
	return s.storage.CheckPassword(uid, password)
}

// UpdatePassword 更新密码
func (s *UserService) UpdatePassword(uid, password string) error {
	s.logger.Infof("更新用户密码: %s", uid)

	if len(password) < 6 {
		return fmt.Errorf("密码长度不能少于6个字符")
	}

	return s.storage.UpdatePassword(uid, password)
}

// UpdateUsername 更新用户名
func (s *UserService) UpdateUsername(uid, username string) error {
	s.logger.Infof("更新用户名: %s -> %s", uid, username)

	if len(username) < 3 {
		return fmt.Errorf("用户名长度不能少于3个字符")
	}

	return s.storage.UpdateUsername(uid, username)
}

// DeleteUser 删除用户
func (s *UserService) DeleteUser(uid string) error {
	s.logger.Infof("删除用户: %s", uid)
	return s.storage.DeleteUser(uid)
}

// VerifyEmailCode 验证邮箱验证码
func (s *UserService) VerifyEmailCode(email, code, purpose string) bool {
	req := &pb.ResetPasswordReq{
		Email:     email,
		EmailCode: code,
	}

	resp, err := s.rpcManager.CallWithRetry(context.Background(), "notification-service", "/verify_email_code_register", req, 3)
	if err != nil {
		s.logger.Errorf("调用通知服务失败: %v", err)
		return false
	}

	var apiResp pb.APIResp
	if err := proto.Unmarshal(resp, &apiResp); err != nil {
		s.logger.Errorf("解析响应失败: %v", err)
		return false
	}

	return apiResp.Code == 0
}
