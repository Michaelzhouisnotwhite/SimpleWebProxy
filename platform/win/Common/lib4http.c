//
// Created by michael on 2022/6/28.
//

#include "lib4http.h"

client_config client_config_init() {
    client_config new;
    memset(new.host.name, '\0', sizeof(new.host.name));
    memset(new.host.port, '\0', sizeof(new.host.port));
    new.super_config = base_config_init();
    return new;
}

base_config base_config_init() {
    base_config new;
    new.pipe       = NULL;
    new.sock_host  = INVALID_SOCKET;
    new.is_chunked = INVALID_CHUNK;
    return new;
}

int GetHostName(ByteString *recvBuf, host_info_s *host_info) {
    StrList *result = FindSubStr(recvBuf->ch, recvBuf->len, "\r\n");
    if (result->length > 5) {
        for (int i = 0; i < result->length; ++i) {
            StrList *sub_res = FindSubStr(result->list[i], strlen(result->list[i]), "Host:");
            if (sub_res->length > 1) {
                StrList *find_port = FindSubStr(sub_res->list[1], strlen(sub_res->list[1]), ":");
                strcpy(host_info->name, StrStrim(find_port->list[0]));
                if (find_port->length > 1) {
                    strcpy(host_info->port, StrStrim(find_port->list[1]));
                } else {
                    strcpy(host_info->port, "80");
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


int check_base_config(base_config config) {
    if (config.sock_host == INVALID_SOCKET) {
        return 1;
    }
    return 0;
}

int check_http_header(base_config_t config) {
    StrList *by_content_length    = FindSubStr(config->pipe->ch, config->pipe->len, "Content-Length:");
    StrList *by_transfer_encoding = FindSubStr(config->pipe->ch, config->pipe->len, "Transfer-Encoding:");
    if (!(by_content_length->length == 1 ^ by_transfer_encoding->length == 1)) {
        FreeStrList(&by_content_length);
        FreeStrList(&by_transfer_encoding);
        return 1;
    }
    if (by_content_length->length > 1) {
        config->is_chunked = 0;
        goto Fin;
    }
    if (by_transfer_encoding->length > 1) {
        config->is_chunked = 1;
        goto Fin;
    }
Fin:
    FreeStrList(&by_content_length);
    FreeStrList(&by_transfer_encoding);
    return 0;
}

/* 返回ch字符在sign数组中的序号 */
int getIndexOfSigns(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    return -1;
}

int check_chunk_buffer(ByteString *http_buffer) {
    int chunk_buffer_len = 0;

    StrList *by_enter  = FindSubStr(http_buffer->ch, http_buffer->len, "\r\n");
    int     full_chunk = (int) by_enter->length - (int) by_enter->length % 2;

    for (int i = 0; i < full_chunk; i += 2) {
        chunk_buffer_len += getIndexOfSigns(by_enter->list[i][0]) + 2 * (int) strlen("\r\n");
    }
    FreeStrList(&by_enter);
    return chunk_buffer_len;
}

int get_header_length(ByteString *http_buffer) {
    StrList *by_double_enter = FindSubStr(http_buffer->ch, http_buffer->len, "\r\n\r\n");
    if (by_double_enter->length == 1) {
        FreeStrList(&by_double_enter);
        return -1;
    }
    return (int) (strlen(by_double_enter->list[0]) + strlen("\r\n\r\n"));
}

