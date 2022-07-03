//
// Created by michael on 2022/6/30.
//

#include "lib4proxy.h"

socket_buffer_s socket_buffer_init() {
    socket_buffer_s new = {NULL, 0};
    return new;
}

proxy_event_list_s proxy_event_list_init(int max_length) {
    proxy_event_list_s new;
    new.head       = NULL;
    new.max_length = max_length;
    return new;
}

void proxy_event_create(proxy_event_list_s *event_list, SOCKET sock_id, int ready_receive) {
    proxy_event_s *event = calloc(1, sizeof(proxy_event_s));
    if (event == NULL) {
        goto failed;
    }

    pthread_mutex_init(&event->mux, NULL);
    event->client_buffer       = socket_buffer_init();
    event->server_buffer       = socket_buffer_init();
    event->server_host         = host_info_init();

    if (ready_receive) {
        event->event_id = EV_RECEIVE_CLIENT;
    }
    event->next                = NULL;
    event->is_closed           = 0;
    event->client_socket_id    = sock_id;
    event->respond_header_info = http_header_info_init();
    event->server_socket_id    = INVALID_SOCKET;

    if (event_list->head == NULL) {
        event_list->head       = event;
        event_list->head->prev = NULL;

    } else {
        event->next            = event_list->head;
        event_list->head->prev = event;
        event_list->head       = event;
    }
    return;
failed:
    printf("malloc Failed");
    exit(-1);
}

proxy_event_ptr proxy_event_at(proxy_event_list_s *event_list, int index) {
    int                  enum_no = 0;
    for (proxy_event_ptr ptr     = event_list->head; ptr != NULL; ptr = ptr->next, ++enum_no) {
        if (enum_no == index) {
            return ptr;
        }
    }
    return NULL;
}

void clear_closed_proxy_events(proxy_event_list_s *event_list) {
    for (proxy_event_ptr ptr = event_list->head; ptr != NULL;) {
        if (ptr->is_closed == 1) {
            if (ptr->prev == NULL && ptr->next == NULL) {
                FreeByteString(&(ptr->server_buffer.buf));
                FreeByteString(&(ptr->client_buffer.buf));
                free(ptr);
                event_list->head = NULL;
                break;
            }
            proxy_event_ptr temp = ptr;
            if (ptr->prev == NULL) {
                ptr->next->prev = NULL;

                ptr = ptr->next;
                event_list->head = ptr;
                goto Free;
            }
            if (ptr->next == NULL) {
                ptr->prev->next = NULL;

                ptr = ptr->prev;
                goto Free;
            }
            if (ptr->next && ptr->prev) {
                ptr->prev->next = ptr->next;
                ptr->next->prev = ptr->prev->next;

                ptr = ptr->next;
                goto Free;
            }

Free:
            FreeByteString(&(temp->server_buffer.buf));
            FreeByteString(&(temp->client_buffer.buf));

            free(temp);
            temp = NULL;
        } else {
            ptr = ptr->next;
        }
    }
}

int socket_recv(SOCKET socket_id, byte_string_t *byte_string) {
    char buf[SOCKET_RECV_BUF_LEN] = {0};

    int nRes = recv(socket_id, buf, SOCKET_RECV_BUF_LEN - 1, 0);
    if (nRes > 0) {
        ERROR_CODE e_code = ByteStringAdd(byte_string, buf, nRes);
        if (e_code == ByteListError) {
            return SOCKET_RECV_NOT_KNOW_ERR;
        }
    } else if (nRes == 0) {
        return SOCKET_RECV_END;
    } else {
        return SOCKET_RECV_ERROR;
    }
    return SOCKET_RECV_SUCCESS;
}

int socket_connect_establish(host_info_s *host_info) {
    struct addrinfo hints,
                    *result = NULL,
                    *ptr    = NULL;

    int result_label;

    if (strlen(host_info->name) == 0) {
        return -1;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result_label = getaddrinfo(host_info->name, host_info->port, &hints, &result);
    if (result_label != 0) {
        return 1;
    }
    SOCKET host_sock = INVALID_SOCKET;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        host_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (host_sock == INVALID_SOCKET) {
            return 1;
        }
        // turn to no-block mode
        u_long argp    = 1;
        int    iResult = ioctlsocket(host_sock, FIONBIO, &argp);

        if (iResult != NO_ERROR) {
            printf("ioctlsocket failed with error: %ld\n", iResult);
            exit(-1);
        }
        host_info->host_sock[host_info->host_sock_len] = host_sock;
        host_info->host_sock_len++;
    }

    host_info->result = result;
    return 0;
}

int socket_connect(host_info_s host_info, SOCKET *server) {
    int                  i    = 0;
    int                  result_label;
    for (struct addrinfo *ptr = host_info.result; ptr != NULL; i++, ptr = ptr->ai_next) {
        result_label = connect(host_info.host_sock[i], ptr->ai_addr, (int) ptr->ai_addrlen);

        if (result_label == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEISCONN) {
                *server = host_info.host_sock[i];
                freeaddrinfo(host_info.result);
                return SOCKET_CONN_SUCCESS;
            }
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                closesocket(host_info.host_sock[i]);
                continue;
            }
        } else {
            *server = host_info.host_sock[i];
            freeaddrinfo(host_info.result);
            return SOCKET_CONN_SUCCESS;
        }
    }
    return SOCKET_CONN_ERROR;
}

