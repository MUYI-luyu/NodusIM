#pragma once
#include<iostream>
#include <fcntl.h>
#include "../../client.h"
#include "../../../user.h"
#include <unistd.h>
#include <cstring>
#include "../MessageQueue.h"
#include <chrono>

#include "../../../message.h"
#include "../../../group.h"
#include "../../../user.h"
#include "../../../report.h"

extern user us;
extern report rpt;

// 全局变量
extern std::chrono::steady_clock::time_point start_time;
extern void startTime();
extern int throughtime();
extern bool Getrpt(void *clientp);

class reportfucs{
private:
    void* clientp;
    int page = 0;
    int enterfuc = 1;
    int ctpage = 0;
    int msgcnt = 0;
    messages save;
    group viewgp;
public:

    reportfucs(user& arg1, void*p):clientp(p){
    };
    reportfucs(){};
    void reportinit(){
    };
    //获取当前全部通知
    //（外部函数）

    //处理好友申请页面
    void friendreport();
    void groupreport();
    //好友申请页面的菜单
    void menu(char c , int fg);
    //处理好友申请的函数
    void handleapply(char );
    //处理加群申请
    void handlegroupapply(char );
    //查看事务通知的页面
    void notice();
    //分析公告
    void Analysisnotice(std::string&str, int i);
    //处理公告
    void handlenotice(char );
    //查看聊天请求的页面
    void ctfrdreport();
    //查看群聊天请求的页面
    void ctfgrpreport();
    //处理聊天
    void handlechat(char, report& rpt);
    //聊天界面
    void chatmenu(char c, user& ud2);

    void gchatmenu(char c, group& gp);
    void ghandlechat(char c, int fg);

    //打印新通知
    static void newreport(user&us, void* p){

        if(rpt.friendapply.size())
            printf("           \033[0;34m[new] 有%ld条未处理的好友申请\033[0m\n", rpt.friendapply.size());
        if(rpt.total_friend_msg)
            printf("           \033[0;34m[new] 有%d个未读的好友消息\033[0m\n", rpt.total_friend_msg);
        if(rpt.total_group_msg)
            printf("           \033[0;34m[new] 有%d个未读的群聊消息\033[0m\n", rpt.total_group_msg);
        if(rpt.groupapply.size())
            printf("           \033[0;34m[new] 有%ld条未处理的加群申请\033[0m\n", rpt.groupapply.size());
        int cnt = 0;
        for(int i = 0; i < rpt.notice.size(); i++)
            if(rpt.notice[i][5] == 'n') cnt++;
        if(cnt)
        printf("           \033[0;34m[new] 有%d条未读的通知\033[0m\n", cnt);
    }
};
