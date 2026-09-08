package main

import (
	"fmt"
	"net/http"
	"os"

	"im/internal/services/message-service/handler"
	"im/internal/services/message-service/service"
	"im/internal/services/message-service/storage"
	"im/internal/shared/config"
	"im/internal/shared/database"
	"im/internal/shared/discovery"
	"im/internal/shared/logger"
)

func main() {
	// 加载配置
	cfg := config.LoadServiceConfig("message-service")

	// 初始化日志
	logger := logger.NewLogger(cfg.Log.Level)

	// 初始化数据库连接池管理器
	dbManager, err := database.NewManager(cfg.Database, cfg.Redis, cfg.Mongo, logger)
	if err != nil {
		logger.Fatalf("初始化数据库连接池失败: %v", err)
	}
	defer dbManager.Close()

	// 初始化存储
	chatStorage, err := storage.NewMessageStorage(dbManager)
	if err != nil {
		logger.Fatalf("初始化存储失败: %v", err)
	}

	// 初始化服务
	chatService := service.NewMessageService(chatStorage, logger)

	// 初始化处理器
	chatHandler := handler.NewMessageHandler(chatService, logger, dbManager)

	// 启动时注册到etcd（如果提供了ETCD_ENDPOINTS）
	endpoints := os.Getenv("ETCD_ENDPOINTS")
	if endpoints == "" {
		endpoints = "localhost:2379"
	}
	disc, err := discovery.New(discovery.Config{Endpoints: []string{endpoints}})
	if err == nil {
		registrar := &discovery.Registrar{}
		ip := discovery.GetOutboundIP()
		_ = registrar.Register(disc, "/im/services", "message-service", ip, cfg.Server.Port, 10)
	} else {
		logger.Warnf("etcd连接失败，跳过服务注册: %v", err)
	}

	// 设置路由
	mux := http.NewServeMux()
	chatHandler.RegisterRoutes(mux)

	// 启动服务
	logger.Infof("消息服务启动在端口 %d", cfg.Server.Port)
	server := &http.Server{
		Addr:    fmt.Sprintf(":%d", cfg.Server.Port),
		Handler: mux,
	}

	if err := server.ListenAndServe(); err != nil {
		logger.Fatalf("启动消息服务失败: %v", err)
		os.Exit(1)
	}
}
