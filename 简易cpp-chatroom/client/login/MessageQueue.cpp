#include "MessageQueue.h"

bool recv_running = false;

// 全局实例
MessageQueue EchoMsgQueue;
MessageQueue ReptMsgQueue;
MessageQueue ChatMsgQueue;
MessageQueue UserMsgQueue;
MessageQueue CtspMsgQueue;

void MessageQueue::push(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    queue_.push(msg);
    cv_.notify_one();  // 唤醒一个等待线程
}

std::string MessageQueue::wait_and_pop(int seconds) {
    std::unique_lock<std::mutex> lock(mtx_);
    bool has_data = cv_.wait_for(lock, std::chrono::seconds(seconds), [this] { return !queue_.empty(); });
    if(!has_data) {
        return "time_out";  // 如果超时则返回 "time_out"
    }
    std::string msg = queue_.front();
    queue_.pop();
    return msg;
}

bool MessageQueue::try_pop(std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (queue_.empty()) return false;
    msg = queue_.front();
    queue_.pop();
    return true;
}

bool MessageQueue::empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.empty();
}

bool MessageQueue::has_data() const {
    return !empty();  // 直接返回队列是否为空的反向
}

bool MessageQueue::clear() {
    std::lock_guard<std::mutex> lock(mtx_);
    while (!queue_.empty()) {
        queue_.pop();
    }
    return true;
}

void recv_thread(Socket* sock) {
    std::string msg;
    while (recv_running) {
        if (sock->recvMsg(msg) != 0) {
            continue;
        }
        if (msg.size() < 5) {
            continue;
        }
        if(msg[0] == 'e' && msg[1] == 'c' && msg[2] == 'h' && msg[3] == 'o' && msg[4] == ':')
            EchoMsgQueue.push(msg.c_str() + 5);    // 放入队列
        else if(msg[0] == 'r' && msg[1] == 'e' && msg[2] == 'p' && msg[3] == 't' && msg[4] == ':')
            ReptMsgQueue.push(msg.c_str() + 5);    // 放入队列
        else if(msg[0] == 'u' && msg[1] == 's' && msg[2] == 'e' && msg[3] == 'r' && msg[4] == ':'){
            UserMsgQueue.clear();
            UserMsgQueue.push(msg.c_str() + 5);    // 放入队列
        }
        else if(msg[0] == 'c' && msg[1] == 'h' && msg[2] == 'a' && msg[3] == 't' && msg[4] == ':')
            ChatMsgQueue.push(msg.c_str() + 5);    // 放入队列
        else if(msg[0] == 'c' && msg[1] == 't' && msg[2] == 's' && msg[3] == 'p' && msg[4] == ':')
            CtspMsgQueue.push(msg.c_str() + 5);    // 放入队列
        else if(msg[0] == 'l' && msg[1] == 'g' && msg[2] == 'e' && msg[3] == 'x' && msg[4] == ':'){
            system("clear");
            fflush(stdout);
            printf("\033[0;31m您的账号在别处登录!\033[0m\n");
            sock->mshutdown();
            exit(1);
        }
        else {
            printf("\033[0;31m服务器发来错误前缀的消息！！！\033[0m\n");
            printf("\033[0;31m读到消息msg:%s\033[0m\n", msg.c_str());
            continue;
        }
    }
}