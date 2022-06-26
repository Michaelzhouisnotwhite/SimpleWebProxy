//
// Created by michael on 2022/6/25.
//

#include "SocketUtils.h"
#include "SocketException.h"
#include "try_catch.h"

void hello_common() {
    printf("HELLO COMMON\n");
}

int SocketInit() {
    WSADATA wsaData;
    int iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iRes != 0) {
        printf("WSAStartup failed: %d\n", iRes);
        exit(-1);
    }
    return SOCKET_RUNTIME_SUCCESS;
}

SOCKET SocketTcpCreate() {
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int SocketListening(const char *port, SOCKET *ResSocket) {
    SOCKET listenSocket = SocketTcpCreate();

    struct addrinfo hints, *addr_result;
    ZeroMemory(&hints, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    int nRes;

    nRes = getaddrinfo(NULL, port, &hints, &addr_result);

    if (nRes != 0) {
        printf("getaddrinfo failed: %d\n", nRes);
        freeaddrinfo(addr_result);
        WSACleanup();
        return SOCKET_RUNTIME_ERROR;
    }

    nRes = bind(listenSocket, addr_result->ai_addr, (int) addr_result->ai_addrlen);
    if (nRes == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return SOCKET_BIND_ERROR;
    }
    // Listen
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return SOCKET_LISTENING_ERROR;
    }

    *ResSocket = listenSocket;
    return SOCKET_RUNTIME_SUCCESS;
}



void ClearBuf(char **buffer) {
    if (buffer != NULL) {
        free(*buffer);
    }
}
