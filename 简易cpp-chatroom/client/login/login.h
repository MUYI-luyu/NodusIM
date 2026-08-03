#pragma once
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "userfuc.h"
#include "../register/readdata.h"

#include "../../message.h"
#include "../../group.h"
#include "../../user.h"
#include "../../report.h"

class login{
private:
    char name[256];
    char pwd[256];
    char email[256];
    void* clientp;
    int pwdlog();
    int emaillog();
public:

    void lgin(void*p);
    void findpwd(void*p, int fg);
};