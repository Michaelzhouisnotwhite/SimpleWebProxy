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


typedef struct {
    SOCKET     sock_host;
    ByteString *pipe;
} *http_base_config_t, http_base_config;

typedef struct HttpClientConfig {
    http_base_config http;
    char             hostname[HOST_NAME_MAX_LEN];
} http_client_config, *http_client_config_t;

int GetHostName(ByteString *recvBuf, char host_name[HOST_NAME_MAX_LEN]);

http_base_config hmc_init();

http_base_config HttpConnect(http_client_config config);

int check_http_config(http_base_config config);

http_client_config hcc_init();

#endif //SIMPLEWEBPROXY_LIB4HTTP_H
