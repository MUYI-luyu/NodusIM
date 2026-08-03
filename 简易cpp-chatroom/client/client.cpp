#include "client.h"
#include "register/register.h"
#include "login/login.h"
#include <atomic>
//定义数据通信伪客户端

extern std::string server_ip;
extern int server_port;

Client:: Client()
    : fd_(socket(AF_INET, SOCK_STREAM, 0)),
      socket_(std::make_unique<Socket>(fd_)){}

Client::~ Client() {
  close(fd_);
}

Client client;

bool Client::connectToHost(const char* ip, unsigned short port) {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  int ret = inet_pton(AF_INET,ip,&addr.sin_addr.s_addr);
  if(ret <= 0) return false;
  
  ret = connect(fd_,(struct sockaddr *)&addr,sizeof addr);
  if(ret == -1) return false;
  return true;
}

void Client::ctlthread(){
  char input = 0;
  while(1){
    fflush(stdout); // 手动刷新标准输出缓冲区
    system("clear");
    startmenu();
    input = charget();
    switch(input){
      case '1':
        login l;
        l.lgin(this);
        break;
      case '2':{
        Register r;
        r.rgst(this);
        break;
      }
      case '3':
        login f;
        f.findpwd(this, 1);
        break;
      case '4':{
        system("clear");
        return;
      }
      default:continue;
    }
  }
}



void heartbeatThread() {
    const int interval = 30; // 心跳间隔秒数
    std::string heartbeat_msg = "PING";
    Socket* sock = client.getSocket();
    while (1) {
        // 发送心跳包
        sock->sendMsg(heartbeat_msg);
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}


int main(int argc, char* argv[]){
  // 命令行参数解析
  if (argc >= 2) {
      server_ip = argv[1];
  }
  if (argc >= 3) {
      server_port = std::atoi(argv[2]);
  }
  printf("目标服务器: %s:%d\n", server_ip.c_str(), server_port);

  sigset_t set;
  sigaddset(&set,SIGINT);
  sigaddset(&set,SIGQUIT);
  sigprocmask(SIG_BLOCK,&set,NULL);
  int tmp = 20;
  while(client.connectToHost(server_ip.c_str(), server_port)==false){
    printf("连接服务器%d端口失败,error:%s\n",server_port,strerror(errno));
    printf("正在重新连接...剩余重连次数：%d次\n", tmp);
    sleep(1);
    tmp--;
    if(tmp == 0) exit(1);
  }
  recv_running = true;
  std::thread recvThread = std::thread(recv_thread, client.getSocket());
  std::thread hb_t(heartbeatThread);     // 心跳线程

  client.ctlthread();



  //设置标志位为 false，让 recv_thread 自行退出
  recv_running = false;
  //关闭 socket，让阻塞的 recvMsg 返回
  (client.getSocket())->mshutdown();  // 确保这个函数内部会关闭连接
  //等待线程退出
  if (recvThread.joinable()) {
      recvThread.join();
  }
  if (hb_t.joinable()) {
      hb_t.join();
  }
  fflush(stdout);
  system("clear");
  fflush(stdout);
  return 0;
}

//pthread_t tid = 0;
//pthread_create(&tid, NULL, read_clit,(void *)client.getSocket());