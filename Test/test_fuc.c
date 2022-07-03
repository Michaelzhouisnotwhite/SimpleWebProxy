//
// Created by michael on 2022/7/1.
//

#include "test_fuc.h"

void test_server() {
    SocketInit();
    const char *port = "7001";
    ServerStart(port);
    WSACleanup();
}

void test_proxy_event_list() {
    proxy_event_list_s event_list = proxy_event_list_init(3000);
    proxy_event_create(&event_list, 1, 1);
    proxy_event_create(&event_list, 2, 1);
    proxy_event_create(&event_list, 3, 1);
    proxy_event_create(&event_list, 4, 1);

//    proxy_event_at(&event_list, 1)->is_closed = 1;
    proxy_event_at(&event_list, 0)->is_closed = 1;
    proxy_event_at(&event_list, 2)->is_closed = 1;

    clear_closed_proxy_events(&event_list);

    proxy_event_at(&event_list, 0)->is_closed = 1;
    clear_closed_proxy_events(&event_list);
}

void test_strstr() {
    char test_char[] = "httpadkf\r\nasdkljjk\r\nContent-Length: 1024\r\nal;dkfj\r\n\r\n";
    char* remain = strstr(test_char, "Content-Length:");
    char *remain2 = strstr(remain, "\r\n");
    size_t value_len = remain2 - remain;
    char value[101] = {0};
    strncpy(value, remain + strlen("Content-Length:"), value_len - strlen("Content-Length:"));
    value[value_len - strlen("Content-Length:")] = '\0';
    int res = strtol(" 1024", NULL, 10);
}