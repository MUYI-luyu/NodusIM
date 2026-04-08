#include "../socket/socket.h"
#include <thread>
#include "server.h"

extern std::string server_ip_;
extern uint16_t server_port_;
extern std::mutex map_mutex;                                   // 访问映射时用的互斥锁


const int HEARTBEAT_TIMEOUT = 90; // 90秒无心跳就断开

void heartbeatMonitorThread() {
    while (true) {
        time_t now = time(nullptr);

        // 访问映射需加锁，防止并发读写冲突
        {
            std::lock_guard<std::mutex> lock(map_mutex);
            for (auto it = uslast_active.begin(); it != uslast_active.end();) {
                if(it->first <= 2) {
                  it++;
                  continue;
                }
                time_t last = it->second;

                if (now - last > HEARTBEAT_TIMEOUT) {
                    if(socket_to_uid.count(it->first))
                      std::cout << "用户[fd:" << it->first << "]uid:" << socket_to_uid[it->first] << " 心跳超时，断开连接\n";
                    else 
                      std::cout << "用户[fd:" << it->first << "]" << " 心跳超时，断开连接\n";
                    close(it->first);
                    it = uslast_active.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // 休眠一段时间，再检测
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

server::server()
    : listenfd_(socket(AF_INET, SOCK_STREAM, 0)), socket_(nullptr) {
    }

server::~server() {
  if (listenfd_ != -1) {
    close(listenfd_);
  }
}

bool server::setListen(unsigned short port) {
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr_len = sizeof addr;

  int ret = bind(listenfd_,(struct sockaddr*)&addr, sizeof addr);
  if(ret == -1) return false;
  
  ret = listen(listenfd_, 128);
  if(ret == -1) return false;
  return true;
}

bool server::acceptConn() {
  int cfd = accept(listenfd_,NULL, NULL);
  if(cfd == -1)
    return false;
  socket_.reset(new Socket (cfd));
  return true;
}

// 文件传输服务监听函数
void fileTransferThread() {
    MainReactor file_rct(5513, 4, true);  // 文件传输监听端口 5513，线程池 4 线程
}



int main(int argc, char* argv[]){
  Logger::init(argv[0]);  // 第一步调用日志初始化

  // 命令行参数解析
  if (argc >= 2) {
      server_ip_ = argv[1];
  }
  if (argc >= 3) {
      server_port_ = std::atoi(argv[2]);
  }
  
    redisContext* redis = connectRedis();
    if (!redis) {
      LOG_ERROR("Redis 连接失败，服务器无法启动");
      return 1;
    }

    auto ensureCounter = [&](const char* key) {
      redisReply* reply = (redisReply*)redisCommand(redis, "EXISTS %s", key);
      if (!reply) return false;
      bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer != 0);
      freeReplyObject(reply);
      if (exists) return true;

      reply = (redisReply*)redisCommand(redis, "SET %s 1000", key);
      if (!reply) return false;
      freeReplyObject(reply);
      return true;
    };

    if (!ensureCounter("newuid") || !ensureCounter("newgid") || !ensureCounter("newfid")) {
      LOG_ERROR("Redis 计数器初始化失败");
      redisFree(redis);
      return 1;
    }

    redisFree(redis);
    redis = nullptr;

  // 启动心跳检测线程
  std::thread heart_thread(heartbeatMonitorThread);
  heart_thread.detach();  // 后台运行，不阻塞主线程
  
  // 启动文件传输监听线程
  std::thread file_thread(fileTransferThread);
  file_thread.detach();  // 后台运行，不阻塞主线程

  MainReactor rct(server_port_, 16);
  while(1){
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return 0;
}

/*server sv;
  sv.setListen(4413);
  sv.acceptConn();
  Socket sk(*sv.getSocket());
  std::string str;
  while(1){
    str.clear();
    sk.recvMsg(str);
    for(int i = 0; i < str.size() ; i++) printf("%c",str[i]);
    puts("");
  }*/