//
// Created by michael on 2022/6/25.
//

#include "SocketUtils.h"

void hello_common() {
    printf("HELLO COMMON\n");
}

int test_server() {
    WSADATA wsaData;

    const char* default_port = "27015";

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

}