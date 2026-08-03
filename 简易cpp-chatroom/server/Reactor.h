#pragma once
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <ctype.h>
#include <time.h>
#include <queue>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <mutex>
#include <thread>

#include "../ThreadPool.h"
#include "handle.h"
#include "../Logger.h"

#define MAX_EVENTS 1024     //监听上限数
#define MAX_PORT 65535      //端口上限

extern std::string server_ip_;
extern uint16_t server_port_;

extern std::unordered_map<int, std::unique_ptr<std::mutex>> fd_write_mutexes;
extern std::unordered_map<int, std::unique_ptr<std::mutex>> fd_read_mutexes;
extern std::unordered_map<int, time_t> uslast_active;
extern std::mutex map_mutex;

class WorkerReactor{
private:

typedef struct event{
    int fd;         //待监听的文件描述符
    int events;     //对应的监听事件
    void*arg;       //泛型参数
    void (WorkerReactor::*call_back)(int fd, int events, void * arg); //回调函数
    int status;     //是否在红黑树上，1->在，0->不在
    //读入的信息及长度
    std::string buf;
    std::mutex buf_mutex; //缓冲区锁

    int len;
    //用于监听的文件描述符
    int lisfd;
    int lisdatafd;
    //用于给客户端传输数据的文件描述符
    int datafd;
    //用于通信的地址结构
    struct sockaddr_in skaddr;
    socklen_t skaddr_len;
    //用于控制顺序的锁和条件变量及控制原子
    pthread_mutex_t pthlock;
    pthread_cond_t pthcond;

    //用于给数据传输线程通知有新任务的锁
    pthread_mutex_t datalock;
    pthread_cond_t datacond;

    pthread_mutex_t splock;
    pthread_cond_t spcond;


    //true--线程已准备好， false--线程未准备好
    bool pthready;
    bool dataready;
    bool spready;
    bool lockinit;
    long last_active;   //记录最后加入红黑树的时间值

    bool poolrs;//true---已经加入线程池
    bool poolda;//true---已经开启数据传输线程
}event;

    struct EventContext {
        event* ev;
        WorkerReactor* obj;
    };
    
    int epfd;   //红黑树的句柄
    event r_events[MAX_EVENTS + 1];
    pthread_pool &pthpool;
    pthread_mutex_t event_mutex; // 事件锁，用于修改红黑树的公共区域
    std::queue<EventContext*> evq;
    //标记是否为用于数据传输的反应堆
    bool datareactor = false;

    //删除树上节点
    void eventdel(event * ev);
    
    //监听回调
    void acceptconn(int lfd,int tmp, void * arg);

    //获取一个端口与数据传输线程
    unsigned short getport(event * ev);
    static void data_pth(WorkerReactor::event* ev, unsigned short port, WorkerReactor* th);
    
    //获取需发送的字符串
    void getsendstr(event* ev,unsigned short dataport, std::string &str);

    //处理回调
    void HandleCallback(int fd,int tmp, void * arg);
    
    //读回调
    void RecvCallback(int fd, int events, void*arg);


    //初始化事件
    void eventset(event * ev, int fd, void (WorkerReactor::* call_back)(int ,int , void *), void * arg);

    //添加文件描述符到红黑树
    void eventadd(int events, event * ev);

    //初始化监听socket
    void InitListenSocket(unsigned short port);


    //线程池执行的需要执行任务
    static void event_callback_wrapper(struct EventContext * arg) {
        struct EventContext* ctx = arg;
        (ctx->obj->*(ctx->ev->call_back))(ctx->ev->fd, ctx->ev->events, ctx->ev->arg);
    }

public:
    WorkerReactor(unsigned short port, pthread_pool& sharedPool, bool datarct = false);

    ~WorkerReactor();
    void addClient(int cfd, sockaddr_in client_addr); 
    void readctorinit(unsigned short port);
};


class MainReactor{
private:
    static const int WORKER_COUNT = 4;
    std::vector<std::unique_ptr<WorkerReactor>> workers;
    int next_worker = 0;  // 轮询分配新连接

typedef struct event{
    int fd;         //待监听的文件描述符
    int events;     //对应的监听事件
    void*arg;       //泛型参数
    void (MainReactor::*call_back)(int fd, int events, void * arg); //回调函数
    int status;     //是否在红黑树上，1->在，0->不在
    //读入的信息及长度
    std::string buf;
    std::mutex buf_mutex; //缓冲区锁

    int len;
    //用于监听的文件描述符
    int lisfd;
    int lisdatafd;
    //用于给客户端传输数据的文件描述符
    int datafd;
    //用于通信的地址结构
    struct sockaddr_in skaddr;
    socklen_t skaddr_len;
    //用于控制顺序的锁和条件变量及控制原子
    pthread_mutex_t pthlock;
    pthread_cond_t pthcond;

    //用于给数据传输线程通知有新任务的锁
    pthread_mutex_t datalock;
    pthread_cond_t datacond;

    pthread_mutex_t splock;
    pthread_cond_t spcond;


    //true--线程已准备好， false--线程未准备好
    bool pthready;
    bool dataready;
    bool spready;
    bool lockinit;
    long last_active;   //记录最后加入红黑树的时间值

    bool poolrs;//true---已经加入线程池
    bool poolda;//true---已经开启数据传输线程
}event;

    struct EventContext {
        event* ev;
        MainReactor* obj;
    };
    
    int epfd;   //红黑树的句柄
    event r_events[MAX_EVENTS + 1];
    pthread_pool pthpool;
    pthread_mutex_t event_mutex; // 事件锁，用于修改红黑树的公共区域
    std::queue<EventContext*> evq;
    //标记是否为用于数据传输的反应堆
    bool datareactor = false;

    //删除树上节点
    void eventdel(event * ev);
    
    //监听回调
    void fileacceptconn(int lfd,int tmp, void * arg);
    void acceptconn(int lfd,int tmp, void * arg);

    //获取一个端口与数据传输线程
    unsigned short getport(event * ev);
    static void data_pth(MainReactor::event* ev, unsigned short port, MainReactor* th);
    
    //获取需发送的字符串
    void getsendstr(event* ev,unsigned short dataport, std::string &str);

    //处理回调
    void HandleCallback(int fd,int tmp, void * arg);
    
    //读回调
    void RecvCallback(int fd, int events, void*arg);

    //初始化事件
    void eventset(event * ev, int fd, void (MainReactor::* call_back)(int ,int , void *), void * arg);

    //添加文件描述符到红黑树
    void eventadd(int events, event * ev);

    //初始化监听socket
    void InitListenSocket(unsigned short port);

    void readctorinit(unsigned short port);

    //线程池执行的需要执行任务
    static void event_callback_wrapper(struct EventContext * arg) {
        struct EventContext* ctx = arg;
        (ctx->obj->*(ctx->ev->call_back))(ctx->ev->fd, ctx->ev->events, ctx->ev->arg);
    }

public:
    MainReactor(unsigned short port, unsigned int pthread_cnt, bool datarct = false);

    ~MainReactor();
};