#include "./userfuc.h"

user us;

int userfuc::mainfuc(void * p){
    clientp = p;
    //Socket* sock = client->getSocket();
    Getrpt(clientp);
    char input = 0;
    system("clear");
    fflush(stdout); // 手动刷新标准输出缓冲区
    usermainmenu(u, clientp);
    std::string msg;
    bool flag = false;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            us = u = user::fromJson(msg);
            flag = true;
        }
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            usermainmenu(u, clientp);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{
            friendfuc();
            flag = true;
            break;
        }
        case '2':{
            groupfuc();
            flag = true;
            break;
        }
        case '3':{
            filefuc();
            flag = true;
            break;
        }
        case '4':{
            reportfuc();
            flag = true;
            break;
        }
        case '5':{
            if(setupfuc() == -1) return -2;
            flag = true;
        }
        default:continue;
        }
    }
    return 0;
}

int userfuc::friendfuc(void){
    char input = 0;
    friendfucs f(u, clientp);
    std::string msg;
    fflush(stdout); // 手动刷新标准输出缓冲区
    friendmenu(u, clientp);
    bool flag = false;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            us = u = user::fromJson(msg);
            flag = true;
        }
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            friendmenu(u, clientp);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{
            f.addfriend();
            flag = true;
            break;
        }
        case '2':{
            f.delfriend();
            flag = true;
            break;
        }
        case '3':{
            f.listfriend();
            flag = true;
            break;
        }
        case '4':{
            f.chatfriend();
            flag = true;
            break;
        }
        case '5':{
            f.shieldfriend(1);
            flag = true;
            break;
        }
        case '6':{
            f.shieldfriend(2);
            flag = true;
            break;
        }
        case '7':{
            f.searchfriend();
            flag = true;
            break;
        }
        case 27:{
            return 0;
        }
        default:continue;
        }
    }
    return 0;
}

int userfuc::groupfuc(void){
    char input = 0;
    std::string msg;
    system("clear");
    fflush(stdout); // 手动刷新标准输出缓冲区
    groupmenu(u, clientp);
    groupfucs grf(u, clientp);
    bool flag = false;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            us = u = user::fromJson(msg);
            flag = true;
        }
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            fflush(stdout); // 手动刷新标准输出缓冲区
            groupmenu(u, clientp);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{
            grf.creategroup();
            flag = true;
            break;
        }
        case '2':{
            grf.addgroup();
            flag = true;
            break;
        }
        case '3':{
            grf.list_group();
            flag = true;
            break;
        }
        case '4':{
            grf.groupchat();
            flag = true;
            break;
        }
        case '5':{
            grf.quitgroup();
            flag = true;
            break;
        }
        case '6':{
            grf.viewmember();
            flag = true;
            break;
        }
        case '7':{
            grf.manage();
            flag = true;
            break;
        }
        case 27:{
            return 0;
        }
        default:continue;
        }
    }
    return 0;
}

int userfuc::filefuc(void){
    char input = 0;
    std::string msg;
    system("clear");
    filefucs ffc(u, clientp);
    if(!file_sending && ffc.conntect_filepth() == false){
        dataclient.reinitialize();
        if(!ffc.conntect_filepth()){
            printf("\033[0;31m连接服务器文件传输线程失败，请稍后再试。\033[0m\n");
            printf("\033[0;31m请按任意键继续...\033[0m");
            charget();
            return 0;
        }
    }
    filemenu(u, clientp);
    bool flag = false;
    fflush(stdout); // 手动刷新标准输出缓冲区
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            us = u = user::fromJson(msg);
            flag = true;
        }
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            fflush(stdout); // 手动刷新标准输出缓冲区
            filemenu(u, clientp);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{//给某个好友发文件
            ffc.listfriend();
            flag = true;
            break;
        }
        case '2':{
            ffc.listgroup();
            flag = true;
            break;
        }
        case '3':{
            ffc.downloadfile();
            flag = true;
            break;
        }
        case '4':{
            ffc.downloadgfile();
            flag = true;
            break;
        }
        case 27:{
            if(!file_sending){
                dataclient.~Client();
                dataclient.reinitialize();
            }
            return 0;
        }
        default:continue;
        }
    }
    return 0;
}

int userfuc::setupfuc(void){
    setupfucs stf(u, clientp);
    Client * cp = (Client*)clientp;
    Socket* sock = cp->getSocket();
    char input = 0;
    std::string msg;
    system("clear");
    setupmenu(u, clientp);
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            us = u = user::fromJson(msg);
            flag = true;
        }
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            fflush(stdout); // 手动刷新标准输出缓冲区
            setupmenu(u, clientp);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{
            stf.modpwd();
            flag = true;
            break;
        }
        case '2':{
            if(stf.logoff() == -1)
                return -1;
            flag = true;
            break;
        }
        case '3':{
            stf.ViewAccountInformation();
            flag = true;
            break;
        }
        case '4':{
            system("clear");
            printf("\033[0;31m确定要退出登录？（Y/N）\033[0m\n");
            fflush(stdout);
            char input;
            bool ext = false;
            while(1){
                input = charget();
                if(input == 27) break;
                if(input != 'Y' && input != 'N' && input != 'y' && input != 'n') continue;
                if(input == 'Y' || input == 'y') ext = true;
                break;
            }
            if(ext){
                sock->sendMsg("unlg:"+u.name);
                return -1;
            } else flag = true;
            break;
        }
        case 27:{
            return 0;
        }
        default:continue;
        }
    }
    return 0;
}

int userfuc::reportfuc(void){
    char input = 0;
    std::string msg;
    system("clear");
    fflush(stdout); // 手动刷新标准输出缓冲区
    reportfucs rpf(u, clientp);
    bool ret = Getrpt(clientp);
    reportmenu( u, rpt, ret);
    bool flag = false;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            us = u = user::fromJson(msg);
            flag = true;
        }
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            reportmenu(u, rpt, true);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{
            rpf.friendreport();
            flag = true;
            break;
        }
        case '2':{
            rpf.ctfrdreport();
            flag = true;
            break;
        }
        case '3':{
            rpf.ctfgrpreport();
            flag = true;
            break;
        }
        case '4':{
            rpf.groupreport();
            flag = true;
            break;
        }
        case '5':{
            rpf.notice();
            flag = true;
            break;
        }
        case 27:{
            return 0;
        }
        default:continue;
        }
    }
    return 0;
}
