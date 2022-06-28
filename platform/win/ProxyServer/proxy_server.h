//
// Created by michael on 2022/6/25.
//
#pragma once

#include <stdio.h>
#include "SocketUtils.h"
#include "lib4str.h"
#include "lib4http.h"

typedef struct {
    SOCKET             clientSocket;
    struct sockaddr_in addr_info;
    pthread_mutex_t    mux;
} handle_client_args;

typedef struct {
    SOCKET          hostSocket;
    SOCKET          clientSocket;
    pthread_mutex_t mux;
} handle_pipline_args;


int ServerStart(const char *port);

/**
 * @brief handle clients
 * @return int
 */
void ServerHandleClient(handle_client_args *args);

void ServerLoop(SOCKET serverSocket);

void ServerPipline(handle_pipline_args *args);