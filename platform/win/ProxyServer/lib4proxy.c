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
    new.back       = NULL;
    new.max_length = max_length;
    return new;
}

void proxy_event_create(proxy_event_list_s *event_list, SOCKET sock_id, int ready_receive) {
    proxy_event_s *event = calloc(1, sizeof(proxy_event_s));
    if (event == NULL) {
        goto failed;
    }
    event->client_buffer    = socket_buffer_init();
    event->server_buffer    = socket_buffer_init();
    event->server_host      = host_info_init();

    if (ready_receive) {
        event->event_id = EV_RECEIVE_CLIENT;
    }
    event->next             = NULL;
    event->is_closed        = 0;
    event->client_socket_id = sock_id;
    event->server_socket_id = INVALID_SOCKET;

    if (event_list->head == NULL) {
        event_list->head       = event;
        event_list->back       = event;
        event_list->head->prev = NULL;

    } else {
        event_list->back->next = event;
        event->prev            = event_list->back;
        event_list->back       = event_list->back->next;
    }
    return;
failed:
    printf("malloc Failed");
    exit(-1);
}

void clear_closed_proxy_events(proxy_event_list_s *event_list) {

    for (proxy_event_ptr ptr = event_list->head; ptr != NULL;
         ptr = ptr->next) {
        if (ptr->prev == NULL && ptr->next == NULL) {
            FreeByteString(&(ptr->server_buffer.buf));
            FreeByteString(&(ptr->client_buffer.buf));
            free(ptr);
            event_list->head = NULL;
            event_list->back = NULL;
            break;
        }
        if (ptr->is_closed == 1) {
            proxy_event_ptr temp = ptr;
            if (temp->prev != NULL) {
                ptr->prev->next = ptr->next;
            }
            if (temp->next != NULL) {
                ptr->next->prev = ptr->prev;
            }
            FreeByteString(&(ptr->server_buffer.buf));
            FreeByteString(&(ptr->client_buffer.buf));
            if (ptr->prev != NULL) {
                ptr = ptr->prev;
            }
            free(temp);
            if (ptr->next == NULL) {
                break;
            }
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

int socket_connect(host_info_s host_info, SOCKET *server) {
    struct addrinfo hints,
                    *result = NULL,
                    *ptr    = NULL;

    int result_label;

    *server = INVALID_SOCKET;
    if (strlen(host_info.name) == 0) {
        return -1;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result_label = getaddrinfo(host_info.name, host_info.port, &hints, &result);
    if (result_label != 0) {
        return SOCKET_CONN_NOT_KNOWN_ERROR;
    }
    SOCKET host_sock = INVALID_SOCKET;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        host_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (host_sock == INVALID_SOCKET) {
            return SOCKET_CONN_NOT_KNOWN_ERROR;
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
        return SOCKET_CONN_ERROR;
    }

    *server = host_sock;
    return SOCKET_CONN_SUCCESS;
}

int socket_send(SOCKET server, socket_buffer_s *socket_buffer) {
    if (socket_buffer->buf == NULL) {
        return 0;
    }
    int send_len = (int) socket_buffer->buf->len - socket_buffer->pos;
    int res_code = send(server, socket_buffer->buf->ch + socket_buffer->pos,
                        send_len, 0);
    if (res_code == SOCKET_ERROR) {
        return 1;
    }
    if (res_code < send_len) {
        socket_buffer->pos = res_code + socket_buffer->pos;
    }
    if (res_code == send_len) {
        FreeByteString(&socket_buffer->buf);
        socket_buffer->buf = NULL;
        socket_buffer->pos = 0;
    }
    return 0;

}
