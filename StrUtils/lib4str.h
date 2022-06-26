//
// Created by michael on 2022/6/27.
//

#ifndef SIMPLEWEBPROXY_LIB4STR_H
#define SIMPLEWEBPROXY_LIB4STR_H

#include <string.h>
#include <stdlib.h>

#define BYTE char
#define BYTE_SIZE sizeof(char)
#define INT_PTR long long

typedef struct SubStr {
    struct SubStr *next;
    char substr[];
} SubStr;

typedef struct SubStrChain {
    SubStr *front;
    SubStr *back;
    int nums;
    char origin[];
} SubStrChain;

SubStrChain *FindSubStr(const char *origin, const char *substr);

#endif //SIMPLEWEBPROXY_LIB4STR_H
