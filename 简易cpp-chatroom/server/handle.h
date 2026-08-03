#pragma once
#include<iostream>
#include<cstring>
#include<string>
#include "userdata.h"
#include "../user.h"
#include "../client/register/hashpwd.h"
#include "../message.h"
#include "../Logger.h"
#include "server.h"
#include <mutex>
#include <dirent.h>
#include <sys/stat.h>

extern std::unordered_map<int , time_t> uslast_active;
extern std::mutex map_mutex;

class handler{
private:
    std::string str;
    int sockfd;
    userdata u;
    //判断一个uid是否存在
    void jrud();
    //处理是否重名的请求
    void jrnm();
    //处理邮箱是否注册过的请求
    void jrem();
    //处理用户名和邮箱是否是同一组的请求
    void jrne();
    //处理两个uid是否为好友的请求
    void jrfd();
    //处理找回密码的请求
    void fdpd();
    //处理向邮箱集合中增加元素的请求
    void emad();
    //处理注册的请求
    void torgst();
    //处理账号密码登录的请求
    void pwlg();
    //处理邮箱验证码登录成功的请求
    void emlg();
    //利用uid处理登录成功的后端数据,返回user的json字符串
    std::string lgok(std::string);
    //利用用户名处理用户下线的请求
    void unlg(std::string);
    //处理客户端突然断开后的rvlg指令，即让其下线
    void rvlg();
    //处理用户获取report(json字符串)的请求
    void gtrp();
    //处理用户获取 user (json字符串)的请求
    void gtus();
    //处理用户获取 group (json字符串)的请求
    void gtgp();
    //根据用户名获取uid的请求
    void gtud();
    //根据uid+path获取或创建唯一的fid
    void gtfi();
    //根据uid获取用户名的请求
    void gtnm();
    //根据gid获取用户名的请求
    void ggnm();

    //处理添加好友的请求
    void adfr();
    //处理加群的请求
    void adgp();
    //处理同意/拒绝好友的请求
    void adfok();
    //处理同意/拒绝加群申请的请求
    void adgok();
    //处理已读通知的请求
    void rdnt();
    //处理删除通知的请求
    void rmnt();
    //处理删除好友的请求
    void rmfd();
    //处理退出群聊的请求
    void rmgp(std::string&, int fg = 1);
    //拉取两页聊天记录
    void ctms();
    //拉取两页群聊记录
    void gctm();
    //处理用户发送消息的请求
    void sdms();
    //处理用户发送群消息的请求
    void sdgm();
    //处理用户读取聊天记录总条数的请求
    void rdpg();
    void rdgc();
    //处理用户需要保存json字符串的需求
    void svrp();
    //处理用户需要提供聊天记录的请求
    void ndms();
    void ndgm();
    //处理用户屏蔽好友的请求
    void shfd();
    //处理解除屏蔽好友的请求
    void shex();
    //判断群聊名是否重复
    void jrgn();
    //创建群聊
    void ctgp();
    //处理用户获取好友名列表的请求
    void fdlt();
    //处理用户获取群组成员列表的请求
    void gplt();
    //添加管理员
    void admn();
    //删除管理员
    void dlmn();
    //踢出群成员
    void kcmb();
    //解散群聊
    void disg(std::string&, int fg = 1);
    //根据uid获取用户为管理员的全部群
    void mngl();
    //根据uid获取用户所有传来的文件
    void gtrl();
    //根据uid获取该用户所有可接收的群文件
    void gtgf();

    //处理文件结束的通知
    void fled();
    void gfed();
    //获取文件大小
    void flsz();
    void gfsz();
    //处理用户接收完文件的通知
    void rved();
    void rvge();
public:
    handler(std::string buf, int fd):str(buf),sockfd(fd){
    }
    
    //利用uid处理用户下线的请求
    void uulg(std::string);
    //处理用户传来的请求
    int handle();
    //处理下载文件的请求
    void sdfl();
    void sdgf();
    //处理注销账号的请求
    void dsty();
    
};

extern std::string GetFileName(const char arr[]);
extern void rvfl(std::string &str);
extern void rvgf(std::string &str);