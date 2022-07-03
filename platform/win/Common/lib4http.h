//
// Created by michael on 2022/6/28.
//

#ifndef SIMPLEWEBPROXY_LIB4HTTP_H
#define SIMPLEWEBPROXY_LIB4HTTP_H

#include <winsock2.h>
#include "lib4str.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define HOST_NAME_MAX_LEN 300
#define HTTP_TRANSFER_TYPE int
#define HTTP_HEADER_LEN int
#define INVALID_CHUNK -1
#define HTTP_CHUNKED -100

typedef struct {
    SOCKET     sock_host;
    ByteString *pipe;
    HTTP_TRANSFER_TYPE is_chunked;
} *base_config_t, base_config;

typedef struct {
    char name[HOST_NAME_MAX_LEN];
    char port[5];

    SOCKET host_sock[100];
    int    host_sock_pos;
    struct addrinfo *result;
    int    host_sock_len;
} host_info_s;

typedef struct httpClientConfig {
    base_config super_config;
    host_info_s host;
} client_config, *client_config_t;

typedef struct http_header_info_s {
    int content_length;
    int send_length;
    int status_code;
    int header_length;
} http_header_info_s;

int GetHostName(ByteString *recvBuf, host_info_s *host_info);

base_config base_config_init();

int check_base_config(base_config config);

host_info_s host_info_init();

client_config client_config_init();

http_header_info_s http_header_info_init();

int check_http_header(base_config_t config);

int check_chunk_buffer(ByteString *http_buffer);

int get_header_length(ByteString *http_buffer);

int check_http_end(ByteString *http_buffer);

int check_respond_status_code(ByteString *buffer);

#endif //SIMPLEWEBPROXY_LIB4HTTP_H
