//
// Created by michael on 2022/6/30.
//

#ifndef SIMPLEWEBPROXY_LIB4PROXY_H
#define SIMPLEWEBPROXY_LIB4PROXY_H

#define EV_NOEVNET 0
#define EV_RECEIVE_CLIENT 1
#define EV_CONNECT_SERVER 0b10
#define EV_PARSE_HOST 0b100
#define EV_RECEIVE_SERVER 0b1000
#define EVENT_ID int


#include "lib4str.h"
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include "lib4http.h"

#define SOCKET_RECV_BUF_LEN 8120

#define SOCKET_RECV_NOT_KNOW_ERR -3
#define SOCKET_RECV_END -1
#define SOCKET_RECV_ERROR -2
#define SOCKET_RECV_SUCCESS 0

#define SOCKET_CONN_NOT_KNOWN_ERROR -1
#define SOCKET_CONN_ERROR -2
#define SOCKET_CONN_SUCCESS 0

typedef struct {
    byte_string_t buf;
    int           pos;
} socket_buffer_s;


typedef struct proxy_event_s {
    SOCKET               client_socket_id;
    SOCKET               server_socket_id;
    host_info_s          server_host;
    socket_buffer_s      client_buffer;
    socket_buffer_s      server_buffer;
    EVENT_ID event_id;
    int                  is_closed;
    struct proxy_event_s *next;
    struct proxy_event_s *prev;
} proxy_event_s, *proxy_event_ptr;

typedef struct {
    proxy_event_s *head;
    proxy_event_s *back;
    int           max_length;
} proxy_event_list_s;

proxy_event_list_s proxy_event_list_init(int max_length);

void proxy_event_create(proxy_event_list_s *event_list, SOCKET sock_id, int ready_receive);

void clear_closed_proxy_events(proxy_event_list_s *event_list);

int socket_recv(SOCKET socket_id, byte_string_t *byte_string);

int socket_connect(host_info_s host_info, SOCKET *server);

int socket_send(SOCKET server, socket_buffer_s *socket_buffer);

#endif //SIMPLEWEBPROXY_LIB4PROXY_H