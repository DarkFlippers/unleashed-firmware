/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>

#include "common/cs_varint.h"
#include "common/mbuf.h"

#include "mjs_core.h"
#include "mjs_ffi.h"
#include "mjs_gc.h"
#include "mjs_internal.h"
#include "mjs_object.h"
#include "mjs_primitive.h"
#include "mjs_string.h"

/*
 * Macros for marking reachable things: use bit 0.
 */
#define MARK(p) (((struct gc_cell*)(p))->head.word |= 1)
#define UNMARK(p) (((struct gc_cell*)(p))->head.word &= ~1)
#define MARKED(p) (((struct gc_cell*)(p))->head.word & 1)

/*
 * Similar to `MARK()` / `UNMARK()` / `MARKED()`, but `.._FREE` counterparts
 * are intended to mark free cells (as opposed to used ones), so they use
 * bit 1.
 */
#define MARK_FREE(p) (((struct gc_cell*)(p))->head.word |= 2)
#define UNMARK_FREE(p) (((struct gc_cell*)(p))->head.word &= ~2)
#define MARKED_FREE(p) (((struct gc_cell*)(p))->head.word & 2)

/*
 * When each arena has that or less free cells, GC will be scheduled
 */
#define GC_ARENA_CELLS_RESERVE 2

static struct gc_block* gc_new_block(struct gc_arena* a, size_t size);
static void gc_free_block(struct gc_block* b);
static void gc_mark_mbuf_pt(struct mjs* mjs, const struct mbuf* mbuf);

MJS_PRIVATE struct mjs_object* new_object(struct mjs* mjs) {
    return (struct mjs_object*)gc_alloc_cell(mjs, &mjs->object_arena);
}

MJS_PRIVATE struct mjs_property* new_property(struct mjs* mjs) {
    return (struct mjs_property*)gc_alloc_cell(mjs, &mjs->property_arena);
}

MJS_PRIVATE struct mjs_ffi_sig* new_ffi_sig(struct mjs* mjs) {
    return (struct mjs_ffi_sig*)gc_alloc_cell(mjs, &mjs->ffi_sig_arena);
}

/* Initializes a new arena. */
MJS_PRIVATE void gc_arena_init(
    struct gc_arena* a,
    size_t cell_size,
    size_t initial_size,
    size_t size_increment) {
    assert(cell_size >= sizeof(uintptr_t));

    memset(a, 0, sizeof(*a));
    a->cell_size = cell_size;
    a->size_increment = size_increment;
    a->blocks = gc_new_block(a, initial_size);
}

MJS_PRIVATE void gc_arena_destroy(struct mjs* mjs, struct gc_arena* a) {
    struct gc_block* b;

    if(a->blocks != NULL) {
        gc_sweep(mjs, a, 0);
        for(b = a->blocks; b != NULL;) {
            struct gc_block* tmp;
            tmp = b;
            b = b->next;
            gc_free_block(tmp);
        }
    }
}

static void gc_free_block(struct gc_block* b) {
    free(b->base);
    free(b);
}

static struct gc_block* gc_new_block(struct gc_arena* a, size_t size) {
    struct gc_cell* cur;
    struct gc_block* b;

    b = (struct gc_block*)calloc(1, sizeof(*b));
    if(b == NULL) abort();

    b->size = size;
    b->base = (struct gc_cell*)calloc(a->cell_size, b->size);
    if(b->base == NULL) abort();

    for(cur = GC_CELL_OP(a, b->base, +, 0); cur < GC_CELL_OP(a, b->base, +, b->size);
        cur = GC_CELL_OP(a, cur, +, 1)) {
        cur->head.link = a->free;
        a->free = cur;
    }

    return b;
}

/*
 * Returns whether the given arena has GC_ARENA_CELLS_RESERVE or less free
 * cells
 */
static int gc_arena_is_gc_needed(struct gc_arena* a) {
    struct gc_cell* r = a->free;
    int i;

    for(i = 0; i <= GC_ARENA_CELLS_RESERVE; i++, r = r->head.link) {
        if(r == NULL) {
            return 1;
        }
    }

    return 0;
}

MJS_PRIVATE int gc_strings_is_gc_needed(struct mjs* mjs) {
    struct mbuf* m = &mjs->owned_strings;
    return (double)m->len / (double)m->size > (double)0.9;
}

