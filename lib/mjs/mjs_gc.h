/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_GC_H_
#define MJS_GC_H_

#include "mjs_core.h"
#include "mjs_mm.h"
#include "mjs_internal.h"
#include "mjs_gc_public.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*
 * performs arithmetics on gc_cell pointers as if they were arena->cell_size
 * bytes wide
 */
#define GC_CELL_OP(arena, cell, op, arg) \
    ((struct gc_cell*)(((char*)(cell))op((arg) * (arena)->cell_size)))

struct gc_cell {
    union {
        struct gc_cell* link;
        uintptr_t word;
    } head;
};

MJS_PRIVATE int gc_strings_is_gc_needed(struct mjs* mjs);

/* perform gc if not inhibited */
MJS_PRIVATE int maybe_gc(struct mjs* mjs);

MJS_PRIVATE struct mjs_object* new_object(struct mjs*);
MJS_PRIVATE struct mjs_property* new_property(struct mjs*);
MJS_PRIVATE struct mjs_ffi_sig* new_ffi_sig(struct mjs* mjs);

MJS_PRIVATE void gc_mark(struct mjs* mjs, mjs_val_t* val);

MJS_PRIVATE void gc_arena_init(struct gc_arena*, size_t, size_t, size_t);
MJS_PRIVATE void gc_arena_destroy(struct mjs*, struct gc_arena* a);
MJS_PRIVATE void gc_sweep(struct mjs*, struct gc_arena*, size_t);
MJS_PRIVATE void* gc_alloc_cell(struct mjs*, struct gc_arena*);

MJS_PRIVATE uint64_t gc_string_mjs_val_to_offset(mjs_val_t v);

/* return 0 if v is an object/function with a bad pointer */
MJS_PRIVATE int gc_check_val(struct mjs* mjs, mjs_val_t v);

/* checks whether a pointer is within the ranges of an arena */
MJS_PRIVATE int gc_check_ptr(const struct gc_arena* a, const void* p);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_GC_H_ */
