//
// Created by michael on 2022/6/25.
//
#pragma once

#include <stdio.h>
#include "SocketUtils.h"
#include "lib4str.h"
#include "lib4http.h"

#define EV_RECEIVE_CLIENT 0b0001
#define EV_CONNECT_SERVER 0b0010
#define EVENT_ID int
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

typedef struct {
    byte_string_t buf;
    int pos;
}socket_buffer_s;


typedef struct {
    SOCKET        client_socket_id;
    SOCKET        server_socket_id;
    host_info_s   server_host;
    socket_buffer_s client_buffer;
    socket_buffer_s server_buffer;
    EVENT_ID event_id;
} proxy_event_s;


int ServerStart(const char *port);

/**
 * @brief handle clients
 * @return int
 */
void ServerHandleClient(handle_client_args *args);

void ServerLoop(const SOCKET *serverSocket);

void ServerPipline(handle_pipline_args *args);