#pragma once
#include<iostream>
#include<string>
#include<vector>
#include "../user.h"
#include "../group.h"
#include "../Redis.h"
#include "report.h"
#include "../message.h"
#include "sendrecv.h"
#include "../Logger.h"

class userdata{
private:
redisContext * redis;
std::string uid;
std::string name;
std::string email;
std::string pwd;
std::string stat;
std::vector<std::string> friendlist;//存储好友的uid
std::vector<std::string> grouplist; //存储群聊的gid
std::vector<std::string> shieldlist; //存储黑名单的uid

public:

    userdata():redis(connectRedis()){
    }
    ~userdata() {
        if (redis) {
            redisFree(redis);
            redis = nullptr;
        }
    }
    //根据json字符串注册用户,返回uid或fail
    std::string newuser(std::string);
    std::string newgroup(std::string&);

    //根据uid和json字符串存储report
    bool svreport(std::string uid, std::string js);
    //获取一个未使用的uid
    std::string newuid(void);
    std::string newgid(void);
    std::string newfid();

    //根据用户名返回用户id，若用户不存在则返回“norepeat”
    std::string Getuid(const char *);
    std::string Getgid(const char * buf);
    std::string Getfid(std::string&, std::string&, bool isgroupfile);

    //根据用户uid返回用户结构体
    user GetUesr(std::string);
    //返回group的json字符串，若不存在，返回norepeat
    std::string GetGroup(std::string);

    //根据email返回用户id，若用户不存在则返回“norepeat”
    std::string EmailGetuid(const char *);
    //判断电子邮箱是否已存在，返回true已存在，返回false不存在
    bool RepeatEmail(const char *);
    

    //根据用户uid删除用户结构体
    bool DELUesr(std::string);
    //仅存入user:uid -> json的键值对
    bool setutoj(std::string, std::string);
    //仅存入group:gid -> json的键值对
    bool setgtoj(std::string, std::string);

    //将name1放在uid2的好友申请表内（uid1加uid2）
    std::string AddFrd(std::string name1, std::string uid2);
    //判断传入的uid是否存在，存在返回true，不存在返回false
    bool jguid(std::string uid);
    //根据uid获取用户的report(json字符串)，成功返回json，失败返回none
    std::string u_report(std::string uid);
    //根据uid1、uid2、消息条数、拉取顺序获取消息列表，fg为0拉最新消息，为1拉最早消息
    std::vector<std::string> lrange(std::string uid1, std::string uid2, int cnt, int fg);
    std::vector<std::string> glrange(std::string gid, int cnt, int fg);
    //根据uid1、uid2获取消息总量
    int llen(std::string uid1, std::string uid2);
    int gllen(std::string gid);
    //根据聊天的json字符串存聊天记录
    void savechat(std::string js);
    void savechat2(std::string js);
    void savegchat(std::string js);

    //根据gid注销群聊的后端数据
    void disbandgroup(std::string gid, std::string gname);
    //处理uid注销用户的后端数据
    void destroy_user(std::string uid, std::string uname, std::string email);
};