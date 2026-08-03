#pragma once
#include <iostream>
#include "login/allfucs.h"

// 函数声明，不要在头文件中实现函数
void startmenu(void);
void logmenu();
void usermainmenu(user &u, void *p);
void friendmenu(user &u, void *p);
void groupmenu(user& u, void *p);
void filemenu(user& u, void *p);
void reportmenu(user& u, report rpt, bool ret);
void setupmenu(user& u, void *p);
