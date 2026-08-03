package main

import (
	"fmt"
	"net/http"
	"os"

	"im/internal/shared/config"
	"im/internal/shared/database"
	"im/internal/shared/discovery"
	"im/internal/shared/logger"
)

func main() {
	cfg := config.LoadServiceConfig("group-service")
	log := logger.NewLogger(cfg.Log.Level)
	dbManager, err := database.NewManager(cfg.Database, cfg.Redis, cfg.Mongo, log)
	if err != nil {
		log.Fatalf("初始化数据库连接池失败: %v", err)
	}
	defer dbManager.Close()

	registerService(cfg.Server.Port, "group-service", log)
	log.Infof("群组服务基础壳启动在端口 %d", cfg.Server.Port)
	if err := http.ListenAndServe(fmt.Sprintf(":%d", cfg.Server.Port), http.NewServeMux()); err != nil {
		log.Fatalf("启动群组服务失败: %v", err)
	}
}

func registerService(port int, serviceName string, log *logger.Logger) {
	endpoints := os.Getenv("ETCD_ENDPOINTS")
	if endpoints == "" {
		endpoints = "localhost:2379"
	}
	disc, err := discovery.New(discovery.Config{Endpoints: []string{endpoints}})
	if err != nil {
		log.Warnf("etcd连接失败，跳过服务注册: %v", err)
		return
	}
	registrar := &discovery.Registrar{}
	_ = registrar.Register(disc, "/im/services", serviceName, discovery.GetOutboundIP(), port, 10)
}
