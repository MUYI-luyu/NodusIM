#include "sendrecv.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <errno.h>

std::unordered_map<int, std::unique_ptr<std::mutex>> fd_write_mutexes;
std::unordered_map<int, std::unique_ptr<std::mutex>> fd_read_mutexes;

std::unordered_map<std::string, int> uid_to_socket;
std::unordered_map<int, std::string> socket_to_uid;

bool send_all(int sockfd, const void *buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    while (len > 0) {
        int n = send(sockfd, p, len, 0);
        if (n <= 0) {
            perror("send failed:");
            return false;
        }
        p += n;
        len -= n;
    }
    return true;
}

int sendMsg(std::string msg, int sockfd_) {
    uint32_t len = htonl(msg.size());
    auto it = fd_write_mutexes.find(sockfd_);
    if (it != fd_write_mutexes.end()) {
        std::lock_guard<std::mutex> lock(*(it->second));
        if (!send_all(sockfd_, &len, sizeof len)) return -1;
        if (!send_all(sockfd_, msg.data(), msg.size())) return -1;
    }
    return 0;
}

int recv_all(int sockfd, void *buf, size_t len) {
    char* p = static_cast<char*>(buf);
    int n;
    while (len > 0) {
        do {
            n = recv(sockfd, p, len, 0);
            if (n > 0) { p += n; len -= n; }
            else if (n == 0) {
                len = 0;
                return 10;
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK) return false;
        } while (n > 0);
    }
    return true;
}

int recvMsg(std::string &msg, int sockfd_) {
    uint32_t len, slen;
    auto it = fd_read_mutexes.find(sockfd_);
    if (it != fd_read_mutexes.end()) {
        std::lock_guard<std::mutex> lock(*(it->second));
        int ret = recv_all(sockfd_, &len, sizeof len);
        if (!ret) return -1;
        else if (ret == 10) return 10;
        slen = ntohl(len);
        msg.clear();
        msg.resize(slen);
        ret = recv_all(sockfd_, msg.data(), slen);
        if (!ret) return -1;
        else if (ret == 10) return 10;
    }
    return 0;
}

int recvfull(std::string &str, int sockfd) {
    char buf[4096];
    while (true) {
        ssize_t n = recv(sockfd, buf, sizeof(buf), 0);
        if (n > 0) {
            str.append(buf, n);
        } else if (n == 0) {
            return 10;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
    }
    return 0;
}

bool send_allfile(int sockfd, const void *buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    int retry = 0;
    const int MAX_RETRY = 100000;

    while (len > 0) {
        int n = send(sockfd, p, len, 0);
        if (n > 0) {
            p += n;
            len -= n;
            retry = 0;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (++retry > MAX_RETRY) return false;
            
            fd_set write_fds;
            FD_ZERO(&write_fds);
            FD_SET(sockfd, &write_fds);

            struct timeval timeout = {0, 1000}; 
            int ret = select(sockfd + 1, NULL, &write_fds, NULL, &timeout);

            if (ret <= 0) {
                continue;
            }
        } else {
            return false;
        }
    }
    return true;
}

int sendFILE(const std::string &msg, int sockfd_) {
    uint32_t len = htonl(msg.size());
    if (!send_allfile(sockfd_, &len, sizeof len)) return -1;
    if (!send_allfile(sockfd_, msg.data(), msg.size())) return -1;
    return 0;
}
