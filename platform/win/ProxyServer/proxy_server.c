//
// Created by michael on 2022/6/25.
//

#include "proxy_server.h"
#include "SocketException.h"
#include <pthread.h>
int SocketRecv(SOCKET socketId, char **recvBuf, int* resBufLen, pthread_mutex_t *MemoryMutex) {
    int nRes;
    char buf[RECV_BUFLEN] = {0};

    pthread_mutex_lock(MemoryMutex);
    *recvBuf = (char *) calloc(RECV_BUFLEN + 1, sizeof(char));
    pthread_mutex_unlock(MemoryMutex);

    int recvBufLen = RECV_BUFLEN;
    int recvBufPtr = 0;

    int forwardBufPtr = 0;
    do {
        nRes = recv(socketId, buf, RECV_BUFLEN, 0);
        if (nRes > 0) {
            int recvBufRemain = recvBufLen - recvBufPtr;
            if (recvBufRemain >= nRes) {
                pthread_mutex_lock(MemoryMutex);
                memcpy(*recvBuf + recvBufPtr, buf, nRes);
                pthread_mutex_unlock(MemoryMutex);
                recvBufPtr += nRes;
                *resBufLen = recvBufPtr;
            } else {
                recvBufLen *= 2;
                char *newBuf = (char *) calloc(recvBufLen + 1, sizeof(char));

                pthread_mutex_lock(MemoryMutex);
                memcpy(newBuf, *recvBuf, recvBufPtr);
                memcpy(newBuf + recvBufPtr, buf, nRes);
                pthread_mutex_unlock(MemoryMutex);

                recvBufPtr += nRes;
                free(*recvBuf);
                *recvBuf = newBuf;
                *resBufLen = recvBufPtr;
            }
            ServerForward(socketId, *recvBuf, recvBufPtr, &forwardBufPtr);
        }
        else if (nRes == 0) {
            printf("[LOG] End of Receiving\n");
            *resBufLen = recvBufPtr;
            break;
        }
        else {
            printf("[LOG] Receive Error.\n");
            return SOCKET_RUNTIME_ERROR;
        }
    } while (nRes > 0);
    return SOCKET_RUNTIME_SUCCESS;
}

void ServerHandleClient(thread_args *args) {
    SOCKET clientSocket = args->clientSocket;
    struct sockaddr_in addr = args->addr_info;
    pthread_mutex_t MemoryMutex = args->mux;
    char *recvBuf;
    int recvBufLen;
    RUNTIME_CODE rCode = SocketRecv(clientSocket, &recvBuf, &recvBufLen, &MemoryMutex);
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
    SOCKET serverSocket;
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
        int addrlen = sizeof addr;
        SOCKET clientSocket = accept(serverSocket, (SOCKADDR *) &addr, &addrlen);

        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            break;
        }
        pthread_t thread;
        thread_args args;
        args.clientSocket = clientSocket;
        args.addr_info = addr;
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

int ServerForward(SOCKET sockId, char *clientBuf, int clientBufLen, int *clientBufPtr) {

    return 0;
}


