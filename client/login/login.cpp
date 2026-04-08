#include "./login.h"

void login::lgin(void*p){
    clientp = p;
    //Socket* sock = client->getSocket();

    char input = 0;
    //输入用户名和密码
    while(1){
        system("clear");
        logmenu();
        input = charget();
        switch(input){
        case '1':{
            if(pwdlog() == -1) return;
            break;
        }
        case '2':{
            if(emaillog() == -1) return;
            break;
        }
        case 27:{
            system("clear");
            return;
        }
        default:continue;
        }
    }
}

int login::pwdlog(){
    Client* c = (Client*)clientp;
    Socket* sock = c->getSocket();
    system("clear");
    printf("\033[0;32m已选择用户名、密码登录(按ESC可返回上一级菜单)\033[0m\n");
    std::string red;
    char buf[520];
    while(1){
        chu(name);
        chu(pwd);
        printf("\033[0;32m请输入您的用户名:\n\033[0m>");
        if(enter(name, 0) == -1) return 0; 
        printf("\033[0;32m请输入您的密码:\n\033[0m>");
        if(enter(pwd, 1) == -1) return 0;
        printf("\033[0;32m登录中...\033[0m");
        fflush(stdout); // 手动刷新标准输出缓冲区
        //账号密码给服务器处理
        chu(buf);
        sprintf(buf, "pwlg:%s:%s", name, pwd);
        sock->sendMsg(buf);
        red = EchoMsgQueue.wait_and_pop();
        if(red != "pwdfalse") {
            userfuc uf(red);
            us = user::fromJson(red);
            if(uf.mainfuc(c) == -2) return -1;
        } else{
            printf("\033[0;31m\n用户名或密码错误，请重新输入。\033[0m\n");
            continue;
        }
    }
    return 0;
}

int login::emaillog(){
    Client* c = (Client*)clientp;
    Socket* sock = c->getSocket();
    EmailSender emsend;

    system("clear");
    printf("\033[0;32m已选择邮箱、验证码登录(按ESC可返回上一级菜单)\033[0m\n");
    std::string red;
    char buf[520];
    printf("\033[0;32m请输入您的邮箱:\n\033[0m>");
    while(1){
        chu(email);
        chu(pwd);
        int tmp = enter(email, 0);
        if(tmp == -1) return 0; 
        if(tmp <= 7){
            printf("\033[0;31m该电子邮箱无效，请检查并重新输入。\n\033[0m>");
            continue;
        }
        //判断邮箱是否已经注册
        //询问服务器
        sock->sendMsg("jrem:"+std::string(email));//judge_repeat_email
        std::string str;
        str = EchoMsgQueue.wait_and_pop();
        if(strcmp(str.c_str(), "repeat") != 0){
            printf("\033[0;31m该邮箱未被注册，请重新输入。\n\033[0m>");
            continue;
        }
        //发送验证码
        printf("\033[0;32m正在为您发送验证码...\033[0m");
        fflush(stdout); // 手动刷新标准输出缓冲区
        if(!emsend.send(email)){
            printf("\033[0;31m该电子邮箱无效，请检查并重新输入。\n\033[0m>");
            continue;
        }
        //输入验证码
        printf("\033[0;32m请输入验证码\n\033[0m>");
        do{
            chu(pwd);
            int ret = enter(pwd, 0);
            if(ret == -1) return 0;
            if(strcmp(pwd, emsend.code) != 0){
                printf("\033[0;31m验证码错误，请检查并重新输入。\n\033[0m>");
                continue;
            } else {
                str.clear();
                str = "emlg:";
                sock->sendMsg(str+std::string(email));
                str = EchoMsgQueue.wait_and_pop();
                if(str != "false"){
                    userfuc uf(str);
                    us = user::fromJson(str);
                    if(uf.mainfuc(c) == -2) return -1;
                } else {
                    printf("\033[0;31m数据异常，请重试。\n\033[0m>");
                    continue;
                }
            }
            break;
        } while(1);
    }
    return 0;
}

