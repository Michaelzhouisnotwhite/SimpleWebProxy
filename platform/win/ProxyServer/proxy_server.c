//
// Created by michael on 2022/6/25.
//

#include "proxy_server.h"
#include "SocketException.h"
#include <pthread.h>


void ServerHandleClient(handle_client_args *args) {
    http_client_config clientConfig = hcc_init();

    clientConfig.http.sock_host = args->clientSocket;
    struct sockaddr_in addr        = args->addr_info;
    pthread_mutex_t    MemoryMutex = args->mux;

    printf("addr: %s:", inet_ntoa(addr.sin_addr));
    printf("%d\n", addr.sin_port);

    while (1) {
        RUNTIME_CODE rCode = SocketRecv(&clientConfig.http, &MemoryMutex);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(clientConfig.http.sock_host);
            return;
        } else if (rCode == SOCKET_RECEIVE_END) {

        }

        printf("Bytes received length: %d\n", clientConfig.http.pipe->len);

        if (strlen(clientConfig.host.name) == 0) {
            rCode = GetHostName(clientConfig.http.pipe, &clientConfig.host);
            if (rCode != 0) {
                ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
                continue;
            }
            printf("Host name: <%s>", clientConfig.host.name);
            printf("port: [%s]\n", clientConfig.host.port);
        }

        http_base_config hostConfig = HttpConnect(clientConfig);
        if (0 != check_http_config(hostConfig)) {
            ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
            continue;
        }
        hostConfig.pipe = clientConfig.http.pipe;
        rCode = SocketSend(hostConfig);
        if (0 != rCode) {
            closesocket(clientConfig.http.sock_host);
            closesocket(hostConfig.sock_host);
            shutdown(clientConfig.http.sock_host, SD_SEND);
            return;
        }
        pthread_t           thread;
        handle_pipline_args hp_args = {hostConfig.sock_host, clientConfig.http.sock_host, MemoryMutex};
        pthread_create(&thread, NULL, (void *(*)(void *)) ServerPipline, &hp_args);
        FreeByteString(&clientConfig.http.pipe);
        hostConfig.pipe = NULL;
    }
}

int ServerStart(const char *port) {
    printf("Starting Server...\n");
    SOCKET       serverSocket;
    RUNTIME_CODE nRes = SocketListening(port, &serverSocket);
    printf("Server Listening...\n");
    if (nRes == SOCKET_RUNTIME_ERROR) {
        return SOCKET_RUNTIME_ERROR;
    }
    ServerLoop(serverSocket);
    WSACleanup();
    pthread_exit(NULL);
    return SOCKET_RUNTIME_SUCCESS;
}

void ServerLoop(SOCKET serverSocket) {
    pthread_mutex_t MemoryMutex;
    if (pthread_mutex_init(&MemoryMutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return;
    }
    while (1) {
        struct sockaddr_in addr;
        int                addrlen      = sizeof addr;
        SOCKET             clientSocket = accept(serverSocket, (SOCKADDR *) &addr, &addrlen);

        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            break;
        }
        pthread_t          thread;
        handle_client_args args;
        args.clientSocket = clientSocket;
        args.addr_info    = addr;
        //        ServerHandleClient(&args);

        args.mux = MemoryMutex;
        int rc = pthread_create(&thread, NULL, (void *(*)(void *)) ServerHandleClient, (void *) &args);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            break;
        }
    }
    pthread_mutex_destroy(&MemoryMutex);
}

int ServerForward(SOCKET sockId, char *clientBuf, int clientBufLen) {
    int iRes;
    iRes = send(sockId, clientBuf, clientBufLen, 0);
    if (iRes == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        //        closesocket(sockId);
        return 1;
    }
    return 0;
}


void ConnectHost(SOCKET *connectSocket, char *host_name) {
    struct addrinfo hints, *result = NULL;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int  iResult;
    char *res = StrStrim(host_name);
    iResult = getaddrinfo(res, "80", &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        return;
    }
    *connectSocket = INVALID_SOCKET;
    // Attempt to connect to an address until one succeeds
    for (struct addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        *connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                                ptr->ai_protocol);
        if (*connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            return;
        }
        // Connect to server.
        iResult = connect(*connectSocket, ptr->ai_addr, (int) ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(*connectSocket);
            *connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (*connectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
    }
}

int GetContentLength(char *recvBuf) {
    StrList *result     = FindSubStr(recvBuf, "\r\n");
    int     content_len = 0;
    if (result->length > 5) {
        for (int i = 0; i < result->length; ++i) {
            StrList *sub_res = FindSubStr(result->list[i], "Content-Length:");
            if (sub_res->length > 1) {
                char *cl = StrStrim(sub_res->list[1]);
                //                char * temp;
                content_len = strtol(cl, NULL, 10);
                FreeStrList(&sub_res);
                break;
            } else {
                FreeStrList(&sub_res);
            }
        }
    }
    FreeStrList(&result);
    return content_len;
}

http_transform_pipe_s httpTransformPipeSCreate() {
    http_transform_pipe_s new;
    new.receiver       = hbc_init();
    new.sender_sock_id = INVALID_SOCKET;
    return new;
}

void ServerPipline(handle_pipline_args *args) {
    http_base_config hostConfig = hbc_init();
    hostConfig.sock_host = args->hostSocket;

    http_base_config clientConfig = hbc_init();
    clientConfig.sock_host = args->clientSocket;
    int rCode;
    printf("Respond :");
    while (1) {
        rCode = SocketRecv(&hostConfig, &args->mux);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(hostConfig.sock_host);
            return;
        } else if (rCode == SOCKET_RECEIVE_END) {
            closesocket(hostConfig.sock_host);
            printf("\n");
            return;
        }
        clientConfig.pipe = hostConfig.pipe;
        printf("%s", clientConfig.pipe->ch);
        rCode = SocketSend(clientConfig);
        if (0 != rCode) {
            closesocket(clientConfig.sock_host);
            closesocket(hostConfig.sock_host);
            shutdown(clientConfig.sock_host, SD_SEND);
            FreeByteString(&hostConfig.pipe);
            return;
        }
        FreeByteString(&hostConfig.pipe);
        clientConfig.pipe = NULL;
    }
}