MJS_PRIVATE void* gc_alloc_cell(struct mjs* mjs, struct gc_arena* a) {
    struct gc_cell* r;

    if(a->free == NULL) {
        struct gc_block* b = gc_new_block(a, a->size_increment);
        b->next = a->blocks;
        a->blocks = b;
    }
    r = a->free;

    UNMARK(r);

    a->free = r->head.link;

#if MJS_MEMORY_STATS
    a->allocations++;
    a->alive++;
#endif

    /* Schedule GC if needed */
    if(gc_arena_is_gc_needed(a)) {
        mjs->need_gc = 1;
    }

    /*
   * TODO(mkm): minor opt possible since most of the fields
   * are overwritten downstream, but not worth the yak shave time
   * when fields are added to GC-able structures */
    memset(r, 0, a->cell_size);
    return (void*)r;
}

/*
 * Scans the arena and add all unmarked cells to the free list.
 *
 * Empty blocks get deallocated. The head of the free list will contais cells
 * from the last (oldest) block. Cells will thus be allocated in block order.
 */
void gc_sweep(struct mjs* mjs, struct gc_arena* a, size_t start) {
    struct gc_block* b;
    struct gc_cell* cur;
    struct gc_block** prevp = &a->blocks;
#if MJS_MEMORY_STATS
    a->alive = 0;
#endif

    /*
   * Before we sweep, we should mark all free cells in a way that is
   * distinguishable from marked used cells.
   */
    {
        struct gc_cell* next;
        for(cur = a->free; cur != NULL; cur = next) {
            next = cur->head.link;
            MARK_FREE(cur);
        }
    }

    /*
   * We'll rebuild the whole `free` list, so initially we just reset it
   */
    a->free = NULL;

    for(b = a->blocks; b != NULL;) {
        size_t freed_in_block = 0;
        /*
     * if it turns out that this block is 100% garbage
     * we can release the whole block, but the addition
     * of it's cells to the free list has to be undone.
     */
        struct gc_cell* prev_free = a->free;

        for(cur = GC_CELL_OP(a, b->base, +, start); cur < GC_CELL_OP(a, b->base, +, b->size);
            cur = GC_CELL_OP(a, cur, +, 1)) {
            if(MARKED(cur)) {
                /* The cell is used and marked  */
                UNMARK(cur);
#if MJS_MEMORY_STATS
                a->alive++;
#endif
            } else {
                /*
         * The cell is either:
         * - free
         * - garbage that's about to be freed
         */

                if(MARKED_FREE(cur)) {
                    /* The cell is free, so, just unmark it */
                    UNMARK_FREE(cur);
                } else {
                    /*
           * The cell is used and should be freed: call the destructor and
           * reset the memory
           */
                    if(a->destructor != NULL) {
                        a->destructor(mjs, cur);
                    }
                    memset(cur, 0, a->cell_size);
                }

                /* Add this cell to the `free` list */
                cur->head.link = a->free;
                a->free = cur;
                freed_in_block++;
#if MJS_MEMORY_STATS
                a->garbage++;
#endif
            }
        }

        /*
     * don't free the initial block, which is at the tail
     * because it has a special size aimed at reducing waste
     * and simplifying initial startup. TODO(mkm): improve
     * */
        if(b->next != NULL && freed_in_block == b->size) {
            *prevp = b->next;
            gc_free_block(b);
            b = *prevp;
            a->free = prev_free;
        } else {
            prevp = &b->next;
            b = b->next;
        }
    }
}

/* Mark an FFI signature */
static void gc_mark_ffi_sig(struct mjs* mjs, mjs_val_t* v) {
    struct mjs_ffi_sig* psig;

    assert(mjs_is_ffi_sig(*v));

    psig = mjs_get_ffi_sig_struct(*v);

    /*
   * we treat all object like things like objects but they might be functions,
   * gc_check_val checks the appropriate arena per actual value type.
   */
    if(!gc_check_val(mjs, *v)) {
        abort();
    }

    if(MARKED(psig)) return;

    MARK(psig);
}

