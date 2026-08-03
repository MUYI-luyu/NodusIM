#include "./setupfuc.h"

void setupfucs::ViewAccountInformation(){
    system("clear");
    printf("\033[0;32m以下为您的基本信息:\n\033[0m");
    printf("\033[0;32m用户名:\033[0m%s\n",us.name.c_str());
    printf("\033[0;32m电子邮箱:\033[0m%s\n",us.email.c_str());
    printf("\033[0;32muid:\033[0m%s\n",us.uid.c_str());
    printf("\033[0;32m请按任意键继续...\033[0m");
    charget();
}

void setupfucs::modpwd(int fg){
    Client* c = (Client*)clientp;
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


int setupfucs::logoff(){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    printf("\033[0;31m确定要注销该账号？（Y/N）\033[0m\n");
    fflush(stdout);
    char input;
    bool off = false;
    while(1){
        input = charget();
        if(input == 27) break;
        if(input != 'Y' && input != 'N' && input != 'y' && input != 'n') continue;
        if(input == 'Y' || input == 'y') off = true;
        break;
    }
    if(!off) return 0;
    off = false;   
    printf("\033[0;31m真的确定要注销该账号？（T/F）\033[0m\n");
    while(1){
        input = charget();
        if(input == 27) break;
        if(input != 'T' && input != 't' && input != 'F' && input != 'f') continue;
        if(input == 'T' || input == 't') off = true;
        break;
    }
    if(!off) return 0;
    //发送验证码
    printf("\033[0;32m正在为您发送验证码...\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    EmailSender emsend;
    if(!emsend.send(us.email)){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m\n");
        charget();
        return 0;
    }
    //输入验证码
    char yan[100];
    printf("\033[0;32m请输入验证码\n\033[0m>");
    do{
        chu(yan);
        int ret = enter(yan, 0);
        if(ret == -1) return 0;
        if(strcmp(yan, emsend.code) != 0){
            printf("\033[0;31m验证码错误，请检查并重新输入。\n\033[0m>");
            continue;
        }
        break;
    } while(1);
    printf("\033[0;32m正在为您注销账号...\n\033[0m");
    sock->sendMsg("dsty:" + us.uid);
    std::string rev = EchoMsgQueue.wait_and_pop();
    printf("\033[0;32m该账户已注销。\n\033[0m");
    printf("\033[0;32m请按任意键继续...\033[0m\n");
    charget();
    return -1;
}
