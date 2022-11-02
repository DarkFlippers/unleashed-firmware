#include "memset_s.h"

#define RSIZE_MAX 0x7fffffffffffffffUL

errno_t memset_s(void* s, rsize_t smax, int c, rsize_t n) {
    if(!s || smax > RSIZE_MAX) {
        return EINVAL;
    }

    errno_t violation_present = 0;
    if(n > smax) {
        n = smax;
        violation_present = EINVAL;
    }

    volatile unsigned char* v = s;
    for(rsize_t i = 0u; i < n; ++i) {
        *v++ = (unsigned char)c;
    }

    return violation_present;
}