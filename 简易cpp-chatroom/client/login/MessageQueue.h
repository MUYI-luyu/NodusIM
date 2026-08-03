#pragma once
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <queue>
#include "../../socket/socket.h"
#include "../register/readdata.h"

class MessageQueue {
public:
    // 队列操作
    void push(const std::string& msg);
    std::string wait_and_pop(int seconds = 30);
    bool try_pop(std::string& msg);
    bool empty() const;
    bool has_data() const;
    bool clear();

private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::string> queue_;
};

extern bool recv_running;
void recv_thread(Socket* sock);

// 外部声明全局 MessageQueue 实例
extern MessageQueue EchoMsgQueue;
extern MessageQueue ReptMsgQueue;
extern MessageQueue ChatMsgQueue;
extern MessageQueue UserMsgQueue;
extern MessageQueue CtspMsgQueue;
