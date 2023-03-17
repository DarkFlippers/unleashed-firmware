#include "strnlen.h"

size_t strnlen(const char* s, size_t maxlen) {
    size_t len;

    for(len = 0; len < maxlen; len++, s++) {
        if(!*s) break;
    }

    return len;
}