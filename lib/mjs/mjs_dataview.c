/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "mjs_exec_public.h"
#include "mjs_internal.h"
#include "mjs_object.h"
#include "mjs_primitive.h"
#include "mjs_util.h"

void* mjs_mem_to_ptr(unsigned val) {
    return (void*)(uintptr_t)val;
}

void* mjs_mem_get_ptr(void* base, int offset) {
    return (char*)base + offset;
}

void mjs_mem_set_ptr(void* ptr, void* val) {
    *(void**)ptr = val;
}

double mjs_mem_get_dbl(void* ptr) {
    double v;
    memcpy(&v, ptr, sizeof(v));
    return v;
}

void mjs_mem_set_dbl(void* ptr, double val) {
    memcpy(ptr, &val, sizeof(val));
}

/*
 * TODO(dfrank): add support for unsigned ints to ffi and use
 * unsigned int here
 */
double mjs_mem_get_uint(void* ptr, int size, int bigendian) {
    uint8_t* p = (uint8_t*)ptr;
    int i, inc = bigendian ? 1 : -1;
    unsigned int res = 0;
    p += bigendian ? 0 : size - 1;
    for(i = 0; i < size; i++, p += inc) {
        res <<= 8;
        res |= *p;
    }
    return res;
}

/*
 * TODO(dfrank): add support for unsigned ints to ffi and use
 * unsigned int here
 */
double mjs_mem_get_int(void* ptr, int size, int bigendian) {
    uint8_t* p = (uint8_t*)ptr;
    int i, inc = bigendian ? 1 : -1;
    int res = 0;
    p += bigendian ? 0 : size - 1;

    for(i = 0; i < size; i++, p += inc) {
        res <<= 8;
        res |= *p;
    }

    /* sign-extend */
    {
        int extra = sizeof(res) - size;
        for(i = 0; i < extra; i++) res <<= 8;
        for(i = 0; i < extra; i++) res >>= 8;
    }

    return res;
}

void mjs_mem_set_uint(void* ptr, unsigned int val, int size, int bigendian) {
    uint8_t* p = (uint8_t*)ptr + (bigendian ? size - 1 : 0);
    int i, inc = bigendian ? -1 : 1;
    for(i = 0; i < size; i++, p += inc) {
        *p = val & 0xff;
        val >>= 8;
    }
}

void mjs_mem_set_int(void* ptr, int val, int size, int bigendian) {
    mjs_mem_set_uint(ptr, val, size, bigendian);
}
