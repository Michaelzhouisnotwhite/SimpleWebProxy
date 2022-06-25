//
// Created by michael on 2022/6/25.
//
#include "SocketUtils.h"
#include "proxy_server.h"
#include <stdio.h>
#include <string.h>

int main(int args, char *argv[]) {
    //    printf("Hello Main\n");
    if (args == 2) {
        if (0 == strcmp(argv[1], "1")) {
            int stats_code = server_func();
            printf("status: %d\n", stats_code);
        } else if (0 == strcmp(argv[1], "0")) {
            int argc = 2;
            const char *addr = "127.0.0.1";
            const char *p[2] = {"xxx", addr};
            int res = test_client(argc, p);
            printf("client res: %d\n", res);
        }
    }
    return 0;
}
