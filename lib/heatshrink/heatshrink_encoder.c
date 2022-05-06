#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "heatshrink_encoder.h"

typedef enum {
    HSES_NOT_FULL,              /* input buffer not full enough */
    HSES_FILLED,                /* buffer is full */
    HSES_SEARCH,                /* searching for patterns */
    HSES_YIELD_TAG_BIT,         /* yield tag bit */
    HSES_YIELD_LITERAL,         /* emit literal byte */
    HSES_YIELD_BR_INDEX,        /* yielding backref index */
    HSES_YIELD_BR_LENGTH,       /* yielding backref length */
    HSES_SAVE_BACKLOG,          /* copying buffer to backlog */
    HSES_FLUSH_BITS,            /* flush bit buffer */
    HSES_DONE,                  /* done */
} HSE_state;

#if HEATSHRINK_DEBUGGING_LOGS
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define ASSERT(X) assert(X)
static const char *state_names[] = {
    "not_full",
    "filled",
    "search",
    "yield_tag_bit",
    "yield_literal",
    "yield_br_index",
    "yield_br_length",
    "save_backlog",
    "flush_bits",
    "done",
};
#else
#define LOG(...) /* no-op */
#define ASSERT(X) /* no-op */
#endif

// Encoder flags
enum {
    FLAG_IS_FINISHING = 0x01,
};

typedef struct {
    uint8_t *buf;               /* output buffer */
    size_t buf_size;            /* buffer size */
    size_t *output_size;        /* bytes pushed to buffer, so far */
} output_info;

#define MATCH_NOT_FOUND ((uint16_t)-1)

static uint16_t get_input_offset(heatshrink_encoder *hse);
static uint16_t get_input_buffer_size(heatshrink_encoder *hse);
static uint16_t get_lookahead_size(heatshrink_encoder *hse);
static void add_tag_bit(heatshrink_encoder *hse, output_info *oi, uint8_t tag);
static int can_take_byte(output_info *oi);
static int is_finishing(heatshrink_encoder *hse);
static void save_backlog(heatshrink_encoder *hse);

/* Push COUNT (max 8) bits to the output buffer, which has room. */
static void push_bits(heatshrink_encoder *hse, uint8_t count, uint8_t bits,
    output_info *oi);
static uint8_t push_outgoing_bits(heatshrink_encoder *hse, output_info *oi);
static void push_literal_byte(heatshrink_encoder *hse, output_info *oi);

#if HEATSHRINK_DYNAMIC_ALLOC
heatshrink_encoder *heatshrink_encoder_alloc(uint8_t* buffer, uint8_t window_sz2,
        uint8_t lookahead_sz2) {
    if ((window_sz2 < HEATSHRINK_MIN_WINDOW_BITS) ||
        (window_sz2 > HEATSHRINK_MAX_WINDOW_BITS) ||
        (lookahead_sz2 < HEATSHRINK_MIN_LOOKAHEAD_BITS) ||
        (lookahead_sz2 >= window_sz2)) {
        return NULL;
    }
    
    /* Note: 2 * the window size is used because the buffer needs to fit
     * (1 << window_sz2) bytes for the current input, and an additional
     * (1 << window_sz2) bytes for the previous buffer of input, which
     * will be scanned for useful backreferences. */
    size_t buf_sz = (2 << window_sz2);

    heatshrink_encoder *hse = HEATSHRINK_MALLOC(sizeof(*hse));
    if (hse == NULL) { return NULL; }
    hse->window_sz2 = window_sz2;
    hse->lookahead_sz2 = lookahead_sz2;
    hse->buffer = buffer;
    heatshrink_encoder_reset(hse);

#if HEATSHRINK_USE_INDEX
    size_t index_sz = buf_sz*sizeof(uint16_t);
    hse->search_index = HEATSHRINK_MALLOC(index_sz + sizeof(struct hs_index));
    if (hse->search_index == NULL) {
        HEATSHRINK_FREE(hse, sizeof(*hse) + buf_sz);
        return NULL;
    }
    hse->search_index->size = index_sz;
#endif

    LOG("-- allocated encoder with buffer size of %zu (%u byte input size)\n",
        buf_sz, get_input_buffer_size(hse));
    return hse;
}

