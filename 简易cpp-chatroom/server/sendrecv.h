#pragma once
#include <unordered_map>
#include <mutex>
#include <string>
#include <memory>

// 外部声明共享的变量
extern std::unordered_map<int, std::unique_ptr<std::mutex>> fd_write_mutexes;
extern std::unordered_map<int, std::unique_ptr<std::mutex>> fd_read_mutexes;
extern std::unordered_map<int, time_t> uslast_active;

extern std::unordered_map<std::string, int> uid_to_socket;
extern std::unordered_map<int, std::string> socket_to_uid;

// 声明函数
extern bool send_all(int sockfd, const void *buf, size_t len);
extern int sendMsg(std::string msg, int sockfd_);
extern int recv_all(int sockfd, void *buf, size_t len);
extern int recvMsg(std::string &msg, int sockfd_);
extern int recvfull(std::string &str, int sockfd);
extern bool send_allfile(int sockfd, const void *buf, size_t len);
extern int sendFILE(const std::string &msg, int sockfd_);
