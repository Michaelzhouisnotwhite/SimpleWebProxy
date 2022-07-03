//
// Created by michael on 2022/6/25.
//

#include "proxy_server.h"
#include "SocketException.h"
#include <pthread.h>


void ServerHandleClient(handle_client_args *args) {
    client_config clientConfig = client_config_init();
    clientConfig.super_config.sock_host = *args->clientSocket;

    struct sockaddr_in addr        = args->addr_info;
    pthread_mutex_t    MemoryMutex = args->mux;
    base_config        hostConfig;
    while (1) {
        RUNTIME_CODE rCode = SocketRecv(&clientConfig.super_config, &MemoryMutex);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(clientConfig.super_config.sock_host);
            FreeByteString(&clientConfig.super_config.pipe);
            shutdown(clientConfig.super_config.sock_host, SD_RECEIVE);
            break;
        } else if (rCode == SOCKET_RECEIVE_END) {
            closesocket(clientConfig.super_config.sock_host);
            FreeByteString(&clientConfig.super_config.pipe);
            shutdown(clientConfig.super_config.sock_host, SD_RECEIVE);
            break;
        }

        if (strlen(clientConfig.host.name) == 0) {
            rCode = GetHostName(clientConfig.super_config.pipe, &clientConfig.host);
            if (rCode != 0) {
                ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
                continue;
            }
            printf("[*] %s:", inet_ntoa(addr.sin_addr));
            printf("%d => ", addr.sin_port);
            printf("%s", clientConfig.host.name);
            printf(":[%s] ", clientConfig.host.port);
            printf("%d bytes\n", clientConfig.super_config.pipe->len);

            hostConfig = SocketConnect(clientConfig.host);
            if (0 != check_base_config(hostConfig)) {
                ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
                continue;
            }
            handle_pipline_args hp_args = {
                    &hostConfig.sock_host,
                    &clientConfig.super_config.sock_host,
                    MemoryMutex
            };
            pthread_create(NULL, NULL, (void *(*)(void *)) ServerPipline, &hp_args);
        }
        hostConfig.pipe = clientConfig.super_config.pipe;
        rCode = SocketSend(hostConfig, (int) hostConfig.pipe->len);
        if (0 != rCode) {
            closesocket(clientConfig.super_config.sock_host);
            closesocket(hostConfig.sock_host);
            shutdown(hostConfig.sock_host, SD_BOTH);
            shutdown(clientConfig.super_config.sock_host, SD_BOTH);
            break;
        }
        FreeByteString(&clientConfig.super_config.pipe);
        hostConfig.pipe = NULL;
    }
    pthread_exit(NULL);
}


int ServerStart(const char *port) {
//    printf("Starting Server...\n");
    SOCKET       serverSocket;
    RUNTIME_CODE nRes = SocketListening(port, &serverSocket);
    printf("==== server is starting at 127.0.0.1:%s ====\n", port);

    if (nRes == SOCKET_RUNTIME_ERROR) {
        return SOCKET_RUNTIME_ERROR;
    }

    // turn to no-block mode
    u_long argp    = 1;
    int    iResult = ioctlsocket(serverSocket, FIONBIO, &argp);

    if (iResult != NO_ERROR) {
        printf("ioctlsocket failed with error: %ld\n", iResult);
        exit(-1);
    }

    ServerLoop(serverSocket);
    WSACleanup();
    return SOCKET_RUNTIME_SUCCESS;
}

_Noreturn void ServerLoop(const SOCKET serverSocket) {

    proxy_event_list_s event_list = proxy_event_list_init(8000);

    while (1) {
        struct sockaddr_in addr;
        int                addrlen = sizeof(addr);

        SOCKET clientSocket = accept(serverSocket, (SOCKADDR *) &addr, &addrlen);

        if (clientSocket != INVALID_SOCKET) {
            proxy_event_create(&event_list, clientSocket, 1);

            printf("client connected: %s:%d\n", inet_ntoa(addr.sin_addr), addr.sin_port);
        }
        for (proxy_event_ptr ptr = event_list.head; ptr != NULL; ptr = ptr->next) {
            int i_res = handle_event(ptr);
            handle_event_close(i_res, ptr);
//            pthread_create(NULL, NULL, (void *(*)(void *)) create_event_thread, ptr);
        }
        clear_closed_proxy_events(&event_list);
    }
}

void create_event_thread(proxy_event_ptr ptr) {
    pthread_mutex_lock(&ptr->mux);
    int i_res = handle_event(ptr);
    handle_event_close(i_res, ptr);
    pthread_mutex_unlock(&ptr->mux);
}

void handle_event_close(int ev_no, proxy_event_ptr event) {
    if (ev_no & EV_CLOSE_CLIENT) {
        closesocket(event->client_socket_id);
    }
    if (ev_no & EV_CLOSE_SERVER) {
        closesocket(event->server_socket_id);
    }
}

