package config

import (
	"os"
	"strconv"

	"github.com/joho/godotenv"
)

// ServiceConfig 服务配置
type ServiceConfig struct {
	Server   ServerConfig   `json:"server"`
	Database DatabaseConfig `json:"database"`
	Redis    RedisConfig    `json:"redis"`
	Mongo    MongoConfig    `json:"mongo"`
	Log      LogConfig      `json:"log"`
	Email    EmailConfig    `json:"email"`
	File     FileConfig     `json:"file"`
}

// ServerConfig 服务器配置
type ServerConfig struct {
	Port int `json:"port"`
}

// DatabaseConfig 数据库配置
type DatabaseConfig struct {
	Host     string `json:"host"`
	Port     int    `json:"port"`
	Username string `json:"username"`
	Password string `json:"password"`
	Database string `json:"database"`
}

// RedisConfig Redis配置
type RedisConfig struct {
	Host     string `json:"host"`
	Port     int    `json:"port"`
	Password string `json:"password"`
	DB       int    `json:"db"`
}

// MongoConfig MongoDB配置
type MongoConfig struct {
	URI      string `json:"uri"`
	Database string `json:"database"`
}

// LogConfig 日志配置
type LogConfig struct {
	Level string `json:"level"`
}

// EmailConfig 邮件配置
type EmailConfig struct {
	Host      string `json:"host"`
	Port      int    `json:"port"`
	Username  string `json:"username"`
	Password  string `json:"password"`
	FromEmail string `json:"from_email"`
}

// FileConfig 文件配置
type FileConfig struct {
	UploadPath string `json:"upload_path"`
}

// LoadServiceConfig 加载服务配置
func LoadServiceConfig(serviceName string) *ServiceConfig {
	// 加载环境变量（优先根目录 config.env）
	// 尝试多个路径
	configPaths := []string{
		"config.env",
		"../../config.env",
		"../../../config.env",
	}

	var err error
	for _, path := range configPaths {
		err = godotenv.Load(path)
		if err == nil {
			break
		}
	}

	// 根据服务名设置不同的端口
	port := getPortForService(serviceName)

	// 如果设置了环境变量PORT，优先使用环境变量
	if envPort := getEnvAsInt("PORT", 0); envPort > 0 {
		port = envPort
	}

	return &ServiceConfig{
		Server: ServerConfig{
			Port: port,
		},
		Database: DatabaseConfig{
			Host:     getEnv("MYSQL_HOST", "localhost"),
			Port:     getEnvAsInt("MYSQL_PORT", 3306),
			Username: getEnv("MYSQL_USERNAME", "root"),
			Password: getEnv("MYSQL_PASSWORD", ""),
			Database: getEnv("MYSQL_DATABASE", "im_system"),
		},
		Redis: RedisConfig{
			Host:     getEnv("REDIS_HOST", "localhost"),
			Port:     getEnvAsInt("REDIS_PORT", 6379),
			Password: getEnv("REDIS_PASSWORD", ""),
			DB:       getEnvAsInt("REDIS_DB", 0),
		},
		Mongo: MongoConfig{
			URI:      getEnv("MONGODB_URI", "mongodb://localhost:27017"),
			Database: getEnv("MONGODB_DATABASE", "im_messages"),
		},
		Log: LogConfig{
			Level: getEnv("LOG_LEVEL", "info"),
		},
		Email: EmailConfig{
			Host:      firstNonEmpty(getEnv("SMTP_HOST", ""), getEnv("EMAIL_HOST", "smtp.gmail.com")),
			Port:      firstNonZero(getEnvAsInt("SMTP_PORT", 0), getEnvAsInt("EMAIL_PORT", 587)),
			Username:  firstNonEmpty(getEnv("SMTP_USERNAME", ""), getEnv("EMAIL_USERNAME", "")),
			Password:  firstNonEmpty(getEnv("SMTP_PASSWORD", ""), getEnv("EMAIL_PASSWORD", "")),
			FromEmail: firstNonEmpty(getEnv("FROM_EMAIL", ""), getEnv("EMAIL_FROM", "")),
		},
		File: FileConfig{
			UploadPath: firstNonEmpty(getEnv("UPLOAD_PATH", ""), getEnv("UPLOAD_DIR", "./uploads")),
		},
	}
}

// getPortForService 根据服务名获取端口
// 端口分配规划：
// 8080-8086: 单实例服务端口
// 8087: Traefik网关
// 8088: 前端界面
// 8090-8099: 用户服务多实例 (8090, 8091, 8092...)
// 8100-8109: 好友服务多实例 (8100, 8101, 8102...)
// 8110-8119: 群组服务多实例 (8110, 8111, 8112...)
// 8120-8129: 消息服务多实例 (8120, 8121, 8122...)
// 8130-8139: 文件服务多实例 (8130, 8131, 8132...)
// 8140-8149: 通知服务多实例 (8140, 8141, 8142...)
func getPortForService(serviceName string) int {
	ports := map[string]int{
		"user-service":         8090, // 改为8090，支持多实例
		"friend-service":       8100, // 改为8100，支持多实例
		"group-service":        8110, // 改为8110，支持多实例
		"message-service":      8120, // 改为8120，支持多实例
		"file-service":         8130, // 改为8130，支持多实例
		"notification-service": 8140, // 改为8140，支持多实例
		"gateway":              8080,
	}
	if port, exists := ports[serviceName]; exists {
		return port
	}
	return 8080
}

// getEnv 获取环境变量，如果不存在则返回默认值
func getEnv(key, defaultValue string) string {
	value := os.Getenv(key)
	if value != "" {
		return value
	}
	return defaultValue
}

// getEnvAsInt 获取环境变量并转换为整数
func getEnvAsInt(key string, defaultValue int) int {
	if value := os.Getenv(key); value != "" {
		if intValue, err := strconv.Atoi(value); err == nil {
			return intValue
		}
	}
	return defaultValue
}

// firstNonEmpty 返回第一个非空字符串，否则空串
func firstNonEmpty(values ...string) string {
	for _, v := range values {
		if v != "" {
			return v
		}
	}
	return ""
}

// firstNonZero 返回第一个非零整数，否则0
func firstNonZero(values ...int) int {
	for _, v := range values {
		if v != 0 {
			return v
		}
	}
	return 0
}
