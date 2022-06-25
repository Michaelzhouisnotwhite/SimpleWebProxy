//
// Created by michael on 2022/6/25.
//
#pragma once
#include <stdio.h>
#include "SocketUtils.h"
int server_func();

/**
 * @brief Init a port to start server 
 * @param ResSocket The result of Init socket
 * @param port (const char*)
 * @return int: Static Code
 */
int ServerInit(SOCKET *ResSocket, const char* port);
int ServerStart(SOCKET *listenSocket);

/**
 * @brief handle clients
 * TODO: complete the references
 * @return int 
 */
int ServerHandleClient(SOCKET, struct sockaddr, int);