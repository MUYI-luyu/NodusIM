#pragma once
#include <glog/logging.h>
#include <string>
static std::string g_program_name;

class Logger {
public:
    // 初始化日志系统
    static void init(const std::string& program_name, const std::string& log_dir = "./logs") {
        g_program_name = program_name;
        // 设置日志输出目录
        google::InitGoogleLogging(g_program_name.c_str());
        FLAGS_log_dir = log_dir;

        // 同时输出到 stderr（终端）
        FLAGS_alsologtostderr = true;

        // 日志文件按级别分别写（INFO, WARNING, ERROR, FATAL）
        FLAGS_logbufsecs = 0;        // 立即刷新
        FLAGS_max_log_size = 50;     // 单个日志文件最大 50MB
        FLAGS_stop_logging_if_full_disk = true;

        // 创建目录（防止不存在）
        std::string cmd = "mkdir -p " + log_dir;
        system(cmd.c_str());
    }

    // 关闭日志系统
    static void shutdown() {
        google::ShutdownGoogleLogging();
    }
};

// 日志级别宏
#define LOG_INFO(...)  LOG(INFO)    << __VA_ARGS__
#define LOG_WARN(...)  LOG(WARNING) << __VA_ARGS__
#define LOG_ERROR(...) LOG(ERROR)   << __VA_ARGS__
#define LOG_FATAL(...) LOG(FATAL)   << __VA_ARGS__
