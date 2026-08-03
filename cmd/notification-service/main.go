package main

import (
	"fmt"
	"net/http"
	"os"

	"im/internal/services/notification-service/handler"
	"im/internal/services/notification-service/service"
	"im/internal/shared/config"
	"im/internal/shared/discovery"
	"im/internal/shared/logger"
)

func main() {
	// 加载配置
	cfg := config.LoadServiceConfig("notification-service")

	// 初始化日志
	logger := logger.NewLogger(cfg.Log.Level)

	// 初始化服务
	notificationService := service.NewNotificationService(logger)

	// 初始化处理器
	notificationHandler := handler.NewNotificationHandler(notificationService, logger)

	// 启动时注册到etcd（如果提供了ETCD_ENDPOINTS）
	endpoints := os.Getenv("ETCD_ENDPOINTS")
	if endpoints == "" {
		endpoints = "localhost:2379"
	}
	disc, err := discovery.New(discovery.Config{Endpoints: []string{endpoints}})
	if err == nil {
		registrar := &discovery.Registrar{}
		ip := discovery.GetOutboundIP()
		_ = registrar.Register(disc, "/im/services", "notification-service", ip, cfg.Server.Port, 10)
	} else {
		logger.Warnf("etcd连接失败，跳过服务注册: %v", err)
	}

	// 设置路由
	mux := http.NewServeMux()
	notificationHandler.RegisterRoutes(mux)

	// 启动服务
	logger.Infof("通知服务启动在端口 %d", cfg.Server.Port)
	server := &http.Server{
		Addr:    fmt.Sprintf(":%d", cfg.Server.Port),
		Handler: mux,
	}

	if err := server.ListenAndServe(); err != nil {
		logger.Fatalf("启动通知服务失败: %v", err)
		os.Exit(1)
	}
}
