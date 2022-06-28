//
// Created by michael on 2022/6/25.
//

#ifndef SIMPLEWEBPROXY_SOCKETUTILS_H
#define SIMPLEWEBPROXY_SOCKETUTILS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <pthread.h>
#include <stdio.h>

#include "lib4http.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#ifndef RECV_BUFLEN
#define RECV_BUFLEN 8192
#endif

#define MAX_HEADER_LEN 8192

/**
 * @brief Init socket to match the version socket 2.2
 * 
 * @return int 
 */
int SocketInit();

int SocketListening(const char *port, SOCKET *ResSocket);

int SocketRecv(base_config_t config_t, pthread_mutex_t *MemoryMutex);

base_config SocketConnect(host_info_s host_info);

int SocketSend(base_config sender, int len);

#endif //SIMPLEWEBPROXY_SOCKETUTILS_H
