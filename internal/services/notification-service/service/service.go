package service

import (
	"context"
	"encoding/json"
	"fmt"
	"math/rand"
	"strconv"
	"time"

	"im/internal/shared/config"
	"im/internal/shared/database"
	"im/internal/shared/logger"

	"github.com/redis/go-redis/v9"
	"gopkg.in/gomail.v2"
)

// EmailCode 邮箱验证码结构
type EmailCode struct {
	Code     string    `json:"code"`
	ExpireAt time.Time `json:"expire_at"`
	Purpose  string    `json:"purpose"` // "register" 或 "reset_password"
}

// NotificationService 通知服务
type NotificationService struct {
	logger   *logger.Logger
	emailCfg config.EmailConfig
	redis    *redis.Client
}

// NewNotificationService 创建通知服务实例
func NewNotificationService(logger *logger.Logger) *NotificationService {
	cfg := config.LoadServiceConfig("notification-service")

	// 初始化数据库连接池管理器
	dbManager, err := database.NewManager(cfg.Database, cfg.Redis, cfg.Mongo, logger)
	if err != nil {
		logger.Fatalf("初始化数据库连接池失败: %v", err)
	}

	return &NotificationService{
		logger:   logger,
		emailCfg: cfg.Email,
		redis:    dbManager.GetRedis(),
	}
}

// GetRedis exposes the existing Redis client for the notification HTTP middleware.
func (ns *NotificationService) GetRedis() *redis.Client {
	return ns.redis
}

// SendEmailCode 发送邮箱验证码
func (ns *NotificationService) SendEmailCode(email, purpose string) error {
	// 生成6位数字验证码
	code := generateCode()

	// 存储验证码到Redis（5分钟有效期）
	emailCode := EmailCode{
		Code:     code,
		ExpireAt: time.Now().Add(5 * time.Minute),
		Purpose:  purpose,
	}

	// 序列化验证码数据
	data, err := json.Marshal(emailCode)
	if err != nil {
		return fmt.Errorf("序列化验证码失败: %v", err)
	}

	// 存储到Redis，设置过期时间
	ctx := context.Background()
	key := fmt.Sprintf("email_code:%s", email)
	err = ns.redis.Set(ctx, key, data, 5*time.Minute).Err()
	if err != nil {
		return fmt.Errorf("存储验证码到Redis失败: %v", err)
	}

	// 构建邮件内容
	subject := "Chatroom 验证码"
	var body string
	switch purpose {
	case "register":
		body = fmt.Sprintf(`
			<h2>欢迎注册 Chatroom</h2>
			<p>您的验证码是：<strong style="font-size: 24px; color: #409eff;">%s</strong></p>
			<p>验证码有效期为5分钟，请尽快完成注册。</p>
			<p>如果这不是您的操作，请忽略此邮件。</p>
		`, code)
	case "reset_password":
		body = fmt.Sprintf(`
			<h2>Chatroom 密码重置</h2>
			<p>您的验证码是：<strong style="font-size: 24px; color: #409eff;">%s</strong></p>
			<p>验证码有效期为5分钟，请尽快完成密码重置。</p>
			<p>如果这不是您的操作，请忽略此邮件。</p>
		`, code)
	}

	// 发送邮件
	err = ns.sendEmail(email, subject, body)
	if err != nil {
		// 发送失败，删除存储的验证码
		ns.redis.Del(ctx, key)
		return fmt.Errorf("发送邮件失败: %v", err)
	}

	ns.logger.Info("邮件验证码发送成功: " + email)
	return nil
}

// VerifyEmailCode 验证邮箱验证码
func (ns *NotificationService) VerifyEmailCode(email, code, purpose string) bool {
	ctx := context.Background()
	key := fmt.Sprintf("email_code:%s", email)

	// 从Redis获取验证码
	data, err := ns.redis.Get(ctx, key).Result()
	if err != nil {
		if err == redis.Nil {
			// 验证码不存在
			return false
		}
		ns.logger.Errorf("从Redis获取验证码失败: %v", err)
		return false
	}

	// 反序列化验证码数据
	var emailCode EmailCode
	err = json.Unmarshal([]byte(data), &emailCode)
	if err != nil {
		ns.logger.Errorf("反序列化验证码失败: %v", err)
		return false
	}

	// 检查是否过期
	if time.Now().After(emailCode.ExpireAt) {
		// 过期，删除验证码
		ns.redis.Del(ctx, key)
		return false
	}

	// 检查验证码和用途是否匹配
	if emailCode.Code != code || emailCode.Purpose != purpose {
		return false
	}

	// 验证成功后删除验证码
	ns.redis.Del(ctx, key)
	return true
}

// sendEmail 发送邮件
func (ns *NotificationService) sendEmail(to, subject, body string) error {
	host := ns.emailCfg.Host
	port := ns.emailCfg.Port
	username := ns.emailCfg.Username
	password := ns.emailCfg.Password

	m := gomail.NewMessage()
	m.SetHeader("From", username)
	m.SetHeader("To", to)
	m.SetHeader("Subject", subject)
	m.SetBody("text/html", body)

	d := gomail.NewDialer(host, port, username, password)
	return d.DialAndSend(m)
}

// generateCode 生成6位数字验证码
func generateCode() string {
	rand.Seed(time.Now().UnixNano())
	code := rand.Intn(900000) + 100000 // 生成100000-999999之间的数字
	return strconv.Itoa(code)
}
