#pragma once
#include<iostream>
#include <fcntl.h>
#include "../client.h"
#include "../../user.h"
#include "../login/MessageQueue.h"
// #include "SendEmail.h"
#include <unistd.h>
#include <cstring>
#include "readdata.h"

class Register{
public:
    void rgst(void * p);

private:

    char name[256];
    char pwd[256];
    // char email[256];
    
    std::string pmitchar = "~!@#$%^&*()-=[];',./_+";
};
