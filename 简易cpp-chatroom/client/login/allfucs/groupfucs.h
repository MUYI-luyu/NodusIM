#pragma once
#include "../MessageQueue.h"
#include "../../client.h"
#include "./reportfuc.h"

#include "../../../message.h"
#include "../../../group.h"
#include "../../../user.h"
#include "../../../report.h"

extern user us;
extern report rpt;

class groupfucs{
private:
    void* clientp;
    int page = 0;
    int viewpage = 0;
    int enterfuc = 1;
    char name[512];
    friendnamelist fnl;
    group viewgp;

    messages save;
    int ctpage = 0;
    int msgcnt;

    friendnamelist grouplist;
    std::string identity;
public:

    groupfucs(user& arg1, void*p):clientp(p){
    };
    //创建群聊
    void creategroup();
    //打印群聊列表
    void list(char);
    //加群
    void addgroup();
    //退出
    void quitgroup();
    void handlequit(char, int fg = 1);
    //查看群成员
    void viewmember();
    void view(char , int fg = 1, int hdl = 0);
    void viewlist(char c);

    //进行群聊
    void groupchat();
    int handlechat(char, int fg = 1);
    void chatmenu(char c, group& gp);

    //查看群聊列表
    void list_group();
    //选中的菜单，选中群聊后触发的对应流程
    int selectmenu(char c, int fg);
    //选中功能
    void select(char, int);
    void manage();
    void managelist(char);

    //权限功能
    void disband(char , int);
    void addmanager(std::string gid, std::string uid2);
    void delmanager(std::string gid, std::string uid2);
    void kickmember(std::string gid, std::string uid2);

};