int handle_event(proxy_event_ptr event) {
    int res = socket_recv(event->client_socket_id, &event->client_buffer.buf);
    if (res == SOCKET_RECV_NOT_KNOW_ERR) {
        return 1;
    } else if (res == SOCKET_RECV_ERROR) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            event->is_closed = 1;
            return EV_CLOSE_CLIENT | EV_CLOSE_SERVER;
        }
    }
    if (!(event->event_id & EV_CLIENT_END)) {
        int f_res = check_http_end(event->client_buffer.buf);
        if (f_res == 1) {
            event->event_id |= EV_CLIENT_END;
        }
    }
    if (!(EV_DONE_PARSE_HOST & event->event_id)) {
        int i_res = GetHostName(event->client_buffer.buf, &event->server_host);
        if (i_res == 0) {
            event->event_id |= EV_DONE_PARSE_HOST;
        }
    }
    if (EV_DONE_PARSE_HOST & event->event_id && !(EV_CONNECT_SERVER & event->event_id) ||
        EV_HANDLE_REDIRECT_RESPOND & event->event_id) {
        if (!(event->event_id & EV_CONNECT_READY)) {
            socket_connect_establish(&event->server_host);
            event->event_id |= EV_CONNECT_READY;
        }
        if (event->event_id & EV_CONNECT_READY) {
            int i_res = socket_connect(event->server_host, &event->server_socket_id);
            if (i_res == SOCKET_CONN_ERROR) {
//                if (WSAGetLastError() != WSAEWOULDBLOCK) {
//                    event->is_closed = 1;
//                    return EV_CLOSE_SERVER | EV_CLOSE_CLIENT;
//                }
                printf("waiting for connection...\n");
            } else if (i_res == SOCKET_CONN_SUCCESS) {
                event->event_id |= EV_CONNECT_SERVER;
                event->event_id |= EV_READY_HEADER_CHECK;
                printf("connection established: %s:%s\n", event->server_host.name, event->server_host.port);
            }
        }
    }
    if (EV_CONNECT_SERVER & event->event_id) {
        int s_res = socket_send(event->server_socket_id, &event->client_buffer);
        if (s_res != 0) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                event->is_closed = 1;
                return EV_CLOSE_SERVER | EV_CLOSE_CLIENT;
            } else {

            }
        }
//        event->event_id |= EV_RECEIVE_SERVER;

        int r_res = socket_recv(event->server_socket_id, &event->server_buffer.buf);
        switch (r_res) {
            case SOCKET_RECV_NOT_KNOW_ERR:
                return 1;
            case SOCKET_RECV_ERROR: {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    event->is_closed = 1;
                    return EV_CLOSE_SERVER | EV_CLOSE_CLIENT;
                }
                break;
            }
            case SOCKET_RECV_END: {
                event->event_id |= EV_SERVER_END;
                break;
            }
            default:
                break;
        }

        if (event->server_buffer.buf != NULL && (event->event_id & EV_CONNECT_SERVER) &&
            !(event->event_id & EV_HEADER_CHECKED)) {
            proxy_event_check_header(event);
            printf("respond content-length: %d\n", event->respond_header_info.content_length);
            printf("respond status: %d\n", event->respond_header_info.status_code);
            printf("respond header length: %d\n", event->respond_header_info.header_length);
        }
        if (event->event_id & EV_HEADER_CHECKED) {
            if (event->respond_header_info.status_code == -1) {
                return EV_CLOSE_CLIENT | EV_CLOSE_SERVER;
            } else if (event->respond_header_info.status_code < 400 && event->respond_header_info.status_code > 300) {
                event->event_id |= EV_HANDLE_REDIRECT_RESPOND;
            }
        }
        if (!(event->event_id & EV_SERVER_END) && event->event_id & EV_HEADER_CHECKED) {
            int prs_res = proxy_res_send(event);
            if (prs_res != 0) {
                event->is_closed = 1;
                return EV_CLOSE_SERVER | EV_CLOSE_CLIENT;
            }
        }

        if (event->event_id & EV_SERVER_END && event->server_buffer.buf == NULL) {
            if (event->event_id & EV_DONE_PARSE_HOST) {
                event->server_host = host_info_init();
//                event->event_id ^= EV_DONE_PARSE_HOST;
            }
            if (!(event->event_id & EV_HANDLE_REDIRECT_RESPOND)) {
                event->is_closed = 1;
                return EV_CLOSE_CLIENT | EV_CLOSE_SERVER;
            }
            if (event->event_id & EV_HANDLE_REDIRECT_RESPOND) {
                event->event_id = 0;
                return EV_CLOSE_SERVER;
            }
            event->event_id = 0;
        }
    }

    return 0;
}

void ServerPipline(handle_pipline_args *args) {
    base_config hostConfig = base_config_init();
    hostConfig.sock_host = *args->hostSocket;

    base_config clientConfig = base_config_init();
    clientConfig.sock_host = *args->clientSocket;
    int rCode;
    while (1) {
        rCode = SocketRecv(&hostConfig, &args->mux);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(hostConfig.sock_host);
            closesocket(clientConfig.sock_host);
            shutdown(hostConfig.sock_host, SD_BOTH);
            shutdown(clientConfig.sock_host, SD_BOTH);
            return;
        } else if (rCode == SOCKET_RECEIVE_END) {
            closesocket(hostConfig.sock_host);
            shutdown(hostConfig.sock_host, SD_BOTH);
            shutdown(clientConfig.sock_host, SD_BOTH);
            printf("\n");
            return;
        }
        clientConfig.pipe = hostConfig.pipe;
        rCode = SocketSend(clientConfig, (int) clientConfig.pipe->len);
        if (0 != rCode) {
            goto send_err;
        }

        StrList *res = FindSubStr(clientConfig.pipe->ch, clientConfig.pipe->len, "\r\n\r\n");

        if (strlen(res->list[res->length - 1]) == 0) {
            goto send_err;
        }

        FreeByteString(&hostConfig.pipe);
        clientConfig.pipe = NULL;
    }
send_err:
    closesocket(clientConfig.sock_host);
    closesocket(hostConfig.sock_host);
    shutdown(clientConfig.sock_host, SD_BOTH);
    shutdown(hostConfig.sock_host, SD_BOTH);
    FreeByteString(&hostConfig.pipe);
}
