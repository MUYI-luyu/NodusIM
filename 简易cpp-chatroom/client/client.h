#pragma once
#include <memory>
#include "../socket/socket.h"
#include "../server/Reactor.h"
#include "./register/readdata.h"
#include "../socket/socket.h"
#include "./register/hashpwd.h"
#include <algorithm>
#include <cstring>
#include <stdlib.h>

#define CONPORT 4413


class  Client {
  public:
   Client();
  ~ Client();

  /**
   * @brief 获取当前连接的通信套接字
   * @return 返回TcpSocket指针
   */
  Socket* getSocket() const { return socket_.get(); }

  /**
   * @brief 连接到主机
   * @param ip 主机的IP地址
   * @param port 主机的端口号
   * @return 成功返回true，失败返回false
   */
  bool connectToHost(const char* ip, unsigned short port);

  //客户端运行应有功能
  void ctlthread(void);

  //在当前内存位置重新构造对象
  void reinitialize() {
    new (this) Client();
  }
  
  private:
  // 通信套接字
  int fd_;
  // 通信类对象
  std::unique_ptr<Socket> socket_;
  
};