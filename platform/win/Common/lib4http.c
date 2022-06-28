//
// Created by michael on 2022/6/28.
//

#include "lib4http.h"

http_client_config hcc_init() {
    http_client_config new;
    memset(new.host.name, '\0', sizeof(new.host.name));
    memset(new.host.port, '\0', sizeof(new.host.port));
    new.http = hbc_init();
    return new;
}

http_base_config hbc_init() {
    http_base_config new;
    new.pipe      = NULL;
    new.sock_host = INVALID_SOCKET;
    return new;
}

int GetHostName(ByteString *recvBuf, host_db_s *hostDbS) {
    StrList *result = FindSubStr(recvBuf->ch, "\r\n");
    if (result->length > 5) {
        for (int i = 0; i < result->length; ++i) {
            StrList *sub_res = FindSubStr(result->list[i], "Host:");
            if (sub_res->length > 1) {
                StrList *find_port = FindSubStr(sub_res->list[1], ":");
                strcpy(hostDbS->name, StrStrim(find_port->list[0]));
                if (find_port->length > 1) {
                    strcpy(hostDbS->port, StrStrim(find_port->list[1]));
                }else {
                    strcpy(hostDbS->port, "80");
                }
                FreeStrList(&find_port);
                FreeStrList(&sub_res);
                break;
            } else {
                FreeStrList(&sub_res);
            }
        }
        return 0;
    }
    FreeStrList(&result);
    return 1;
}

http_base_config HttpConnect(http_client_config config) {


    struct addrinfo hints,
                    *result = NULL,
                    *ptr    = NULL;

    int result_label;


    if (strlen(config.host.name) == 0) {
        goto Failed;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result_label = getaddrinfo(config.host.name, config.host.port, &hints, &result);
    if (result_label != 0) {
        goto Failed;
    }
    SOCKET host_sock = INVALID_SOCKET;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        host_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (host_sock == INVALID_SOCKET) {
            goto Failed;
        }

        result_label = connect(host_sock, ptr->ai_addr, (int) ptr->ai_addrlen);

        if (result_label == SOCKET_ERROR) {
            closesocket(host_sock);
            host_sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    if (host_sock == INVALID_SOCKET) {
        closesocket(host_sock);
        goto Failed;
    }

    http_base_config conn_config = hbc_init();

    conn_config.sock_host = host_sock;
    return conn_config;

Failed:
    freeaddrinfo(result);
    return hbc_init();
}

int check_http_config(http_base_config config) {
    if (config.sock_host == INVALID_SOCKET) {
        return 1;
    }
    return 0;
}