void heatshrink_encoder_free(heatshrink_encoder *hse) {
#if HEATSHRINK_USE_INDEX
    size_t index_sz = sizeof(struct hs_index) + hse->search_index->size;
    HEATSHRINK_FREE(hse->search_index, index_sz);
    (void)index_sz;
#endif
    HEATSHRINK_FREE(hse, sizeof(heatshrink_encoder));
}
#endif

void heatshrink_encoder_reset(heatshrink_encoder *hse) {
    hse->input_size = 0;
    hse->state = HSES_NOT_FULL;
    hse->match_scan_index = 0;
    hse->flags = 0;
    hse->bit_index = 0x80;
    hse->current_byte = 0x00;
    hse->match_length = 0;

    hse->outgoing_bits = 0x0000;
    hse->outgoing_bits_count = 0;

    #ifdef LOOP_DETECT
    hse->loop_detect = (uint32_t)-1;
    #endif
}

HSE_sink_res heatshrink_encoder_sink(heatshrink_encoder *hse,
        uint8_t *in_buf, size_t size, size_t *input_size) {
    if ((hse == NULL) || (in_buf == NULL) || (input_size == NULL)) {
        return HSER_SINK_ERROR_NULL;
    }

    /* Sinking more content after saying the content is done, tsk tsk */
    if (is_finishing(hse)) { return HSER_SINK_ERROR_MISUSE; }

    /* Sinking more content before processing is done */
    if (hse->state != HSES_NOT_FULL) { return HSER_SINK_ERROR_MISUSE; }

    uint16_t write_offset = get_input_offset(hse) + hse->input_size;
    uint16_t ibs = get_input_buffer_size(hse);
    uint16_t rem = ibs - hse->input_size;
    uint16_t cp_sz = rem < size ? rem : size;

    memcpy(&hse->buffer[write_offset], in_buf, cp_sz);
    *input_size = cp_sz;
    hse->input_size += cp_sz;

    LOG("-- sunk %u bytes (of %zu) into encoder at %d, input buffer now has %u\n",
        cp_sz, size, write_offset, hse->input_size);
    if (cp_sz == rem) {
        LOG("-- internal buffer is now full\n");
        hse->state = HSES_FILLED;
    }

    return HSER_SINK_OK;
}


/***************
 * Compression *
 ***************/

static uint16_t find_longest_match(heatshrink_encoder *hse, uint16_t start,
    uint16_t end, const uint16_t maxlen, uint16_t *match_length);
static void do_indexing(heatshrink_encoder *hse);

static HSE_state st_step_search(heatshrink_encoder *hse);
static HSE_state st_yield_tag_bit(heatshrink_encoder *hse,
    output_info *oi);
static HSE_state st_yield_literal(heatshrink_encoder *hse,
    output_info *oi);
static HSE_state st_yield_br_index(heatshrink_encoder *hse,
    output_info *oi);
static HSE_state st_yield_br_length(heatshrink_encoder *hse,
    output_info *oi);
static HSE_state st_save_backlog(heatshrink_encoder *hse);
static HSE_state st_flush_bit_buffer(heatshrink_encoder *hse,
    output_info *oi);

