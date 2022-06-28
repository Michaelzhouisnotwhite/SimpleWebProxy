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
    int     iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
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
    hints.ai_family   = AF_INET;
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

int SocketRecv(http_base_config_t config_t, pthread_mutex_t *MemoryMutex) {
    int  nRes;
    char buf[RECV_BUFLEN] = {0};

    nRes = recv(config_t->sock_host, buf, RECV_BUFLEN - 1, 0);
    if (nRes > 0) {
        pthread_mutex_lock(MemoryMutex);
        ERROR_CODE iRes = ByteStringAdd(&(config_t->pipe), buf, nRes);
        pthread_mutex_unlock(MemoryMutex);

        if (iRes == ByteListError) {
            return SOCKET_RUNTIME_ERROR;
        }

    } else if (nRes == 0) {
        printf("[LOG] End of Receiving\n");
        ZeroMemory(buf, sizeof(buf));
        return SOCKET_RECEIVE_END;
    } else {
        printf("[LOG] Receive Error.\n");
        ZeroMemory(buf, sizeof(buf));
        return SOCKET_RUNTIME_ERROR;
    }
    ZeroMemory(buf, sizeof(buf));
    return SOCKET_RUNTIME_SUCCESS;
}

int SocketSend(http_base_config sender) {
    if (sender.pipe == NULL) {
        goto Fail;
    }
    int res_code = send(sender.sock_host, sender.pipe->ch, (int) sender.pipe->len, 0);
    if (res_code == SOCKET_ERROR) {
        goto Fail;
    }
    return 0;
Fail:
    return 1;
}
