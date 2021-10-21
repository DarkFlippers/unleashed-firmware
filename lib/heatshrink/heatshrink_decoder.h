#ifndef HEATSHRINK_DECODER_H
#define HEATSHRINK_DECODER_H

#include <stdint.h>
#include <stddef.h>
#include "heatshrink_common.h"
#include "heatshrink_config.h"

typedef enum {
    HSDR_SINK_OK,               /* data sunk, ready to poll */
    HSDR_SINK_FULL,             /* out of space in internal buffer */
    HSDR_SINK_ERROR_NULL=-1,    /* NULL argument */
} HSD_sink_res;

typedef enum {
    HSDR_POLL_EMPTY,            /* input exhausted */
    HSDR_POLL_MORE,             /* more data remaining, call again w/ fresh output buffer */
    HSDR_POLL_ERROR_NULL=-1,    /* NULL arguments */
    HSDR_POLL_ERROR_UNKNOWN=-2,
} HSD_poll_res;

typedef enum {
    HSDR_FINISH_DONE,           /* output is done */
    HSDR_FINISH_MORE,           /* more output remains */
    HSDR_FINISH_ERROR_NULL=-1,  /* NULL arguments */
} HSD_finish_res;

#if HEATSHRINK_DYNAMIC_ALLOC
#define HEATSHRINK_DECODER_INPUT_BUFFER_SIZE(BUF) \
    ((BUF)->input_buffer_size)
#define HEATSHRINK_DECODER_WINDOW_BITS(BUF) \
    ((BUF)->window_sz2)
#define HEATSHRINK_DECODER_LOOKAHEAD_BITS(BUF) \
    ((BUF)->lookahead_sz2)
#else
#define HEATSHRINK_DECODER_INPUT_BUFFER_SIZE(_) \
    HEATSHRINK_STATIC_INPUT_BUFFER_SIZE
#define HEATSHRINK_DECODER_WINDOW_BITS(_) \
    (HEATSHRINK_STATIC_WINDOW_BITS)
#define HEATSHRINK_DECODER_LOOKAHEAD_BITS(BUF) \
    (HEATSHRINK_STATIC_LOOKAHEAD_BITS)
#endif

typedef struct {
    uint16_t input_size;        /* bytes in input buffer */
    uint16_t input_index;       /* offset to next unprocessed input byte */
    uint16_t output_count;      /* how many bytes to output */
    uint16_t output_index;      /* index for bytes to output */
    uint16_t head_index;        /* head of window buffer */
    uint8_t state;              /* current state machine node */
    uint8_t current_byte;       /* current byte of input */
    uint8_t bit_index;          /* current bit index */

#if HEATSHRINK_DYNAMIC_ALLOC
    /* Fields that are only used if dynamically allocated. */
    uint8_t window_sz2;         /* window buffer bits */
    uint8_t lookahead_sz2;      /* lookahead bits */
    uint16_t input_buffer_size; /* input buffer size */

    /* Input buffer, then expansion window buffer */
    uint8_t* buffers;
#else
    /* Input buffer, then expansion window buffer */
    uint8_t buffers[(1 << HEATSHRINK_DECODER_WINDOW_BITS(_))
        + HEATSHRINK_DECODER_INPUT_BUFFER_SIZE(_)];
#endif
} heatshrink_decoder;

#if HEATSHRINK_DYNAMIC_ALLOC
/* Allocate a decoder with an input buffer of INPUT_BUFFER_SIZE bytes,
 * an expansion buffer size of 2^WINDOW_SZ2, and a lookahead
 * size of 2^lookahead_sz2. (The window buffer and lookahead sizes
 * must match the settings used when the data was compressed.)
 * Returns NULL on error. */
heatshrink_decoder *heatshrink_decoder_alloc(uint8_t* buffer, uint16_t input_buffer_size,
    uint8_t expansion_buffer_sz2, uint8_t lookahead_sz2);

/* Free a decoder. */
void heatshrink_decoder_free(heatshrink_decoder *hsd);
#endif

/* Reset a decoder. */
void heatshrink_decoder_reset(heatshrink_decoder *hsd);

/* Sink at most SIZE bytes from IN_BUF into the decoder. *INPUT_SIZE is set to
 * indicate how many bytes were actually sunk (in case a buffer was filled). */
HSD_sink_res heatshrink_decoder_sink(heatshrink_decoder *hsd,
    uint8_t *in_buf, size_t size, size_t *input_size);

/* Poll for output from the decoder, copying at most OUT_BUF_SIZE bytes into
 * OUT_BUF (setting *OUTPUT_SIZE to the actual amount copied). */
HSD_poll_res heatshrink_decoder_poll(heatshrink_decoder *hsd,
    uint8_t *out_buf, size_t out_buf_size, size_t *output_size);

/* Notify the dencoder that the input stream is finished.
 * If the return value is HSDR_FINISH_MORE, there is still more output, so
 * call heatshrink_decoder_poll and repeat. */
HSD_finish_res heatshrink_decoder_finish(heatshrink_decoder *hsd);

#endif