HSE_poll_res heatshrink_encoder_poll(heatshrink_encoder *hse,
        uint8_t *out_buf, size_t out_buf_size, size_t *output_size) {
    if ((hse == NULL) || (out_buf == NULL) || (output_size == NULL)) {
        return HSER_POLL_ERROR_NULL;
    }
    if (out_buf_size == 0) {
        LOG("-- MISUSE: output buffer size is 0\n");
        return HSER_POLL_ERROR_MISUSE;
    }
    *output_size = 0;

    output_info oi;
    oi.buf = out_buf;
    oi.buf_size = out_buf_size;
    oi.output_size = output_size;

    while (1) {
        LOG("-- polling, state %u (%s), flags 0x%02x\n",
            hse->state, state_names[hse->state], hse->flags);

        uint8_t in_state = hse->state;
        switch (in_state) {
        case HSES_NOT_FULL:
            return HSER_POLL_EMPTY;
        case HSES_FILLED:
            do_indexing(hse);
            hse->state = HSES_SEARCH;
            break;
        case HSES_SEARCH:
            hse->state = st_step_search(hse);
            break;
        case HSES_YIELD_TAG_BIT:
            hse->state = st_yield_tag_bit(hse, &oi);
            break;
        case HSES_YIELD_LITERAL:
            hse->state = st_yield_literal(hse, &oi);
            break;
        case HSES_YIELD_BR_INDEX:
            hse->state = st_yield_br_index(hse, &oi);
            break;
        case HSES_YIELD_BR_LENGTH:
            hse->state = st_yield_br_length(hse, &oi);
            break;
        case HSES_SAVE_BACKLOG:
            hse->state = st_save_backlog(hse);
            break;
        case HSES_FLUSH_BITS:
            hse->state = st_flush_bit_buffer(hse, &oi);
            /* fall through */
        case HSES_DONE:
            return HSER_POLL_EMPTY;
        default:
            LOG("-- bad state %s\n", state_names[hse->state]);
            return HSER_POLL_ERROR_MISUSE;
        }

        if (hse->state == in_state) {
            /* Check if output buffer is exhausted. */
            if (*output_size == out_buf_size) return HSER_POLL_MORE;
        }
    }
}

HSE_finish_res heatshrink_encoder_finish(heatshrink_encoder *hse) {
    if (hse == NULL) { return HSER_FINISH_ERROR_NULL; }
    LOG("-- setting is_finishing flag\n");
    hse->flags |= FLAG_IS_FINISHING;
    if (hse->state == HSES_NOT_FULL) { hse->state = HSES_FILLED; }
    return hse->state == HSES_DONE ? HSER_FINISH_DONE : HSER_FINISH_MORE;
}

static HSE_state st_step_search(heatshrink_encoder *hse) {
    uint16_t window_length = get_input_buffer_size(hse);
    uint16_t lookahead_sz = get_lookahead_size(hse);
    uint16_t msi = hse->match_scan_index;
    LOG("## step_search, scan @ +%d (%d/%d), input size %d\n",
        msi, hse->input_size + msi, 2*window_length, hse->input_size);

    bool fin = is_finishing(hse);
    if (msi > hse->input_size - (fin ? 1 : lookahead_sz)) {
        /* Current search buffer is exhausted, copy it into the
         * backlog and await more input. */
        LOG("-- end of search @ %d\n", msi);
        return fin ? HSES_FLUSH_BITS : HSES_SAVE_BACKLOG;
    }

    uint16_t input_offset = get_input_offset(hse);
    uint16_t end = input_offset + msi;
    uint16_t start = end - window_length;

    uint16_t max_possible = lookahead_sz;
    if (hse->input_size - msi < lookahead_sz) {
        max_possible = hse->input_size - msi;
    }
    
    uint16_t match_length = 0;
    uint16_t match_pos = find_longest_match(hse,
        start, end, max_possible, &match_length);
    
    if (match_pos == MATCH_NOT_FOUND) {
        LOG("ss Match not found\n");
        hse->match_scan_index++;
        hse->match_length = 0;
        return HSES_YIELD_TAG_BIT;
    } else {
        LOG("ss Found match of %d bytes at %d\n", match_length, match_pos);
        hse->match_pos = match_pos;
        hse->match_length = match_length;
        ASSERT(match_pos <= 1 << HEATSHRINK_ENCODER_WINDOW_BITS(hse) /*window_length*/);

        return HSES_YIELD_TAG_BIT;
    }
}

