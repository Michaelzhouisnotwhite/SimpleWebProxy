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

void hello_common();



#endif //SIMPLEWEBPROXY_SOCKETUTILS_H
