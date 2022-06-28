//
// Created by michael on 2022/6/28.
//

#include "lib4http.h"

http_client_config hcc_init() {
    http_client_config new;
    memset(new.hostname, '\0', sizeof(new.hostname));
    new.http = hmc_init();
    return new;
}

http_base_config hmc_init() {
    http_base_config new;
    new.pipe      = NULL;
    new.sock_host = INVALID_SOCKET;
    return new;
}

int GetHostName(ByteString *recvBuf, char host_name[HOST_NAME_MAX_LEN]) {
    StrList *result = FindSubStr(recvBuf->ch, "\r\n");
    if (result->length > 5) {
        for (int i = 0; i < result->length; ++i) {
            StrList *sub_res = FindSubStr(result->list[i], "Host:");
            if (sub_res->length > 1) {
                strcpy(host_name, sub_res->list[1]);
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

    int result_label = 0;


    if (strlen(config.hostname) == 0) {
        goto Failed;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result_label = getaddrinfo(config.hostname, "80", &hints, &result);
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

    http_base_config conn_config = hmc_init();

    conn_config.sock_host = host_sock;
    return conn_config;

Failed:
    freeaddrinfo(result);
    return hmc_init();
}

int check_http_config(http_base_config config) {
    if (config.sock_host == INVALID_SOCKET) {
        return 1;
    }
    return 0;
}