static HSE_state st_yield_tag_bit(heatshrink_encoder *hse,
        output_info *oi) {
    if (can_take_byte(oi)) {
        if (hse->match_length == 0) {
            add_tag_bit(hse, oi, HEATSHRINK_LITERAL_MARKER);
            return HSES_YIELD_LITERAL;
        } else {
            add_tag_bit(hse, oi, HEATSHRINK_BACKREF_MARKER);
            hse->outgoing_bits = hse->match_pos - 1;
            hse->outgoing_bits_count = HEATSHRINK_ENCODER_WINDOW_BITS(hse);
            return HSES_YIELD_BR_INDEX;
        }
    } else {
        return HSES_YIELD_TAG_BIT; /* output is full, continue */
    }
}

static HSE_state st_yield_literal(heatshrink_encoder *hse,
        output_info *oi) {
    if (can_take_byte(oi)) {
        push_literal_byte(hse, oi);
        return HSES_SEARCH;
    } else {
        return HSES_YIELD_LITERAL;
    }
}

static HSE_state st_yield_br_index(heatshrink_encoder *hse,
        output_info *oi) {
    if (can_take_byte(oi)) {
        LOG("-- yielding backref index %u\n", hse->match_pos);
        if (push_outgoing_bits(hse, oi) > 0) {
            return HSES_YIELD_BR_INDEX; /* continue */
        } else {
            hse->outgoing_bits = hse->match_length - 1;
            hse->outgoing_bits_count = HEATSHRINK_ENCODER_LOOKAHEAD_BITS(hse);
            return HSES_YIELD_BR_LENGTH; /* done */
        }
    } else {
        return HSES_YIELD_BR_INDEX; /* continue */
    }
}

static HSE_state st_yield_br_length(heatshrink_encoder *hse,
        output_info *oi) {
    if (can_take_byte(oi)) {
        LOG("-- yielding backref length %u\n", hse->match_length);
        if (push_outgoing_bits(hse, oi) > 0) {
            return HSES_YIELD_BR_LENGTH;
        } else {
            hse->match_scan_index += hse->match_length;
            hse->match_length = 0;
            return HSES_SEARCH;
        }
    } else {
        return HSES_YIELD_BR_LENGTH;
    }
}

static HSE_state st_save_backlog(heatshrink_encoder *hse) {
    LOG("-- saving backlog\n");
    save_backlog(hse);
    return HSES_NOT_FULL;
}

static HSE_state st_flush_bit_buffer(heatshrink_encoder *hse,
        output_info *oi) {
    if (hse->bit_index == 0x80) {
        LOG("-- done!\n");
        return HSES_DONE;
    } else if (can_take_byte(oi)) {
        LOG("-- flushing remaining byte (bit_index == 0x%02x)\n", hse->bit_index);
        oi->buf[(*oi->output_size)++] = hse->current_byte;
        LOG("-- done!\n");
        return HSES_DONE;
    } else {
        return HSES_FLUSH_BITS;
    }
}

static void add_tag_bit(heatshrink_encoder *hse, output_info *oi, uint8_t tag) {
    LOG("-- adding tag bit: %d\n", tag);
    push_bits(hse, 1, tag, oi);
}

static uint16_t get_input_offset(heatshrink_encoder *hse) {
    return get_input_buffer_size(hse);
}

static uint16_t get_input_buffer_size(heatshrink_encoder *hse) {
    return (1 << HEATSHRINK_ENCODER_WINDOW_BITS(hse));
    (void)hse;
}

static uint16_t get_lookahead_size(heatshrink_encoder *hse) {
    return (1 << HEATSHRINK_ENCODER_LOOKAHEAD_BITS(hse));
    (void)hse;
}

