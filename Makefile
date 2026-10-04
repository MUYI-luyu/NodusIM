# NodusIM Makefile
# 提供核心的构建、启动、停止、清理功能

# 变量定义
BUILD_DIR=bin

# 颜色定义
RED=\033[0;31m
GREEN=\033[0;32m
YELLOW=\033[1;33m
BLUE=\033[0;34m
NC=\033[0m

# 默认目标
.DEFAULT_GOAL := help

# 帮助信息
.PHONY: help
help: ## 显示帮助信息
	@echo "$(BLUE)NodusIM Makefile$(NC)"
	@echo ""
	@echo "$(GREEN)可用命令:$(NC)"
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  $(YELLOW)%-15s$(NC) %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@echo ""
	@echo "$(GREEN)示例:$(NC)"
	@echo "  make build    # 构建所有微服务"
	@echo "  make start    # 启动所有服务"
	@echo "  make stop     # 停止所有服务"
	@echo "  make clean    # 清理构建文件"

# 构建所有微服务
.PHONY: build
build: ## 构建所有微服务
	@echo "$(BLUE)构建所有微服务...$(NC)"
	@chmod +x scripts/*.sh
	@scripts/build.sh

# 启动所有服务
.PHONY: start
start: ## 启动所有服务
	@echo "$(BLUE)启动 NodusIM 服务...$(NC)"
	@chmod +x scripts/*.sh
	@scripts/start.sh

# 停止所有服务
.PHONY: stop
stop: ## 停止所有服务
	@echo "$(BLUE)停止 NodusIM 服务...$(NC)"
	@scripts/stop.sh

# 清理构建文件和临时文件
.PHONY: clean
clean: ## 清理构建文件和临时文件
	@echo "$(BLUE)清理构建文件...$(NC)"
	rm -rf $(BUILD_DIR)
	rm -rf scripts/pids scripts/logs
	@echo "$(GREEN)清理完成$(NC)"
