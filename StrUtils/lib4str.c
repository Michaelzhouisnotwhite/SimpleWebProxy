//
// Created by michael on 2022/6/27.
//

#include <stdio.h>
#include "lib4str.h"

SubStrChain *SubStrChainNew(const char *origin);

void ChainAdd(SubStrChain *, const char *);

SubStr *SubStrNew(const char *data);

SubStrChain *SubStrChainNew(const char *origin) {
    size_t charSize = ((origin) ? strlen(origin) : 0) + 1;
    size_t mallocSize = charSize + sizeof(SubStrChain);
    SubStrChain *new = (SubStrChain *) calloc(mallocSize, BYTE_SIZE);

    if (!new) {
        printf("Malloc failed.");
        exit(-1);
    }
    if (origin) {
        strcpy(new->origin, origin);
    }
    return new;
}

SubStr *SubStrNew(const char *data) {
    size_t charSize = ((data) ? strlen(data) : 0) + 1;
    size_t mallocSize = charSize + sizeof(SubStr);
    SubStr *new = (SubStr *) calloc(mallocSize, BYTE_SIZE);

    if (!new) {
        printf("Malloc failed.");
        exit(-1);
    }
    if (data) {
        strcpy(new->substr, data);
    }
    return new;
}

void ChainAdd(SubStrChain *chain, const char *data) {
    SubStr *substr = SubStrNew(data);
    if (!chain->front) {
        chain->front = substr;
        chain->back = substr;
    } else {
        chain->back->next = substr;
    }
}

SubStrChain *FindSubStr(const char *origin, const char *substr) {
    SubStrChain *subStrChain = SubStrChainNew(origin);
    INT_PTR ptr = (long long int) origin;
    char *temp = subStrChain->origin;
    while (temp != NULL) {
        // TODO: continue find strstr
        char *remain = strstr(temp, substr);
        if (remain != NULL) {
            ptr = remain - origin;
            char *buf = calloc(ptr, BYTE_SIZE);
            memcpy(buf,)
            free(buf);
        }
    }
    return NULL;
}