static void do_indexing(heatshrink_encoder *hse) {
#if HEATSHRINK_USE_INDEX
    /* Build an index array I that contains flattened linked lists
     * for the previous instances of every byte in the buffer.
     * 
     * For example, if buf[200] == 'x', then index[200] will either
     * be an offset i such that buf[i] == 'x', or a negative offset
     * to indicate end-of-list. This significantly speeds up matching,
     * while only using sizeof(uint16_t)*sizeof(buffer) bytes of RAM.
     *
     * Future optimization options:
     * 1. Since any negative value represents end-of-list, the other
     *    15 bits could be used to improve the index dynamically.
     *    
     * 2. Likewise, the last lookahead_sz bytes of the index will
     *    not be usable, so temporary data could be stored there to
     *    dynamically improve the index.
     * */
    struct hs_index *hsi = HEATSHRINK_ENCODER_INDEX(hse);
    int16_t last[256];
    memset(last, 0xFF, sizeof(last));

    uint8_t * const data = hse->buffer;
    int16_t * const index = hsi->index;

    const uint16_t input_offset = get_input_offset(hse);
    const uint16_t end = input_offset + hse->input_size;

    for (uint16_t i=0; i<end; i++) {
        uint8_t v = data[i];
        int16_t lv = last[v];
        index[i] = lv;
        last[v] = i;
    }
#else
    (void)hse;
#endif
}

static int is_finishing(heatshrink_encoder *hse) {
    return hse->flags & FLAG_IS_FINISHING;
}

static int can_take_byte(output_info *oi) {
    return *oi->output_size < oi->buf_size;
}

/* Return the longest match for the bytes at buf[end:end+maxlen] between
 * buf[start] and buf[end-1]. If no match is found, return -1. */
static uint16_t find_longest_match(heatshrink_encoder *hse, uint16_t start,
        uint16_t end, const uint16_t maxlen, uint16_t *match_length) {
    LOG("-- scanning for match of buf[%u:%u] between buf[%u:%u] (max %u bytes)\n",
        end, end + maxlen, start, end + maxlen - 1, maxlen);
    uint8_t *buf = hse->buffer;

    uint16_t match_maxlen = 0;
    uint16_t match_index = MATCH_NOT_FOUND;

    uint16_t len = 0;
    uint8_t * const needlepoint = &buf[end];
#if HEATSHRINK_USE_INDEX
    struct hs_index *hsi = HEATSHRINK_ENCODER_INDEX(hse);
    int16_t pos = hsi->index[end];

    while (pos - (int16_t)start >= 0) {
        uint8_t * const pospoint = &buf[pos];
        len = 0;

        /* Only check matches that will potentially beat the current maxlen.
         * This is redundant with the index if match_maxlen is 0, but the
         * added branch overhead to check if it == 0 seems to be worse. */
        if (pospoint[match_maxlen] != needlepoint[match_maxlen]) {
            pos = hsi->index[pos];
            continue;
        }

        for (len = 1; len < maxlen; len++) {
            if (pospoint[len] != needlepoint[len]) break;
        }

        if (len > match_maxlen) {
            match_maxlen = len;
            match_index = pos;
            if (len == maxlen) { break; } /* won't find better */
        }
        pos = hsi->index[pos];
    }
#else    
    for (int16_t pos=end - 1; pos - (int16_t)start >= 0; pos--) {
        uint8_t * const pospoint = &buf[pos];
        if ((pospoint[match_maxlen] == needlepoint[match_maxlen])
            && (*pospoint == *needlepoint)) {
            for (len=1; len<maxlen; len++) {
                if (0) {
                    LOG("  --> cmp buf[%d] == 0x%02x against %02x (start %u)\n",
                        pos + len, pospoint[len], needlepoint[len], start);
                }
                if (pospoint[len] != needlepoint[len]) { break; }
            }
            if (len > match_maxlen) {
                match_maxlen = len;
                match_index = pos;
                if (len == maxlen) { break; } /* don't keep searching */
            }
        }
    }
#endif
    
    const size_t break_even_point =
      (1 + HEATSHRINK_ENCODER_WINDOW_BITS(hse) +
          HEATSHRINK_ENCODER_LOOKAHEAD_BITS(hse));

    /* Instead of comparing break_even_point against 8*match_maxlen,
     * compare match_maxlen against break_even_point/8 to avoid
     * overflow. Since MIN_WINDOW_BITS and MIN_LOOKAHEAD_BITS are 4 and
     * 3, respectively, break_even_point/8 will always be at least 1. */
    if (match_maxlen > (break_even_point / 8)) {
        LOG("-- best match: %u bytes at -%u\n",
            match_maxlen, end - match_index);
        *match_length = match_maxlen;
        return end - match_index;
    }
    LOG("-- none found\n");
    return MATCH_NOT_FOUND;
}

