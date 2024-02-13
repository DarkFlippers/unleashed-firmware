/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_BCODE_H_
#define MJS_BCODE_H_

#include "mjs_internal.h"

#include "mjs_core.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

enum mjs_opcode {
    OP_NOP, /* ( -- ) */
    OP_DROP, /* ( a -- ) */
    OP_DUP, /* ( a -- a a ) */
    OP_SWAP, /* ( a b -- b a ) */
    OP_JMP, /* ( -- ) */
    OP_JMP_TRUE, /* ( -- ) */
    OP_JMP_NEUTRAL_TRUE, /* ( -- ) */
    OP_JMP_FALSE, /* ( -- ) */
    OP_JMP_NEUTRAL_FALSE, /* ( -- ) */
    OP_FIND_SCOPE, /* ( a -- a b ) */
    OP_PUSH_SCOPE, /* ( -- a ) */
    OP_PUSH_STR, /* ( -- a ) */
    OP_PUSH_TRUE, /* ( -- a ) */
    OP_PUSH_FALSE, /* ( -- a ) */
    OP_PUSH_INT, /* ( -- a ) */
    OP_PUSH_DBL, /* ( -- a ) */
    OP_PUSH_NULL, /* ( -- a ) */
    OP_PUSH_UNDEF, /* ( -- a ) */
    OP_PUSH_OBJ, /* ( -- a ) */
    OP_PUSH_ARRAY, /* ( -- a ) */
    OP_PUSH_FUNC, /* ( -- a ) */
    OP_PUSH_THIS, /* ( -- a ) */
    OP_GET, /* ( key obj  -- obj[key] ) */
    OP_CREATE, /* ( key obj -- ) */
    OP_EXPR, /* ( ... -- a ) */
    OP_APPEND, /* ( a b -- ) */
    OP_SET_ARG, /* ( a -- a ) */
    OP_NEW_SCOPE, /* ( -- ) */
    OP_DEL_SCOPE, /* ( -- ) */
    OP_CALL, /* ( func param1 param2 ... num_params -- result ) */
    OP_RETURN, /* ( -- ) */
    OP_LOOP, /* ( -- ) Push break & continue addresses to loop_labels */
    OP_BREAK, /* ( -- ) */
    OP_CONTINUE, /* ( -- ) */
    OP_SETRETVAL, /* ( a -- ) */
    OP_EXIT, /* ( -- ) */
    OP_BCODE_HEADER, /* ( -- ) */
    OP_ARGS, /* ( -- ) Mark the beginning of function call arguments */
    OP_FOR_IN_NEXT, /* ( name obj iter_ptr -- name obj iter_ptr_next ) */
    OP_MAX
};

struct pstate;
struct mjs;

MJS_PRIVATE void emit_byte(struct pstate* pstate, uint8_t byte);
MJS_PRIVATE void emit_int(struct pstate* pstate, int64_t n);
MJS_PRIVATE void emit_str(struct pstate* pstate, const char* ptr, size_t len);

/*
 * Inserts provided offset `v` at the offset `offset`.
 *
 * Returns delta at which the code was moved; the delta can be any: 0 or
 * positive or negative.
 */
MJS_PRIVATE int
    mjs_bcode_insert_offset(struct pstate* p, struct mjs* mjs, size_t offset, size_t v);

/*
 * Adds a new bcode part; does not retain `bp`.
 */
MJS_PRIVATE void mjs_bcode_part_add(struct mjs* mjs, const struct mjs_bcode_part* bp);

/*
 * Returns bcode part by the bcode number
 */
MJS_PRIVATE struct mjs_bcode_part* mjs_bcode_part_get(struct mjs* mjs, int num);

/*
 * Returns bcode part by the global bcode offset
 */
MJS_PRIVATE struct mjs_bcode_part* mjs_bcode_part_get_by_offset(struct mjs* mjs, size_t offset);

/*
 * Returns a number of bcode parts
 */
MJS_PRIVATE int mjs_bcode_parts_cnt(struct mjs* mjs);

/*
 * Adds the bcode being generated (mjs->bcode_gen) as a next bcode part
 */
MJS_PRIVATE void mjs_bcode_commit(struct mjs* mjs);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_BCODE_H_ */