int socket_send(SOCKET server, socket_buffer_s *socket_buffer) {
    if (socket_buffer->buf == NULL) {
        return 0;
    }
    printf("[log] 1\n");
    int send_len = (int) socket_buffer->buf->len - socket_buffer->pos;
    int res_code = send(server, socket_buffer->buf->ch + socket_buffer->pos,
                        send_len, 0);
    printf("[log] 2\n");

    if (res_code == SOCKET_ERROR) {
        printf("[log] 3\n");

        return 1;
    }
    if (res_code < send_len) {
        socket_buffer->pos = res_code + socket_buffer->pos;
        printf("[log] 4\n");

    }
    if (res_code == send_len) {
        FreeByteString(&socket_buffer->buf);
        socket_buffer->buf = NULL;
        socket_buffer->pos = 0;
        printf("[log] 5\n");

    }
    return 0;
}

int proxy_res_send(proxy_event_ptr event) {
    if (event->server_buffer.pos < event->respond_header_info.header_length &&
        !(event->event_id & EV_RES_HEADER_SENDED)) {

        int send_len = event->respond_header_info.header_length - event->server_buffer.pos;
        int res_code = send(event->client_socket_id, event->server_buffer.buf->ch + event->server_buffer.pos,
                            send_len, 0);
        if (res_code == SOCKET_ERROR) {
            return 1;
        }
        event->server_buffer.pos += res_code;
        if (res_code == send_len) {
            // Header Has Sended
            event->event_id |= EV_RES_HEADER_SENDED;
            event->counter_1 = 0;
        }

    }
    if (event->event_id & EV_RES_HEADER_SENDED) {
        // begin send body
        if (event->respond_header_info.content_length > 0) {
            // body is not chunked data
            int send_len = (int) event->server_buffer.buf->len - event->server_buffer.pos;
            int res_code = send(event->client_socket_id, event->server_buffer.buf->ch + event->server_buffer.pos,
                                send_len, 0);
            if (res_code > 0) {
                event->counter_1 += res_code;
                if (res_code == send_len) {
                    FreeByteString(&event->server_buffer.buf);
                    event->server_buffer.buf = NULL;
                    event->server_buffer.pos = 0;
                }
                if (res_code < send_len) {
                    event->server_buffer.pos = res_code + event->server_buffer.pos;
                }
                if (event->counter_1 >= event->respond_header_info.content_length) {
                    FreeByteString(&event->server_buffer.buf);
                    event->server_buffer.buf = NULL;
                    event->server_buffer.pos = 0;
                    event->event_id |= EV_SERVER_END;
                    return 0;
                }
            }
            if (res_code != 0) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    event->is_closed = 1;
                    return 1;
                }
            }
        } else if (event->respond_header_info.content_length == 0) {
            FreeByteString(&event->server_buffer.buf);
            event->server_buffer.buf = NULL;
            event->server_buffer.pos = 0;
            event->event_id |= EV_SERVER_END;
        } else {
            if (!(event->event_id & EV_SERVER_END)) {
                int c_res = check_http_end(event->server_buffer.buf); // 1 is found
                if (c_res == 1) {
                    event->event_id |= EV_SERVER_END;
                }
            }
            int ss_res = socket_send(event->client_socket_id, &event->server_buffer);
            if (ss_res != 0) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    event->is_closed = 1;
                    return 1;
                }
            }
        }
    }

    return 0;
}

int proxy_event_check_header(proxy_event_ptr event) {
    StrList *by_double_enter = FindSubStr(event->server_buffer.buf->ch, event->server_buffer.buf->len, "\r\n\r\n");
    if (by_double_enter->length > 1) {
        event->respond_header_info.header_length = (int) strlen(by_double_enter->list[0]) + 4;
        // Check content length
        StrList *by_content_length               = FindSubStr(by_double_enter->list[0],
                                                              strlen(by_double_enter->list[0]),
                                                              "Content-Length:");
        StrList *by_transfer_encoding            = FindSubStr(by_double_enter->list[0],
                                                              strlen(by_double_enter->list[0]),
                                                              "Transfer-Encoding:");
        if (!(by_content_length->length == 1 ^ by_transfer_encoding->length == 1)) {
            FreeStrList(&by_content_length);
            FreeStrList(&by_transfer_encoding);
            return 1;
        }
        if (by_content_length->length > 1) {
            char content_len_value[100] = {0};
            char *other                 = strstr(by_content_length->list[1], "\r\n");
            if (other != NULL) {
                strncpy(content_len_value, by_content_length->list[1], other - by_content_length->list[1]);
                event->respond_header_info.content_length = strtol(content_len_value, NULL, 10);
            } else {
                strncpy(content_len_value, by_content_length->list[1], strlen(by_content_length->list[1]));
                event->respond_header_info.content_length = strtol(content_len_value, NULL, 10);
            }
            goto Fin;
        }
        if (by_transfer_encoding->length > 1) {
            event->respond_header_info.content_length = HTTP_CHUNKED;
            goto Fin;
        }
Fin:
        FreeStrList(&by_content_length);
        FreeStrList(&by_transfer_encoding);
        // check status code
        event->respond_header_info.status_code = check_respond_status_code(event->server_buffer.buf);
        event->event_id |= EV_HEADER_CHECKED;
        return 0;
    }

    return 1;
}