static uint8_t push_outgoing_bits(heatshrink_encoder *hse, output_info *oi) {
    uint8_t count = 0;
    uint8_t bits = 0;
    if (hse->outgoing_bits_count > 8) {
        count = 8;
        bits = hse->outgoing_bits >> (hse->outgoing_bits_count - 8);
    } else {
        count = hse->outgoing_bits_count;
        bits = hse->outgoing_bits;
    }

    if (count > 0) {
        LOG("-- pushing %d outgoing bits: 0x%02x\n", count, bits);
        push_bits(hse, count, bits, oi);
        hse->outgoing_bits_count -= count;
    }
    return count;
}

/* Push COUNT (max 8) bits to the output buffer, which has room.
 * Bytes are set from the lowest bits, up. */
static void push_bits(heatshrink_encoder *hse, uint8_t count, uint8_t bits,
        output_info *oi) {
    ASSERT(count <= 8);
    LOG("++ push_bits: %d bits, input of 0x%02x\n", count, bits);

    /* If adding a whole byte and at the start of a new output byte,
     * just push it through whole and skip the bit IO loop. */
    if (count == 8 && hse->bit_index == 0x80) {
        oi->buf[(*oi->output_size)++] = bits;
    } else {
        for (int i=count - 1; i>=0; i--) {
            bool bit = bits & (1 << i);
            if (bit) { hse->current_byte |= hse->bit_index; }
            if (0) {
                LOG("  -- setting bit %d at bit index 0x%02x, byte => 0x%02x\n",
                    bit ? 1 : 0, hse->bit_index, hse->current_byte);
            }
            hse->bit_index >>= 1;
            if (hse->bit_index == 0x00) {
                hse->bit_index = 0x80;
                LOG(" > pushing byte 0x%02x\n", hse->current_byte);
                oi->buf[(*oi->output_size)++] = hse->current_byte;
                hse->current_byte = 0x00;
            }
        }
    }
}

static void push_literal_byte(heatshrink_encoder *hse, output_info *oi) {
    uint16_t processed_offset = hse->match_scan_index - 1;
    uint16_t input_offset = get_input_offset(hse) + processed_offset;
    uint8_t c = hse->buffer[input_offset];
    LOG("-- yielded literal byte 0x%02x ('%c') from +%d\n",
        c, isprint(c) ? c : '.', input_offset);
    push_bits(hse, 8, c, oi);
}

static void save_backlog(heatshrink_encoder *hse) {
    size_t input_buf_sz = get_input_buffer_size(hse);
    
    uint16_t msi = hse->match_scan_index;
    
    /* Copy processed data to beginning of buffer, so it can be
     * used for future matches. Don't bother checking whether the
     * input is less than the maximum size, because if it isn't,
     * we're done anyway. */
    uint16_t rem = input_buf_sz - msi; // unprocessed bytes
    uint16_t shift_sz = input_buf_sz + rem;

    memmove(&hse->buffer[0],
        &hse->buffer[input_buf_sz - rem],
        shift_sz);
        
    hse->match_scan_index = 0;
    hse->input_size -= input_buf_sz - rem;
}
