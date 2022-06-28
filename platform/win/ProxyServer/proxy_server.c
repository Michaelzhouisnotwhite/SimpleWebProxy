//
// Created by michael on 2022/6/25.
//

#include "proxy_server.h"
#include "SocketException.h"
#include <pthread.h>


void ServerHandleClient(handle_client_args *args) {
    client_config clientConfig = client_config_init();
    clientConfig.super_config.sock_host = args->clientSocket;

    struct sockaddr_in addr        = args->addr_info;
    pthread_mutex_t    MemoryMutex = args->mux;

    printf("[*] %s:", inet_ntoa(addr.sin_addr));
    printf("%d => ", addr.sin_port);

    while (1) {
        RUNTIME_CODE rCode = SocketRecv(&clientConfig.super_config, &MemoryMutex);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(clientConfig.super_config.sock_host);
            return;
        } else if (rCode == SOCKET_RECEIVE_END) {

        }

        if (strlen(clientConfig.host.name) == 0) {
            rCode = GetHostName(clientConfig.super_config.pipe, &clientConfig.host);
            if (rCode != 0) {
                ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
                continue;
            }
            // TODO(It only can run once)
            printf("%s", clientConfig.host.name);
            printf(":[%s] ", clientConfig.host.port);
//            printf("%d bytes\n", clientConfig.super_config.pipe->len);
        }

        base_config hostConfig = SocketConnect(clientConfig.host);
        if (0 != check_base_config(hostConfig)) {
            ZeroMemory(&clientConfig.host, sizeof(clientConfig.host));
            continue;
        }
        hostConfig.pipe = clientConfig.super_config.pipe;
        rCode = SocketSend(hostConfig, (int) hostConfig.pipe->len);
        if (0 != rCode) {
            closesocket(clientConfig.super_config.sock_host);
            closesocket(hostConfig.sock_host);
            shutdown(clientConfig.super_config.sock_host, SD_SEND);
            return;
        }
        pthread_t           thread;
        handle_pipline_args hp_args = {
                hostConfig.sock_host,
                clientConfig.super_config.sock_host,
                MemoryMutex
        };
        pthread_create(&thread, NULL, (void *(*)(void *)) ServerPipline, &hp_args);
        FreeByteString(&clientConfig.super_config.pipe);
        hostConfig.pipe = NULL;
    }
}

int ServerStart(const char *port) {
    printf("Starting Server...\n");
    SOCKET       serverSocket;
    RUNTIME_CODE nRes = SocketListening(port, &serverSocket);
    printf("Server Listening...\n");
    if (nRes == SOCKET_RUNTIME_ERROR) {
        return SOCKET_RUNTIME_ERROR;
    }
    ServerLoop(serverSocket);
    WSACleanup();
    pthread_exit(NULL);
    return SOCKET_RUNTIME_SUCCESS;
}

void ServerLoop(SOCKET serverSocket) {
    pthread_mutex_t MemoryMutex;
    if (pthread_mutex_init(&MemoryMutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return;
    }
    while (1) {
        struct sockaddr_in addr;
        int                addrlen      = sizeof addr;
        SOCKET             clientSocket = accept(serverSocket, (SOCKADDR *) &addr, &addrlen);

        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            break;
        }
        pthread_t          thread;
        handle_client_args args;
        args.clientSocket = clientSocket;
        args.addr_info    = addr;
        args.mux          = MemoryMutex;
        int rc = pthread_create(&thread, NULL, (void *(*)(void *)) ServerHandleClient, (void *) &args);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            break;
        }
    }
    pthread_mutex_destroy(&MemoryMutex);
}


void ServerPipline(handle_pipline_args *args) {
    base_config hostConfig = base_config_init();
    hostConfig.sock_host = args->hostSocket;

    base_config clientConfig = base_config_init();
    clientConfig.sock_host = args->clientSocket;
    int rCode;
    printf("Respond :");

    int header_length = -1;
    while (1) {
        rCode = SocketRecv(&hostConfig, &args->mux);
        if (rCode == SOCKET_RUNTIME_ERROR) {
            closesocket(hostConfig.sock_host);
            return;
        } else if (rCode == SOCKET_RECEIVE_END) {
            closesocket(hostConfig.sock_host);
            printf("\n");
            return;
        }
        clientConfig.pipe = hostConfig.pipe;
//        printf("%s", clientConfig.pipe->ch);
        rCode = SocketSend(clientConfig, (int) clientConfig.pipe->len);
        if (0 != rCode) {
            goto send_err;
        }
        FreeByteString(&hostConfig.pipe);
        clientConfig.pipe = NULL;
        continue;

        // TODO: FIX THIS!!
        if (clientConfig.is_chunked == INVALID_CHUNK) {
            rCode = check_http_header(&clientConfig);
            if (rCode != 0) {
                continue;
            }
        }

        if (clientConfig.is_chunked) {
            if (header_length == -1) {
                header_length = get_header_length(clientConfig.pipe);
                if (header_length == -1) {
                    continue;
                }
            }
            if (header_length > 0) {
                rCode = SocketSend(clientConfig, header_length);
                if (0 != rCode) {
                    goto send_err;
                }
                rCode = ByteStrPopFront(hostConfig.pipe, header_length);
                clientConfig.pipe = hostConfig.pipe;
                if (rCode != 0) {
                    goto send_err;
                }
                header_length = -2;
            }
//            continue;
            int chunk_buffer_len = check_chunk_buffer(clientConfig.pipe);
            if (chunk_buffer_len == INVALID_CHUNK) {
                continue;
            } else {
                rCode = SocketSend(clientConfig, chunk_buffer_len);
                if (0 != rCode) {
                    goto send_err;
                }
                rCode = ByteStrPopFront(hostConfig.pipe, chunk_buffer_len);
                if (rCode != 0) {
                    goto send_err;
                }
            }
        } else {
            rCode = SocketSend(clientConfig, (int) clientConfig.pipe->len);
            if (0 != rCode) {
                goto send_err;
            }
            FreeByteString(&hostConfig.pipe);
            clientConfig.pipe = NULL;
        }
    }
send_err:
    closesocket(clientConfig.sock_host);
    closesocket(hostConfig.sock_host);
    shutdown(clientConfig.sock_host, SD_SEND);
    FreeByteString(&hostConfig.pipe);
}
