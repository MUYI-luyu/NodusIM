#pragma once
#include "../MessageQueue.h"
#include "reportfuc.h"
#include "../../../message.h"
#include <algorithm>

#include "../../../message.h"
#include "../../../group.h"
#include "../../../user.h"
#include "../../../report.h"

extern user us;
extern report rpt;

class friendfucs{
private:
    void* clientp;
    //用于菜单的页码
    int page = 0;
    //用于聊天的页码
    int ctpage = 0;
    int msgcnt = 0;
    int enterfuc = 1;
    messages save;

    //用于搜索好友的列表
    friendnamelist fnl;
public:

    friendfucs(user &arg1, void*p):clientp(p){
    };
    //仅展示好友列表
    void list(char);
    //添加好友
    void addfriend();
    //选定删除好友
    void delfriend();
    //处理删除功能
    void handledel(char, int fg = 1);
    //查看好友列表
    void listfriend();
    //选中的菜单，选中好友后触发的对应流程
    void selectmenu(char, int);
    //选中功能
    void select(char, int);
    //选定私聊页面
    void chatfriend();
    //输出聊天页面
    void chatmenu(char c, user& u2);
    //处理私聊功能
    void handlechat(char, int fd = 1);
    //屏蔽/解除屏蔽好友，1为屏蔽，2为解除屏蔽
    void shieldfriend(int);
    //屏蔽功能
    void shield(char, int fg = 1);
    //解除屏蔽功能
    void shield_exit(char, int fg = 1);
    //屏蔽的好友的菜单
    void shield_menu(char);
    //搜索功能
    void searchfriend();
    void searchlist(char);
};
