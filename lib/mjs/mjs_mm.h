/*
 * Copyright (c) 2014-2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_MM_H_
#define MJS_MM_H_

#include "mjs_internal.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

struct mjs;

typedef void (*gc_cell_destructor_t)(struct mjs* mjs, void*);

struct gc_block {
    struct gc_block* next;
    struct gc_cell* base;
    size_t size;
};

struct gc_arena {
    struct gc_block* blocks;
    size_t size_increment;
    struct gc_cell* free; /* head of free list */
    size_t cell_size;

#if MJS_MEMORY_STATS
    unsigned long allocations; /* cumulative counter of allocations */
    unsigned long garbage; /* cumulative counter of garbage */
    unsigned long alive; /* number of living cells */
#endif

    gc_cell_destructor_t destructor;
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_MM_H_ */