/* Mark an object */
static void gc_mark_object(struct mjs* mjs, mjs_val_t* v) {
    struct mjs_object* obj_base;
    struct mjs_property* prop;
    struct mjs_property* next;

    assert(mjs_is_object_based(*v));

    obj_base = get_object_struct(*v);

    /*
   * we treat all object like things like objects but they might be functions,
   * gc_check_val checks the appropriate arena per actual value type.
   */
    if(!gc_check_val(mjs, *v)) {
        abort();
    }

    if(MARKED(obj_base)) return;

    /* mark object itself, and its properties */
    for((prop = obj_base->properties), MARK(obj_base); prop != NULL; prop = next) {
        if(!gc_check_ptr(&mjs->property_arena, prop)) {
            abort();
        }

        gc_mark(mjs, &prop->name);
        gc_mark(mjs, &prop->value);

        next = prop->next;
        MARK(prop);
    }

    /* mark object's prototype */
    /*
   * We dropped support for object prototypes in MJS.
   * If we ever bring it back, don't forget to mark it
   */
    /* gc_mark(mjs, mjs_get_proto(mjs, v)); */
}

/* Mark a string value */
static void gc_mark_string(struct mjs* mjs, mjs_val_t* v) {
    mjs_val_t h, tmp = 0;
    char* s;

    /* clang-format off */

  /*
   * If a value points to an unmarked string we shall:
   *  1. save the first 6 bytes of the string
   *     since we need to be able to distinguish real values from
   *     the saved first 6 bytes of the string, we need to tag the chunk
   *     as MJS_TAG_STRING_C
   *  2. encode value's address (v) into the first 6 bytes of the string.
   *  3. put the saved 8 bytes (tag + chunk) back into the value.
   *  4. mark the string by putting '\1' in the NUL terminator of the previous
   *     string chunk.
   *
   * If a value points to an already marked string we shall:
   *     (0, <6 bytes of a pointer to a mjs_val_t>), hence we have to skip
   *     the first byte. We tag the value pointer as a MJS_TAG_FOREIGN
   *     so that it won't be followed during recursive mark.
   *
   *  ... the rest is the same
   *
   *  Note: 64-bit pointers can be represented with 48-bits
   */

    /* clang-format on */

    assert((*v & MJS_TAG_MASK) == MJS_TAG_STRING_O);

    s = mjs->owned_strings.buf + gc_string_mjs_val_to_offset(*v);
    assert(s < mjs->owned_strings.buf + mjs->owned_strings.len);
    if(s[-1] == '\0') {
        memcpy(&tmp, s, sizeof(tmp) - 2);
        tmp |= MJS_TAG_STRING_C;
    } else {
        memcpy(&tmp, s, sizeof(tmp) - 2);
        tmp |= MJS_TAG_FOREIGN;
    }

    h = (mjs_val_t)(uintptr_t)v;
    s[-1] = 1;
    memcpy(s, &h, sizeof(h) - 2);
    memcpy(v, &tmp, sizeof(tmp));
}

MJS_PRIVATE void gc_mark(struct mjs* mjs, mjs_val_t* v) {
    if(mjs_is_object_based(*v)) {
        gc_mark_object(mjs, v);
    }
    if(mjs_is_ffi_sig(*v)) {
        gc_mark_ffi_sig(mjs, v);
    }
    if((*v & MJS_TAG_MASK) == MJS_TAG_STRING_O) {
        gc_mark_string(mjs, v);
    }
}

MJS_PRIVATE uint64_t gc_string_mjs_val_to_offset(mjs_val_t v) {
    return (((uint64_t)(uintptr_t)get_ptr(v)) & ~MJS_TAG_MASK);
}

MJS_PRIVATE mjs_val_t gc_string_val_from_offset(uint64_t s) {
    return s | MJS_TAG_STRING_O;
}

void gc_compact_strings(struct mjs* mjs) {
    char* p = mjs->owned_strings.buf + 1;
    uint64_t h, next, head = 1;
    int len, llen;

    while(p < mjs->owned_strings.buf + mjs->owned_strings.len) {
        if(p[-1] == '\1') {
            /* relocate and update ptrs */
            h = 0;
            memcpy(&h, p, sizeof(h) - 2);

            /*
       * relocate pointers until we find the tail.
       * The tail is marked with MJS_TAG_STRING_C,
       * while mjs_val_t link pointers are tagged with MJS_TAG_FOREIGN
       */
            for(; (h & MJS_TAG_MASK) != MJS_TAG_STRING_C; h = next) {
                h &= ~MJS_TAG_MASK;
                memcpy(&next, (char*)(uintptr_t)h, sizeof(h));

                *(mjs_val_t*)(uintptr_t)h = gc_string_val_from_offset(head);
            }
            h &= ~MJS_TAG_MASK;

            /*
       * the tail contains the first 6 bytes we stole from
       * the actual string.
       */
            len = cs_varint_decode_unsafe((unsigned char*)&h, &llen);
            len += llen + 1;

            /*
       * restore the saved 6 bytes
       * TODO(mkm): think about endianness
       */
            memcpy(p, &h, sizeof(h) - 2);

            /*
       * and relocate the string data by packing it to the left.
       */
            memmove(mjs->owned_strings.buf + head, p, len);
            mjs->owned_strings.buf[head - 1] = 0x0;
            p += len;
            head += len;
        } else {
            len = cs_varint_decode_unsafe((unsigned char*)p, &llen);
            len += llen + 1;

            p += len;
        }
    }

    mjs->owned_strings.len = head;
}

