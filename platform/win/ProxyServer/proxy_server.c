//
// Created by michael on 2022/6/25.
//

#include "proxy_server.h"
#include "SocketException.h"
#include <pthread.h>


int SocketRecv(http_client_config_t config_t, pthread_mutex_t *MemoryMutex) {
    int  nRes;
    char buf[RECV_BUFLEN] = {0};

    pthread_mutex_lock(MemoryMutex);
//    *recvBuf = (char *) calloc(RECV_BUFLEN, sizeof(char));
    pthread_mutex_unlock(MemoryMutex);

    int recvBufLen = RECV_BUFLEN;
    int recvBufPtr = 0;
    do {
        nRes = recv(config_t->http.sock_host, buf, RECV_BUFLEN - 1, 0);
        if (nRes > 0) {
            int recvBufRemain = recvBufLen - recvBufPtr;
            if (recvBufRemain >= nRes) {

                pthread_mutex_lock(MemoryMutex);
                ByteStringAdd(&(config_t->http.pipe), buf, nRes);
                pthread_mutex_unlock(MemoryMutex);

            } else {
                recvBufLen *= 2;
                pthread_mutex_lock(MemoryMutex);
                char *newBuf = (char *) calloc(recvBufLen, sizeof(char));
                pthread_mutex_unlock(MemoryMutex);

                pthread_mutex_lock(MemoryMutex);
                memcpy(newBuf, *recvBuf, recvBufPtr);
                memcpy(newBuf + recvBufPtr, buf, nRes);
                pthread_mutex_unlock(MemoryMutex);

                recvBufPtr += nRes;
                free(*recvBuf);
                *recvBuf   = newBuf;
                *resBufLen = recvBufPtr;
            }
        } else if (nRes == 0) {
            printf("[LOG] End of Receiving\n");
            *resBufLen = recvBufPtr;
            break;
        } else {
            printf("[LOG] Receive Error.\n");
            return SOCKET_RUNTIME_ERROR;
        }
    } while (nRes > 0);
    return SOCKET_RUNTIME_SUCCESS;
}

void ServerHandleClient(thread_args *args) {
    SOCKET             clientSocket = args->clientSocket;
    struct sockaddr_in addr         = args->addr_info;
    pthread_mutex_t    MemoryMutex  = args->mux;
    char               *recvBuf;
    int                recvBufLen;

    // TODO FIX:
//    RUNTIME_CODE       rCode        = SocketRecv(clientSocket,
//                                                 &recvBuf,
//                                                 &recvBufLen,
//                                                 &MemoryMutex);
    if (rCode == SOCKET_RUNTIME_ERROR) {
        closesocket(clientSocket);
    }
    printf("Bytes received length: %d\n", recvBufLen);
    printf("addr: %s\n", inet_ntoa(addr.sin_addr));
    printf("addr port: %d\n", addr.sin_port);
    printf("recv: \n%s\n", recvBuf);
    ClearBuf(&recvBuf);

    if (rCode != SOCKET_RUNTIME_ERROR) {
        int nRes = shutdown(clientSocket, SD_SEND);
        if (nRes == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
        }
        closesocket(clientSocket);
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
        pthread_t   thread;
        thread_args args;
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




