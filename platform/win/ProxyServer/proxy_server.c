//
// Created by michael on 2022/6/25.
//

#include "proxy_server.h"
#include "SocketException.h"
#include <pthread.h>


void ServerHandleClient(handle_client_args *args) {
    client_config clientConfig = client_config_init();
    clientConfig.super_config.sock_host = *args->clientSocket;

    struct sockaddr_in addr        = args->addr_info;
    pthread_mutex_t    MemoryMutex = args->mux;
    base_config        hostConfig;
    while (1) {
        RUNTIME_CODE rCode = SocketRecv(&clientConfig.super_config, &MemoryMutex);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(clientConfig.super_config.sock_host);
            FreeByteString(&clientConfig.super_config.pipe);
            shutdown(clientConfig.super_config.sock_host, SD_RECEIVE);
            break;
        } else if (rCode == SOCKET_RECEIVE_END) {
            closesocket(clientConfig.super_config.sock_host);
            FreeByteString(&clientConfig.super_config.pipe);
            shutdown(clientConfig.super_config.sock_host, SD_RECEIVE);
            break;
        }

        if (strlen(clientConfig.host.name) == 0) {
            rCode = GetHostName(clientConfig.super_config.pipe, &clientConfig.host);
            if (rCode != 0) {
                ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
                continue;
            }
            printf("[*] %s:", inet_ntoa(addr.sin_addr));
            printf("%d => ", addr.sin_port);
            printf("%s", clientConfig.host.name);
            printf(":[%s] ", clientConfig.host.port);
            printf("%d bytes\n", clientConfig.super_config.pipe->len);

            hostConfig = SocketConnect(clientConfig.host);
            if (0 != check_base_config(hostConfig)) {
                ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
                continue;
            }
            handle_pipline_args hp_args = {
                    &hostConfig.sock_host,
                    &clientConfig.super_config.sock_host,
                    MemoryMutex
            };
            pthread_create(NULL, NULL, (void *(*)(void *)) ServerPipline, &hp_args);
        }
        hostConfig.pipe = clientConfig.super_config.pipe;
        rCode = SocketSend(hostConfig, (int) hostConfig.pipe->len);
        if (0 != rCode) {
            closesocket(clientConfig.super_config.sock_host);
            closesocket(hostConfig.sock_host);
            shutdown(hostConfig.sock_host, SD_BOTH);
            shutdown(clientConfig.super_config.sock_host, SD_BOTH);
            break;
        }
        FreeByteString(&clientConfig.super_config.pipe);
        hostConfig.pipe = NULL;
    }
    pthread_exit(NULL);
}

int ServerStart(const char *port) {
//    printf("Starting Server...\n");
    SOCKET       serverSocket;
    RUNTIME_CODE nRes = SocketListening(port, &serverSocket);
    printf("==== server is starting at 127.0.0.1:%s ====\n", port);
    if (nRes == SOCKET_RUNTIME_ERROR) {
        return SOCKET_RUNTIME_ERROR;
    }
    ServerLoop(&serverSocket);
    shutdown(serverSocket, SD_SEND);
    WSACleanup();
    pthread_exit(NULL);
    return SOCKET_RUNTIME_SUCCESS;
}

void ServerLoop(const SOCKET *serverSocket) {
    pthread_mutex_t MemoryMutex;
    if (pthread_mutex_init(&MemoryMutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return;
    }
    while (1) {
        struct sockaddr_in addr;
        int                addrlen = sizeof(addr);

        SOCKET clientSocket = accept(*serverSocket, (SOCKADDR *) &addr, &addrlen);

        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            continue;
        }
        pthread_t          thread;
        handle_client_args args;
        args.clientSocket = &clientSocket;
        args.addr_info    = addr;
        args.mux          = MemoryMutex;
        int rc = pthread_create(&thread, NULL, (void *(*)(void *)) ServerHandleClient, (void *) &args);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            break;
        }
    }
    pthread_exit(NULL);
    pthread_mutex_destroy(&MemoryMutex);
}


void ServerPipline(handle_pipline_args *args) {
    base_config hostConfig = base_config_init();
    hostConfig.sock_host = *args->hostSocket;

    base_config clientConfig = base_config_init();
    clientConfig.sock_host = *args->clientSocket;
    int rCode;
    while (1) {
        rCode = SocketRecv(&hostConfig, &args->mux);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(hostConfig.sock_host);
            closesocket(clientConfig.sock_host);
            shutdown(hostConfig.sock_host, SD_BOTH);
            shutdown(clientConfig.sock_host, SD_BOTH);
            return;
        } else if (rCode == SOCKET_RECEIVE_END) {
            closesocket(hostConfig.sock_host);
            shutdown(hostConfig.sock_host, SD_BOTH);
            shutdown(clientConfig.sock_host, SD_BOTH);
            printf("\n");
            return;
        }
        clientConfig.pipe = hostConfig.pipe;
        rCode = SocketSend(clientConfig, (int) clientConfig.pipe->len);
        if (0 != rCode) {
            goto send_err;
        }

        StrList *res = FindSubStr(clientConfig.pipe->ch, clientConfig.pipe->len, "\r\n\r\n");

        if (strlen(res->list[res->length - 1]) == 0) {
//            shutdown(hostConfig.sock_host, SD_BOTH);
        }

        FreeByteString(&hostConfig.pipe);
        clientConfig.pipe = NULL;
    }
send_err:
    closesocket(clientConfig.sock_host);
    closesocket(hostConfig.sock_host);
    shutdown(clientConfig.sock_host, SD_BOTH);
    shutdown(hostConfig.sock_host, SD_BOTH);
    FreeByteString(&hostConfig.pipe);
}