void login::findpwd(void*p, int fg){
    Client* c = (Client*)p;
    Socket* sock = c->getSocket();
    std::string pmitchar = "~!@#$%^&*()-=[];',./_+";
    system("clear");
    if(fg == 1)
    printf("\033[0;32m正在为您找回密码(按ESC可返回上一级菜单)\033[0m\n");
    else if(fg == 2)
    printf("\033[0;32m正在为您修改密码(按ESC可返回上一级菜单)\033[0m\n");

    // printf("\033[0;32m请输入用户名。\033[0m\n>");
    // //输入用户名
    // do{
    //     //int ret = read(STDIN_FILENO,name,sizeof name);
    //     chu(name);
    //     int ret = enter(name, 0);
    //     if(ret == -1) return;
    //     //判断是否有重名
    //     //询问服务器
    //     sock->sendMsg("jrnm:"+std::string(name));//judge_repeat_name
    //     //sock->sendMsg(name);
    //     std::string str;
    //     sock->recvMsg(str);
    //     if(strcmp(str.c_str(), "norepeat") == 0){
    //         printf("\033[0;31m当前用户不存在，请重新输入。\n\033[0m>");
    //         continue;
    //     }
    //     break;
    // }while(1);

    //输入电子邮箱
    EmailSender emsend;
    printf("\033[0;32m请输入您账号绑定的电子邮箱\n\033[0m>");
    do{
        chu(email);
        int ret = enter(email, 0);
        if(ret == -1) return;
        if(ret <= 7){
            printf("\033[0;31m该电子邮箱无效，请检查并重新输入。\n\033[0m>");
            continue;
        }
        //判断邮箱是否已经注册
        //询问服务器
        std::string str = "jrem:";
        for(int i = 0; i < strlen(email); i++) str.push_back(email[i]);
        sock->sendMsg(str);//judge_repeat_email
        str = EchoMsgQueue.wait_and_pop();
        if(strcmp(str.c_str(), "repeat") != 0){
            printf("\033[0;31m该邮箱未被注册，请重新输入。\n\033[0m>");
            continue;
        }
        // //判断用户名和邮箱是否绑定
        // str.clear();
        // //jrne:name:email
        // str = "jrne:";
        // for(int i = 0; i < strlen(name); i++) str.push_back(name[i]);
        // str += ':';
        // for(int i = 0; i < strlen(email); i++) str.push_back(email[i]);
        // sock->sendMsg(str);
        // sock->recvMsg(str);
        // if(strcmp(str.c_str(), "false") == 0){
        //     printf("\033[0;31m用户名和邮箱不匹配，请重新输入\n\033[0m>");
        //     continue;
        // }
        //发送验证码
        printf("\033[0;32m正在为您发送验证码...\033[0m");
        fflush(stdout); // 手动刷新标准输出缓冲区
        if(!emsend.send(email)){
            printf("\033[0;31m该电子邮箱无效，请检查并重新输入。\n\033[0m>");
            continue;
        }
        //输入验证码
        char yan[100];
        printf("\033[0;32m请输入验证码\n\033[0m>");
        do{
            chu(yan);
            int ret = enter(yan, 0);
            if(ret == -1) return;
            if(strcmp(yan, emsend.code) != 0){
                printf("\033[0;31m验证码错误，请检查并重新输入。\n\033[0m>");
                continue;
            }
            break;
        } while(1);

        //输入密码
        printf("\033[0;32m请输入您要设置的新密码\n\033[0m>");
        do{
            chu(pwd);
            int ret = enter(pwd, 1);
            if(ret == -1) return;
            else if(ret <= 6) {
                printf("\033[0;31m密码至少为7位，请重新输入。\n\033[0m>");
                continue;
            }
            int key = 0;
            for(int i = 0; i < ret; i++){
                if(!((pwd[i] >= 'a' && pwd[i] <= 'z')||(pwd[i] >= 'A' && pwd[i] <= 'Z')||(pwd[i] >= '0' && pwd[i] <= '9'))){
                    int j;
                    for(j = 0; j < pmitchar.size(); j++){
                        if(pmitchar[j] == pwd[i]) break;
                    }
                    if(j == pmitchar.size()){
                        key = 1;
                        printf("\033[0;31密码仅能含有数字、字母和大部分键盘可打出的字符，当前密码不合法，请重新输入。\n\033[0m>");
                        break;
                    }
                }
            }
            if(key) continue;
            break;
        } while(1);

        //再次输入密码
        printf("\033[0;32m请再次输入您的密码\n\033[0m>");
        char pwd2[256];
        do{
            chu(pwd2);
            int ret = enter(pwd2, 1);
            if(ret == -1) return;
            if(strcmp(pwd,pwd2) != 0){
                printf("\033[0;31m输入与第一次不符，请重新输入。\n\033[0m>");
                continue;
            }
            break;
        } while(1);

        //哈希密码,加密数据
        PasswordHasher hs;
        hs.hashPassword(pwd);

        //将数据打包发给服务器
        str.clear();
        //fdpd:emial:pwd
        str = "fdpd:";
        for(int i = 0;i<strlen(email);i++)str.push_back(email[i]);
        str += ':';
        for(int i = 0;i<strlen(pwd);i++)str.push_back(pwd[i]);
        sock->sendMsg(str);
        str = EchoMsgQueue.wait_and_pop();
        if(str == "right"){
            printf("\033[0;32m设置成功！\n请按任意键继续...\033[0m");
            char rub = charget();
            return;
        } else {
            printf("\033[0;31m数据异常，请重试。\n\033[0m>");
        }
    } while(1);
}
