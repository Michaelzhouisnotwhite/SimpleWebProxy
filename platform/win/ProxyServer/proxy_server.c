//
// Created by michael on 2022/6/25.
//

#include "proxy_server.h"
#include "SocketException.h"

int server_func() {
    printf("Hello from proxy server!\n");
    return test_server();
}

int ServerInit(SOCKET *ResSocket, const char *port) {
    SOCKET listenSocket = INVALID_SOCKET;

    int nRes;
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    listenSocket = SocketTcpCreate(hints);
    if (listenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return SOCKET_RUNTIME_ERROR;
    }
    struct addrinfo *addr_result;
    nRes = getaddrinfo(NULL, port, &hints, &addr_result);

    if (nRes != 0) {
        printf("getaddrinfo failed: %d\n", nRes);
        freeaddrinfo(addr_result);
        WSACleanup();
        return SOCKET_RUNTIME_ERROR;
    }

    nRes = bind(listenSocket, addr_result->ai_addr, (int) addr_result->ai_addrlen);
    freeaddrinfo(addr_result);
    if (nRes == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return SOCKET_RUNTIME_ERROR;
    }
    *ResSocket = listenSocket;
    return SOCKET_RUNTIME_SUCCESS;
}

int ServerHandleClient(SOCKET clientSocket, struct sockaddr addr, int addrlen) {
    return 0;
}

int ServerStart(SOCKET *listenSocket) {
    // Listen
    if (listen(*listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(*listenSocket);
        WSACleanup();
        return SOCKET_RUNTIME_ERROR;
    }
    int keep_connection = 1;

    const int DEFAULT_BUFLEN = 512;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    while (1) {

        struct sockaddr addr;
        int addrlen;
        SOCKET clientSocket = accept(*listenSocket, &addr, &addrlen);
        ServerHandleClient(clientSocket, addr, addrlen);

        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(*listenSocket);
            WSACleanup();
            return SOCKET_RUNTIME_ERROR;
        }
        int iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            printf("recv: \n%s\n", recvbuf);
            // Echo the buffer back to the sender
            int iSendResult = send(clientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        } else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
    }
}
