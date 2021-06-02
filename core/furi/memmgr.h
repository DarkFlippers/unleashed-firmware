#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"

#ifdef __cplusplus
extern "C" {
#endif

// define for test case "link against furi memmgr"
#define FURI_MEMMGR_GUARD 1

size_t memmgr_get_free_heap(void);
size_t memmgr_get_minimum_free_heap(void);

inline static void* furi_alloc(size_t size) {
    void* p = malloc(size);
    furi_check(p);
    return memset(p, 0, size);
}

#ifdef __cplusplus
}
#endif