MJS_PRIVATE int maybe_gc(struct mjs* mjs) {
    if(!mjs->inhibit_gc) {
        mjs_gc(mjs, 0);
        return 1;
    }
    return 0;
}

/*
 * mark an array of `mjs_val_t` values (*not pointers* to them)
 */
static void gc_mark_val_array(struct mjs* mjs, mjs_val_t* vals, size_t len) {
    mjs_val_t* vp;
    for(vp = vals; vp < vals + len; vp++) {
        gc_mark(mjs, vp);
    }
}

/*
 * mark an mbuf containing *pointers* to `mjs_val_t` values
 */
static void gc_mark_mbuf_pt(struct mjs* mjs, const struct mbuf* mbuf) {
    mjs_val_t** vp;
    for(vp = (mjs_val_t**)mbuf->buf; (char*)vp < mbuf->buf + mbuf->len; vp++) {
        gc_mark(mjs, *vp);
    }
}

/*
 * mark an mbuf containing `mjs_val_t` values (*not pointers* to them)
 */
static void gc_mark_mbuf_val(struct mjs* mjs, const struct mbuf* mbuf) {
    gc_mark_val_array(mjs, (mjs_val_t*)mbuf->buf, mbuf->len / sizeof(mjs_val_t));
}

static void gc_mark_ffi_cbargs_list(struct mjs* mjs, ffi_cb_args_t* cbargs) {
    for(; cbargs != NULL; cbargs = cbargs->next) {
        gc_mark(mjs, &cbargs->func);
        gc_mark(mjs, &cbargs->userdata);
    }
}

/* Perform garbage collection */
void mjs_gc(struct mjs* mjs, int full) {
    gc_mark_val_array(mjs, (mjs_val_t*)&mjs->vals, sizeof(mjs->vals) / sizeof(mjs_val_t));

    gc_mark_mbuf_pt(mjs, &mjs->owned_values);
    gc_mark_mbuf_val(mjs, &mjs->scopes);
    gc_mark_mbuf_val(mjs, &mjs->stack);
    gc_mark_mbuf_val(mjs, &mjs->call_stack);

    gc_mark_ffi_cbargs_list(mjs, mjs->ffi_cb_args);

    gc_compact_strings(mjs);

    gc_sweep(mjs, &mjs->object_arena, 0);
    gc_sweep(mjs, &mjs->property_arena, 0);
    gc_sweep(mjs, &mjs->ffi_sig_arena, 0);

    if(full) {
        /*
     * In case of full GC, we also resize strings buffer, but we still leave
     * some extra space (at most, `_MJS_STRING_BUF_RESERVE`) in order to avoid
     * frequent reallocations
     */
        size_t trimmed_size = mjs->owned_strings.len + _MJS_STRING_BUF_RESERVE;
        if(trimmed_size < mjs->owned_strings.size) {
            mbuf_resize(&mjs->owned_strings, trimmed_size);
        }
    }
}

MJS_PRIVATE int gc_check_val(struct mjs* mjs, mjs_val_t v) {
    if(mjs_is_object_based(v)) {
        return gc_check_ptr(&mjs->object_arena, get_object_struct(v));
    }
    if(mjs_is_ffi_sig(v)) {
        return gc_check_ptr(&mjs->ffi_sig_arena, mjs_get_ffi_sig_struct(v));
    }
    return 1;
}

MJS_PRIVATE int gc_check_ptr(const struct gc_arena* a, const void* ptr) {
    const struct gc_cell* p = (const struct gc_cell*)ptr;
    struct gc_block* b;
    for(b = a->blocks; b != NULL; b = b->next) {
        if(p >= b->base && p < GC_CELL_OP(a, b->base, +, b->size)) {
            return 1;
        }
    }
    return 0;
}
