//
// Created by michael on 2022/6/27.
//

#include "lib4str.h"
#include <stdio.h>
#include <malloc.h>
#include <ctype.h>


StrList *SubStrListNew(const char *origin);

void StrListAdd(StrList *, const char *, size_t);

StrList *SubStrListNew(const char *origin) {
    size_t  charSize   = ((origin) ? strlen(origin) : 0) + 1;
    size_t  mallocSize = charSize + sizeof(StrList);
    StrList *new       = (StrList *) calloc(mallocSize, BYTE_SIZE);

    if (!new) {
        printf("Malloc failed.");
        exit(-1);
    }
    if (origin) {
        strcpy(new->origin, origin);
    }
    return new;
}

void StrListAdd(StrList *str_list, const char *data, const size_t data_len) {
    char *new = calloc(data_len + 1, BYTE_SIZE);
    memcpy(new, data, data_len);
    if (str_list->list == NULL) {
        str_list->_msize = 1;
        str_list->list   = (char **) calloc(str_list->_msize, sizeof(char **));
    } else if (str_list->length >= str_list->_msize) {
        str_list->_msize = str_list->length * 2;
        str_list->list   = realloc(str_list->list, str_list->_msize * sizeof(char **));
    }
    str_list->list[str_list->length] = new;
    str_list->length++;
}

StrList *FindSubStr(const char *origin, const char *substr) {
    StrList *subStrChain = SubStrListNew(origin);
    size_t  ptr;
    char    *temp        = subStrChain->origin;
    while (temp != NULL) {
        char *remain = strstr(temp, substr);
        if (remain != NULL) {
            ptr = remain - temp;
            StrListAdd(subStrChain, temp, ptr);
            remain = remain + strlen(substr);
        } else {
            StrListAdd(subStrChain, temp, strlen(temp));
        }
        temp         = remain;
    }
    return subStrChain;
}

void FreeStrList(StrList **pList) {
    if (pList != NULL) {
        if ((*pList)->list != NULL) {
            char        **tempList = (*pList)->list;
            for (size_t i          = 0; i < (*pList)->length; i++) {
                free(tempList[i]);
                tempList[i] = NULL;
            }
            free(tempList);
            (*pList)->list = NULL;
        }
        free(*pList);
        *pList = NULL;
    }
}

char *StrStrim(char *str) {
    char   *end = str + strlen(str) - 1,
           *sp  = str,
           *ep  = end;
    size_t len;

    while (sp <= end && isspace(*sp))// *sp == ' '也可以
        sp++;
    while (ep >= sp && isspace(*ep))
        ep--;
    len = (ep < sp) ? 0 : (ep - sp) + 1;//(ep < sp)判断是否整行都是空格
    sp[len] = '\0';
    return sp;
}

ByteString *ByteStringNew(const char *data, size_t _len) {
    size_t     dataSize = ((data) ? _len : 0) + 1;
    ByteString *pString = calloc(dataSize + sizeof(ByteString), BYTE_SIZE);
    if (pString == NULL) {
        return NULL;
    }
    pString->_msize = dataSize + sizeof(ByteString);
    if (data) {
        memcpy(pString->ch, data, _len);
        pString->len = strlen(pString->ch);
    }
    return pString;
}

ERROR_CODE ByteStringAdd(ByteString **_dst, const char *_src, size_t _len) {
    if (_dst == NULL) {
        printf("pointer _dst can't be NULL");
        return ByteListError;
    }
    if (*_dst == NULL) {
        *_dst = ByteStringNew(_src, _len);
        if (*_dst == NULL) {
            printf("ByteStringNew malloc Error");
            return ByteListError;
        }
    } else {
        size_t remain = (*_dst)->_msize - (*_dst)->len - 1;
        if (remain >= _len) {
//            memcpy((*_dst)->ch + (*_dst)->len, _src, _len);
        } else {
            size_t     dataSize = _len + (*_dst)->len;
            size_t     msize    = dataSize * 2 + 1;
            ByteString *pString = calloc(msize + sizeof(ByteString), BYTE_SIZE);
            if (pString == NULL) {
                return ByteListError;
            }
            memcpy(pString, *_dst, (*_dst)->_msize + sizeof(ByteString));
            free(*_dst);
            *_dst = pString;
            (*_dst)->_msize = msize;
        }
        memcpy((*_dst)->ch + (*_dst)->len, _src, _len);
        (*_dst)->len = (*_dst)->len + _len;
    }
    return 0;
}


void FreeByteString(ByteString **byte_list) {
    if (byte_list != NULL) {
        free(*byte_list);
        *byte_list = NULL;
    }
}

//char *ByteStrPopFront(ByteString *byte_str, size_t len) {
//    char *out_pipe = calloc(len, BYTE_SIZE);
//    if (out_pipe == NULL) {
//        printf("Malloc Size ERROR\n");
//        return NULL;
//    }
//    if (len >= byte_str->len) {
//        memcpy(out_pipe, byte_str->ch, byte_str->len);
//
//    }
//
//}
