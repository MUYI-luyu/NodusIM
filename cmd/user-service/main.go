package main

import (
	"flag"
	"fmt"
	"net/http"
	"os"

	"im/internal/services/user-service/handler"
	"im/internal/services/user-service/service"
	"im/internal/services/user-service/storage"
	"im/internal/shared/config"
	"im/internal/shared/database"
	"im/internal/shared/discovery"
	"im/internal/shared/logger"
)

func main() {
	// 解析命令行参数
	var port = flag.Int("port", 0, "服务端口")
	flag.Parse()

	// 加载配置
	cfg := config.LoadServiceConfig("user-service")

	// 如果指定了端口，使用命令行参数
	if *port > 0 {
		cfg.Server.Port = *port
	}

	// 初始化日志
	logger := logger.NewLogger(cfg.Log.Level)

	// 初始化数据库连接池管理器
	dbManager, err := database.NewManager(cfg.Database, cfg.Redis, cfg.Mongo, logger)
	if err != nil {
		logger.Fatalf("初始化数据库连接池失败: %v", err)
	}
	defer dbManager.Close()

	// 初始化存储
	userStorage, err := storage.NewUserStorage(dbManager)
	if err != nil {
		logger.Fatalf("初始化存储失败: %v", err)
	}

	// 初始化服务
	userService := service.NewUserService(userStorage, logger)

	// 初始化处理器
	userHandler := handler.NewUserHandler(userService, logger, dbManager)

	// 启动时注册到etcd（如果提供了ETCD_ENDPOINTS）
	endpoints := os.Getenv("ETCD_ENDPOINTS")
	if endpoints == "" {
		endpoints = "localhost:2379"
	}
	disc, err := discovery.New(discovery.Config{Endpoints: []string{endpoints}})
	if err == nil {
		registrar := &discovery.Registrar{}
		ip := discovery.GetOutboundIP()
		_ = registrar.Register(disc, "/im/services", "user-service", ip, cfg.Server.Port, 10)
	} else {
		logger.Warnf("etcd连接失败，跳过服务注册: %v", err)
	}

	// 设置路由
	mux := http.NewServeMux()
	userHandler.RegisterRoutes(mux)

	// 启动服务
	logger.Infof("用户服务启动在端口 %d", cfg.Server.Port)
	server := &http.Server{
		Addr:    fmt.Sprintf(":%d", cfg.Server.Port),
		Handler: mux,
	}

	if err := server.ListenAndServe(); err != nil {
		logger.Fatalf("启动用户服务失败: %v", err)
		os.Exit(1)
	}
}
