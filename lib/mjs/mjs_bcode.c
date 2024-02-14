/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "common/cs_varint.h"

#include "mjs_internal.h"
#include "mjs_bcode.h"
#include "mjs_core.h"
#include "mjs_tok.h"

static void add_lineno_map_item(struct pstate* pstate) {
    if(pstate->last_emitted_line_no < pstate->line_no) {
        int offset = pstate->cur_idx - pstate->start_bcode_idx;
        size_t offset_llen = cs_varint_llen(offset);
        size_t lineno_llen = cs_varint_llen(pstate->line_no);
        mbuf_resize(
            &pstate->offset_lineno_map,
            pstate->offset_lineno_map.size + offset_llen + lineno_llen);

        /* put offset */
        cs_varint_encode(
            offset,
            (uint8_t*)pstate->offset_lineno_map.buf + pstate->offset_lineno_map.len,
            offset_llen);
        pstate->offset_lineno_map.len += offset_llen;

        /* put line_no */
        cs_varint_encode(
            pstate->line_no,
            (uint8_t*)pstate->offset_lineno_map.buf + pstate->offset_lineno_map.len,
            lineno_llen);
        pstate->offset_lineno_map.len += lineno_llen;

        pstate->last_emitted_line_no = pstate->line_no;
    }
}

MJS_PRIVATE void emit_byte(struct pstate* pstate, uint8_t byte) {
    add_lineno_map_item(pstate);
    mbuf_insert(&pstate->mjs->bcode_gen, pstate->cur_idx, &byte, sizeof(byte));
    pstate->cur_idx += sizeof(byte);
}

MJS_PRIVATE void emit_int(struct pstate* pstate, int64_t n) {
    struct mbuf* b = &pstate->mjs->bcode_gen;
    size_t llen = cs_varint_llen(n);
    add_lineno_map_item(pstate);
    mbuf_insert(b, pstate->cur_idx, NULL, llen);
    cs_varint_encode(n, (uint8_t*)b->buf + pstate->cur_idx, llen);
    pstate->cur_idx += llen;
}

MJS_PRIVATE void emit_str(struct pstate* pstate, const char* ptr, size_t len) {
    struct mbuf* b = &pstate->mjs->bcode_gen;
    size_t llen = cs_varint_llen(len);
    add_lineno_map_item(pstate);
    mbuf_insert(b, pstate->cur_idx, NULL, llen + len);
    cs_varint_encode(len, (uint8_t*)b->buf + pstate->cur_idx, llen);
    memcpy(b->buf + pstate->cur_idx + llen, ptr, len);
    pstate->cur_idx += llen + len;
}

MJS_PRIVATE int
    mjs_bcode_insert_offset(struct pstate* p, struct mjs* mjs, size_t offset, size_t v) {
    int llen = (int)cs_varint_llen(v);
    int diff = llen - MJS_INIT_OFFSET_SIZE;
    assert(offset < mjs->bcode_gen.len);
    if(diff > 0) {
        mbuf_resize(&mjs->bcode_gen, mjs->bcode_gen.size + diff);
    }
    /*
   * Offset is going to take more than one was reserved, so, move the data
   * forward
   */
    memmove(
        mjs->bcode_gen.buf + offset + llen,
        mjs->bcode_gen.buf + offset + MJS_INIT_OFFSET_SIZE,
        mjs->bcode_gen.len - offset - MJS_INIT_OFFSET_SIZE);
    mjs->bcode_gen.len += diff;
    cs_varint_encode(v, (uint8_t*)mjs->bcode_gen.buf + offset, llen);

    /*
   * If current parsing index is after the offset at which we've inserted new
   * varint, the index might need to be adjusted
   */
    if(p->cur_idx >= (int)offset) {
        p->cur_idx += diff;
    }
    return diff;
}

MJS_PRIVATE void mjs_bcode_part_add(struct mjs* mjs, const struct mjs_bcode_part* bp) {
    mbuf_append(&mjs->bcode_parts, bp, sizeof(*bp));
}

MJS_PRIVATE struct mjs_bcode_part* mjs_bcode_part_get(struct mjs* mjs, int num) {
    assert(num < mjs_bcode_parts_cnt(mjs));
    return (struct mjs_bcode_part*)(mjs->bcode_parts.buf + num * sizeof(struct mjs_bcode_part));
}

MJS_PRIVATE struct mjs_bcode_part* mjs_bcode_part_get_by_offset(struct mjs* mjs, size_t offset) {
    int i;
    int parts_cnt = mjs_bcode_parts_cnt(mjs);
    struct mjs_bcode_part* bp = NULL;

    if(offset >= mjs->bcode_len) {
        return NULL;
    }

    for(i = 0; i < parts_cnt; i++) {
        bp = mjs_bcode_part_get(mjs, i);
        if(offset < bp->start_idx + bp->data.len) {
            break;
        }
    }

    /* given the non-corrupted data, the needed part must be found */
    assert(i < parts_cnt);

    return bp;
}

MJS_PRIVATE int mjs_bcode_parts_cnt(struct mjs* mjs) {
    return mjs->bcode_parts.len / sizeof(struct mjs_bcode_part);
}

MJS_PRIVATE void mjs_bcode_commit(struct mjs* mjs) {
    struct mjs_bcode_part bp;
    memset(&bp, 0, sizeof(bp));

    /* Make sure the bcode doesn't occupy any extra space */
    mbuf_trim(&mjs->bcode_gen);

    /* Transfer the ownership of the bcode data */
    bp.data.p = mjs->bcode_gen.buf;
    bp.data.len = mjs->bcode_gen.len;
    mbuf_init(&mjs->bcode_gen, 0);

    bp.start_idx = mjs->bcode_len;
    bp.exec_res = MJS_ERRS_CNT;

    mjs_bcode_part_add(mjs, &bp);

    mjs->bcode_len += bp.data.len;
}
