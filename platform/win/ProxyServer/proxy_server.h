//
// Created by michael on 2022/6/25.
//
#pragma once

#include <stdio.h>
#include "SocketUtils.h"
#include "lib4str.h"
#include "lib4http.h"
#include "lib4proxy.h"


#define  EV_CLOSE_CLIENT 0b1
#define  EV_CLOSE_SERVER 0b10
#define  EV_DESTROY 0b100



//#define EV_RECEIVE_SERVER 0b0100

typedef struct {
    SOCKET             *clientSocket;
    struct sockaddr_in addr_info;
    pthread_mutex_t    mux;
} handle_client_args;

typedef struct {
    SOCKET          *hostSocket;
    SOCKET          *clientSocket;
    pthread_mutex_t mux;
} handle_pipline_args;


int ServerStart(const char *port);

/**
 * @brief handle clients
 * @return int
 */
void ServerHandleClient(handle_client_args *args);

_Noreturn void ServerLoop(SOCKET serverSocket);

void handle_event_close(int ev_no, proxy_event_ptr event);

void ServerPipline(handle_pipline_args *args);

int handle_event(proxy_event_ptr event);

void create_event_thread(proxy_event_ptr ptr);

