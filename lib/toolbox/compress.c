#include "compress.h"

#include <furi.h>
#include <lib/heatshrink/heatshrink_encoder.h>
#include <lib/heatshrink/heatshrink_decoder.h>
#include <stdint.h>

#define TAG "Compress"

/** Defines encoder and decoder window size */
#define COMPRESS_EXP_BUFF_SIZE_LOG (8u)

/** Defines encoder and decoder lookahead buffer size */
#define COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG (4u)

/** Buffer size for input data */
#define COMPRESS_ICON_ENCODED_BUFF_SIZE (256u)

static bool compress_decode_internal(
    heatshrink_decoder* decoder,
    const uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size);

typedef struct {
    uint8_t is_compressed;
    uint8_t reserved;
    uint16_t compressed_buff_size;
} CompressHeader;

_Static_assert(sizeof(CompressHeader) == 4, "Incorrect CompressHeader size");

struct CompressIcon {
    heatshrink_decoder* decoder;
    uint8_t* buffer;
    size_t buffer_size;
};

CompressIcon* compress_icon_alloc(size_t decode_buf_size) {
    CompressIcon* instance = malloc(sizeof(CompressIcon));
    instance->decoder = heatshrink_decoder_alloc(
        COMPRESS_ICON_ENCODED_BUFF_SIZE,
        COMPRESS_EXP_BUFF_SIZE_LOG,
        COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG);
    heatshrink_decoder_reset(instance->decoder);

    instance->buffer_size = decode_buf_size + 4; /* To account for heatshrink's poller quirks */
    instance->buffer = malloc(instance->buffer_size);

    return instance;
}

void compress_icon_free(CompressIcon* instance) {
    furi_check(instance);
    free(instance->buffer);
    heatshrink_decoder_free(instance->decoder);
    free(instance);
}

void compress_icon_decode(CompressIcon* instance, const uint8_t* icon_data, uint8_t** output) {
    furi_check(instance);
    furi_check(icon_data);
    furi_check(output);

    CompressHeader* header = (CompressHeader*)icon_data;
    if(header->is_compressed) {
        size_t decoded_size = 0;
        /* If decompression fails - check that decode_buf_size is large enough */
        furi_check(compress_decode_internal(
            instance->decoder,
            icon_data,
            /* Decoder will check/process headers again - need to pass them */
            sizeof(CompressHeader) + header->compressed_buff_size,
            instance->buffer,
            instance->buffer_size,
            &decoded_size));
        *output = instance->buffer;
    } else {
        *output = (uint8_t*)&icon_data[1];
    }
}

struct Compress {
    heatshrink_encoder* encoder;
    heatshrink_decoder* decoder;
};

Compress* compress_alloc(uint16_t compress_buff_size) {
    Compress* compress = malloc(sizeof(Compress));
    compress->encoder =
        heatshrink_encoder_alloc(COMPRESS_EXP_BUFF_SIZE_LOG, COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG);
    compress->decoder = heatshrink_decoder_alloc(
        compress_buff_size, COMPRESS_EXP_BUFF_SIZE_LOG, COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG);

    return compress;
}

void compress_free(Compress* compress) {
    furi_check(compress);

    heatshrink_encoder_free(compress->encoder);
    heatshrink_decoder_free(compress->decoder);
    free(compress);
}

