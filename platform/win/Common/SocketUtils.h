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
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

void hello_common();


/**
 * @brief Init socket to match the version socket 2.2
 * 
 * @return int 
 */
int SocketInit();

/**
 * @brief Create a protocol of tcp socket
 * 
 * @return SOCKET 
 */
SOCKET SocketTcpCreate(struct addrinfo config);

int test_server();
int test_client(int argc, const char* argv[]);
#endif //SIMPLEWEBPROXY_SOCKETUTILS_H
