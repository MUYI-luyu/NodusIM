#pragma once

#include "../menu.h"
#include "../client.h"
#include "./allfucs/reportfuc.h"

#include "../../message.h"
#include "../../group.h"
#include "../../user.h"
#include "../../report.h"

extern user us;
extern report rpt;

class userfuc{
private:
    user u;
    void* clientp;
    std::thread recvThread;

public:
    userfuc(std::string js):u(user::fromJson(js)){
    };

    int mainfuc(void*p);

    int friendfuc(void);
    int groupfuc(void);
    int filefuc(void);
    int setupfuc(void);
    int reportfuc(void);

};