static bool compress_encode_internal(
    heatshrink_encoder* encoder,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size) {
    furi_check(encoder);
    furi_check(data_in);
    furi_check(data_in_size);

    size_t sink_size = 0;
    size_t poll_size = 0;
    HSE_sink_res sink_res;
    HSE_poll_res poll_res;
    HSE_finish_res finish_res;
    bool encode_failed = false;
    size_t sunk = 0;
    size_t res_buff_size = sizeof(CompressHeader);

    /* Sink data to encoding buffer */
    while((sunk < data_in_size) && !encode_failed) {
        sink_res =
            heatshrink_encoder_sink(encoder, &data_in[sunk], data_in_size - sunk, &sink_size);
        if(sink_res != HSER_SINK_OK) {
            encode_failed = true;
            break;
        }
        sunk += sink_size;
        do {
            poll_res = heatshrink_encoder_poll(
                encoder, &data_out[res_buff_size], data_out_size - res_buff_size, &poll_size);
            if(poll_res < 0) {
                encode_failed = true;
                break;
            }
            res_buff_size += poll_size;
        } while(poll_res == HSER_POLL_MORE);
    }

    /* Notify sinking complete and poll encoded data */
    finish_res = heatshrink_encoder_finish(encoder);
    if(finish_res < 0) {
        encode_failed = true;
    } else {
        do {
            poll_res = heatshrink_encoder_poll(
                encoder, &data_out[res_buff_size], data_out_size - res_buff_size, &poll_size);
            if(poll_res < 0) {
                encode_failed = true;
                break;
            }
            res_buff_size += poll_size;
            finish_res = heatshrink_encoder_finish(encoder);
        } while(finish_res != HSER_FINISH_DONE);
    }

    bool result = true;
    /* Write encoded data to output buffer if compression is efficient. Otherwise, write header and original data */
    if(!encode_failed && (res_buff_size < data_in_size + 1)) {
        CompressHeader header = {
            .is_compressed = 0x01,
            .reserved = 0x00,
            .compressed_buff_size = res_buff_size - sizeof(CompressHeader)};
        memcpy(data_out, &header, sizeof(header));
        *data_res_size = res_buff_size;
    } else if(data_out_size > data_in_size) {
        data_out[0] = 0x00;
        memcpy(&data_out[1], data_in, data_in_size);
        *data_res_size = data_in_size + 1;
    } else {
        *data_res_size = 0;
        result = false;
    }
    heatshrink_encoder_reset(encoder);
    return result;
}

static bool compress_decode_internal(
    heatshrink_decoder* decoder,
    const uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size) {
    furi_check(decoder);
    furi_check(data_in);
    furi_check(data_out);
    furi_check(data_res_size);

    bool result = false;
    bool decode_failed = false;
    HSD_sink_res sink_res;
    HSD_poll_res poll_res;
    HSD_finish_res finish_res;
    size_t sink_size = 0;
    size_t res_buff_size = 0;
    size_t poll_size = 0;

    CompressHeader* header = (CompressHeader*)data_in;
    if(header->is_compressed) {
        /* Sink data to decoding buffer */
        size_t compressed_size = header->compressed_buff_size;
        size_t sunk = 0;
        while(sunk < compressed_size && !decode_failed) {
            sink_res = heatshrink_decoder_sink(
                decoder,
                (uint8_t*)&data_in[sizeof(CompressHeader) + sunk],
                compressed_size - sunk,
                &sink_size);
            if(sink_res < 0) {
                decode_failed = true;
                break;
            }
            sunk += sink_size;
            do {
                poll_res = heatshrink_decoder_poll(
                    decoder, &data_out[res_buff_size], data_out_size - res_buff_size, &poll_size);
                if((poll_res < 0) || ((data_out_size - res_buff_size) == 0)) {
                    decode_failed = true;
                    break;
                }
                res_buff_size += poll_size;
            } while(poll_res == HSDR_POLL_MORE);
        }
        /* Notify sinking complete and poll decoded data */
        if(!decode_failed) {
            finish_res = heatshrink_decoder_finish(decoder);
            if(finish_res < 0) {
                decode_failed = true;
            } else {
                do {
                    poll_res = heatshrink_decoder_poll(
                        decoder,
                        &data_out[res_buff_size],
                        data_out_size - res_buff_size,
                        &poll_size);
                    res_buff_size += poll_size;
                    finish_res = heatshrink_decoder_finish(decoder);
                } while(finish_res != HSDR_FINISH_DONE);
            }
        }
        *data_res_size = res_buff_size;
        result = !decode_failed;
    } else if(data_out_size >= data_in_size - 1) {
        memcpy(data_out, &data_in[1], data_in_size);
        *data_res_size = data_in_size - 1;
        result = true;
    } else {
        /* Not enough space in output buffer */
        result = false;
    }
    heatshrink_decoder_reset(decoder);
    return result;
}

bool compress_encode(
    Compress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size) {
    return compress_encode_internal(
        compress->encoder, data_in, data_in_size, data_out, data_out_size, data_res_size);
}

bool compress_decode(
    Compress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size) {
    return compress_decode_internal(
        compress->decoder, data_in, data_in_size, data_out, data_out_size, data_res_size);
}
