//
// Created by michael on 2022/6/25.
//
#pragma once
#include <stdio.h>
#include "SocketUtils.h"
typedef struct thread_args{
    SOCKET clientSocket;
    struct sockaddr_in addr_info;
    pthread_mutex_t mux;
}thread_args;

int ServerStart(const char* port);

/**
 * @brief handle clients
 * TODO: complete the references
 * @return int 
 */
void ServerHandleClient(thread_args* args);
void ServerLoop(SOCKET serverSocket);
int SocketRecv(SOCKET socketId, char** recvBuf, int* resBufLen, pthread_mutex_t *MemoryMutex);
int ServerForward(SOCKET sockId, char *clientBuf, int clientBufLen, int* clientBufPtr);
