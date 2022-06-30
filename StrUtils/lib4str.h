//
// Created by michael on 2022/6/27.
//

#ifndef SIMPLEWEBPROXY_LIB4STR_H
#define SIMPLEWEBPROXY_LIB4STR_H

#include <string.h>
#include <stdlib.h>
#include <string.h>

#define BYTE_SIZE sizeof(char)
#define ERROR_CODE int
#define ByteListError (-1)

typedef struct ByteString {
    size_t len;
    size_t _msize;
    char   ch[];
} ByteString, *byte_string_t;

typedef struct StrList {
    char   **list;
    size_t _msize;
    size_t length;
    char   origin[];
} StrList;



ERROR_CODE ByteStringAdd(ByteString **_dst, const char *_src, size_t _len);

void FreeByteString(ByteString **byte_list);

StrList *FindSubStr(const char *origin, size_t origin_len, const char *substr);

void FreeStrList(StrList **pList);

char *StrStrim(char *str);

int ByteStrPopFront(ByteString *byte_str, size_t len);

#endif //SIMPLEWEBPROXY_LIB4STR_H
