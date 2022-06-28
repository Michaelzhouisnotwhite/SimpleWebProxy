//
// Created by michael on 2022/6/25.
//
#include "SocketUtils.h"
#include "proxy_server.h"
#include <stdio.h>

int main(int args, char *argv[]) {
    setbuf(stdout, NULL);
    SocketInit();
    const char *port = "7001";
    ServerStart(port);
    return 0;
}
