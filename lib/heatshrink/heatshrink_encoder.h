#ifndef HEATSHRINK_ENCODER_H
#define HEATSHRINK_ENCODER_H

#include <stdint.h>
#include <stddef.h>
#include "heatshrink_common.h"
#include "heatshrink_config.h"

typedef enum {
    HSER_SINK_OK,               /* data sunk into input buffer */
    HSER_SINK_ERROR_NULL=-1,    /* NULL argument */
    HSER_SINK_ERROR_MISUSE=-2,  /* API misuse */
} HSE_sink_res;

typedef enum {
    HSER_POLL_EMPTY,            /* input exhausted */
    HSER_POLL_MORE,             /* poll again for more output  */
    HSER_POLL_ERROR_NULL=-1,    /* NULL argument */
    HSER_POLL_ERROR_MISUSE=-2,  /* API misuse */
} HSE_poll_res;

typedef enum {
    HSER_FINISH_DONE,           /* encoding is complete */
    HSER_FINISH_MORE,           /* more output remaining; use poll */
    HSER_FINISH_ERROR_NULL=-1,  /* NULL argument */
} HSE_finish_res;

#if HEATSHRINK_DYNAMIC_ALLOC
#define HEATSHRINK_ENCODER_WINDOW_BITS(HSE) \
    ((HSE)->window_sz2)
#define HEATSHRINK_ENCODER_LOOKAHEAD_BITS(HSE) \
    ((HSE)->lookahead_sz2)
#define HEATSHRINK_ENCODER_INDEX(HSE) \
    ((HSE)->search_index)
struct hs_index {
    uint16_t size;
    int16_t index[];
};
#else
#define HEATSHRINK_ENCODER_WINDOW_BITS(_) \
    (HEATSHRINK_STATIC_WINDOW_BITS)
#define HEATSHRINK_ENCODER_LOOKAHEAD_BITS(_) \
    (HEATSHRINK_STATIC_LOOKAHEAD_BITS)
#define HEATSHRINK_ENCODER_INDEX(HSE) \
    (&(HSE)->search_index)
struct hs_index {
    uint16_t size;
    int16_t index[2 << HEATSHRINK_STATIC_WINDOW_BITS];
};
#endif

typedef struct {
    uint16_t input_size;        /* bytes in input buffer */
    uint16_t match_scan_index;
    uint16_t match_length;
    uint16_t match_pos;
    uint16_t outgoing_bits;     /* enqueued outgoing bits */
    uint8_t outgoing_bits_count;
    uint8_t flags;
    uint8_t state;              /* current state machine node */
    uint8_t current_byte;       /* current byte of output */
    uint8_t bit_index;          /* current bit index */
#if HEATSHRINK_DYNAMIC_ALLOC
    uint8_t window_sz2;         /* 2^n size of window */
    uint8_t lookahead_sz2;      /* 2^n size of lookahead */
#if HEATSHRINK_USE_INDEX
    struct hs_index *search_index;
#endif
    /* input buffer and / sliding window for expansion */
    uint8_t* buffer;
#else
    #if HEATSHRINK_USE_INDEX
        struct hs_index search_index;
    #endif
    /* input buffer and / sliding window for expansion */
    uint8_t buffer[2 << HEATSHRINK_ENCODER_WINDOW_BITS(_)];
#endif
} heatshrink_encoder;

#if HEATSHRINK_DYNAMIC_ALLOC
/* Allocate a new encoder struct and its buffers.
 * Returns NULL on error. */
heatshrink_encoder *heatshrink_encoder_alloc(uint8_t* buffer, uint8_t window_sz2,
    uint8_t lookahead_sz2);

/* Free an encoder. */
void heatshrink_encoder_free(heatshrink_encoder *hse);
#endif

/* Reset an encoder. */
void heatshrink_encoder_reset(heatshrink_encoder *hse);

/* Sink up to SIZE bytes from IN_BUF into the encoder.
 * INPUT_SIZE is set to the number of bytes actually sunk (in case a
 * buffer was filled.). */
HSE_sink_res heatshrink_encoder_sink(heatshrink_encoder *hse,
    uint8_t *in_buf, size_t size, size_t *input_size);

/* Poll for output from the encoder, copying at most OUT_BUF_SIZE bytes into
 * OUT_BUF (setting *OUTPUT_SIZE to the actual amount copied). */
HSE_poll_res heatshrink_encoder_poll(heatshrink_encoder *hse,
    uint8_t *out_buf, size_t out_buf_size, size_t *output_size);

/* Notify the encoder that the input stream is finished.
 * If the return value is HSER_FINISH_MORE, there is still more output, so
 * call heatshrink_encoder_poll and repeat. */
HSE_finish_res heatshrink_encoder_finish(heatshrink_encoder *hse);

#endif
