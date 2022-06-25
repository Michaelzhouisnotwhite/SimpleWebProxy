//
// Created by michael on 2022/6/25.
//
#include <stdio.h>
#include "proxy_server.h"

int main(int args, char *argv[]) {
//    printf("Hello Main\n");
    int stats_code = server_func();
    printf("status: %d\n", stats_code);
    return 0;
}
