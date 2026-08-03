#pragma once
#include "../MessageQueue.h"
#include "../../register/SendEmail.h"
#include "../../client.h"
#include "./reportfuc.h"

#include "../../../message.h"
#include "../../../group.h"
#include "../../../user.h"
#include "../../../report.h"

extern user us;
extern report rpt;

class setupfucs{
private:
    void* clientp;
    char name[256];
    char pwd[256];
    char email[256];
public:

    setupfucs(user& arg1, void*p):clientp(p){
    };
    //修改密码
    void modpwd(int fg = 2);
    //查看用户基本信息
    void ViewAccountInformation();
    //注销账号
    int logoff();
};