//
// Created by michael on 2022/6/25.
//

#include "SocketUtils.h"
#include "SocketException.h"

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

int SocketListening(const char *port, SOCKET *ResSocket) {

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
    SOCKET listenSocket = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);

    nRes = bind(listenSocket, addr_result->ai_addr, (int) addr_result->ai_addrlen);

    if (nRes == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        goto Error;
    }
    // Listen
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        goto Error;
    }

    *ResSocket = listenSocket;
    return SOCKET_RUNTIME_SUCCESS;

Error:
    closesocket(listenSocket);
    WSACleanup();
    return SOCKET_RUNTIME_ERROR;
}
base_config SocketConnect(host_info_s host_info) {


    struct addrinfo hints,
                    *result = NULL,
                    *ptr    = NULL;

    int result_label;


    if (strlen(host_info.name) == 0) {
        goto Failed;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result_label = getaddrinfo(host_info.name, host_info.port, &hints, &result);
    if (result_label != 0) {
        goto Failed;
    }
    SOCKET host_sock = INVALID_SOCKET;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        host_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (host_sock == INVALID_SOCKET) {
            goto Failed;
        }

        result_label = connect(host_sock, ptr->ai_addr, (int) ptr->ai_addrlen);

        if (result_label == SOCKET_ERROR) {
            closesocket(host_sock);
            host_sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    if (host_sock == INVALID_SOCKET) {
        closesocket(host_sock);
        goto Failed;
    }

    base_config conn_config = base_config_init();

    conn_config.sock_host = host_sock;
    return conn_config;

Failed:
    freeaddrinfo(result);
    return base_config_init();
}

int SocketRecv(base_config_t config_t, pthread_mutex_t *MemoryMutex) {
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

int SocketSend(base_config sender, int len) {
    if (sender.pipe == NULL) {
        goto Fail;
    }
    int res_code = send(sender.sock_host, sender.pipe->ch, len, 0);
    if (res_code == SOCKET_ERROR) {
        goto Fail;
    }
    return 0;
Fail:
    return 1;
}
