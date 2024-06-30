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

#define COMPRESS_ICON_ENCODED_BUFF_SIZE (256u)

const CompressConfigHeatshrink compress_config_heatshrink_default = {
    .window_sz2 = COMPRESS_EXP_BUFF_SIZE_LOG,
    .lookahead_sz2 = COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG,
    .input_buffer_sz = COMPRESS_ICON_ENCODED_BUFF_SIZE,
};

/** Buffer size for input data */
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
    const void* config;
    heatshrink_encoder* encoder;
    heatshrink_decoder* decoder;
};

Compress* compress_alloc(CompressType type, const void* config) {
    furi_check(type == CompressTypeHeatshrink);
    furi_check(config);

    Compress* compress = malloc(sizeof(Compress));
    compress->config = config;
    compress->encoder = NULL;
    compress->decoder = NULL;

    return compress;
}

void compress_free(Compress* compress) {
    furi_check(compress);

    if(compress->encoder) {
        heatshrink_encoder_free(compress->encoder);
    }
    if(compress->decoder) {
        heatshrink_decoder_free(compress->decoder);
    }
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

    heatshrink_encoder_reset(encoder);
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
    return result;
}

static inline bool compress_decoder_poll(
    heatshrink_decoder* decoder,
    uint8_t* decompressed_chunk,
    size_t decomp_buffer_size,
    CompressIoCallback write_cb,
    void* write_context) {
    HSD_poll_res poll_res;
    size_t poll_size;

    do {
        poll_res =
            heatshrink_decoder_poll(decoder, decompressed_chunk, decomp_buffer_size, &poll_size);
        if(poll_res < 0) {
            return false;
        }

        size_t write_size = write_cb(write_context, decompressed_chunk, poll_size);
        if(write_size != poll_size) {
            return false;
        }
    } while(poll_res == HSDR_POLL_MORE);

    return true;
}

static bool compress_decode_stream_internal(
    heatshrink_decoder* decoder,
    const size_t work_buffer_size,
    CompressIoCallback read_cb,
    void* read_context,
    CompressIoCallback write_cb,
    void* write_context) {
    bool decode_failed = false;
    HSD_sink_res sink_res;
    HSD_finish_res finish_res;
    size_t read_size = 0;
    size_t sink_size = 0;

    uint8_t* compressed_chunk = malloc(work_buffer_size);
    uint8_t* decompressed_chunk = malloc(work_buffer_size);

    /* Sink data to decoding buffer */
    do {
        read_size = read_cb(read_context, compressed_chunk, work_buffer_size);

        size_t sunk = 0;
        while(sunk < read_size && !decode_failed) {
            sink_res = heatshrink_decoder_sink(
                decoder, &compressed_chunk[sunk], read_size - sunk, &sink_size);
            if(sink_res < 0) {
                decode_failed = true;
                break;
            }
            sunk += sink_size;

            if(!compress_decoder_poll(
                   decoder, decompressed_chunk, work_buffer_size, write_cb, write_context)) {
                decode_failed = true;
                break;
            }
        }
    } while(!decode_failed && read_size);

    /* Notify sinking complete and poll decoded data */
    if(!decode_failed) {
        while((finish_res = heatshrink_decoder_finish(decoder)) != HSDR_FINISH_DONE) {
            if(finish_res < 0) {
                decode_failed = true;
                break;
            }

            if(!compress_decoder_poll(
                   decoder, decompressed_chunk, work_buffer_size, write_cb, write_context)) {
                decode_failed = true;
                break;
            }
        }
    }

    free(compressed_chunk);
    free(decompressed_chunk);

    return !decode_failed;
}

typedef struct {
    uint8_t* data_ptr;
    size_t data_size;
    bool is_source;
} MemoryStreamState;

static int32_t memory_stream_io_callback(void* context, uint8_t* ptr, size_t size) {
    MemoryStreamState* state = (MemoryStreamState*)context;

    if(size > state->data_size) {
        size = state->data_size;
    }
    if(state->is_source) {
        memcpy(ptr, state->data_ptr, size);
    } else {
        memcpy(state->data_ptr, ptr, size);
    }
    state->data_ptr += size;
    state->data_size -= size;
    return size;
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

    CompressHeader* header = (CompressHeader*)data_in;
    if(header->is_compressed) {
        MemoryStreamState compressed_context = {
            .data_ptr = (uint8_t*)&data_in[sizeof(CompressHeader)],
            .data_size = header->compressed_buff_size,
            .is_source = true,
        };
        MemoryStreamState decompressed_context = {
            .data_ptr = data_out,
            .data_size = data_out_size,
            .is_source = false,
        };
        heatshrink_decoder_reset(decoder);
        if((result = compress_decode_stream_internal(
                decoder,
                COMPRESS_ICON_ENCODED_BUFF_SIZE,
                memory_stream_io_callback,
                &compressed_context,
                memory_stream_io_callback,
                &decompressed_context))) {
            *data_res_size = data_out_size - decompressed_context.data_size;
        }
    } else if(data_out_size >= data_in_size - 1) {
        memcpy(data_out, &data_in[1], data_in_size);
        *data_res_size = data_in_size - 1;
        result = true;
    } else {
        /* Not enough space in output buffer */
        result = false;
    }
    return result;
}

bool compress_encode(
    Compress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size) {
    if(!compress->encoder) {
        CompressConfigHeatshrink* hs_config = (CompressConfigHeatshrink*)compress->config;
        compress->encoder =
            heatshrink_encoder_alloc(hs_config->window_sz2, hs_config->lookahead_sz2);
    }
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
    if(!compress->decoder) {
        CompressConfigHeatshrink* hs_config = (CompressConfigHeatshrink*)compress->config;
        compress->decoder = heatshrink_decoder_alloc(
            hs_config->input_buffer_sz, hs_config->window_sz2, hs_config->lookahead_sz2);
    }
    return compress_decode_internal(
        compress->decoder, data_in, data_in_size, data_out, data_out_size, data_res_size);
}

bool compress_decode_streamed(
    Compress* compress,
    CompressIoCallback read_cb,
    void* read_context,
    CompressIoCallback write_cb,
    void* write_context) {
    CompressConfigHeatshrink* hs_config = (CompressConfigHeatshrink*)compress->config;
    if(!compress->decoder) {
        compress->decoder = heatshrink_decoder_alloc(
            hs_config->input_buffer_sz, hs_config->window_sz2, hs_config->lookahead_sz2);
    }

    heatshrink_decoder_reset(compress->decoder);
    return compress_decode_stream_internal(
        compress->decoder,
        hs_config->input_buffer_sz,
        read_cb,
        read_context,
        write_cb,
        write_context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CompressStreamDecoder {
    heatshrink_decoder* decoder;
    size_t stream_position;
    size_t decode_buffer_size;
    size_t decode_buffer_position;
    uint8_t* decode_buffer;
    CompressIoCallback read_cb;
    void* read_context;
};

CompressStreamDecoder* compress_stream_decoder_alloc(
    CompressType type,
    const void* config,
    CompressIoCallback read_cb,
    void* read_context) {
    furi_check(type == CompressTypeHeatshrink);
    furi_check(config);

    const CompressConfigHeatshrink* hs_config = (const CompressConfigHeatshrink*)config;
    CompressStreamDecoder* instance = malloc(sizeof(CompressStreamDecoder));
    instance->decoder = heatshrink_decoder_alloc(
        hs_config->input_buffer_sz, hs_config->window_sz2, hs_config->lookahead_sz2);
    instance->stream_position = 0;
    instance->decode_buffer_size = hs_config->input_buffer_sz;
    instance->decode_buffer_position = 0;
    instance->decode_buffer = malloc(hs_config->input_buffer_sz);
    instance->read_cb = read_cb;
    instance->read_context = read_context;

    return instance;
}

void compress_stream_decoder_free(CompressStreamDecoder* instance) {
    furi_check(instance);
    heatshrink_decoder_free(instance->decoder);
    free(instance->decode_buffer);
    free(instance);
}

static bool compress_decode_stream_chunk(
    CompressStreamDecoder* sd,
    CompressIoCallback read_cb,
    void* read_context,
    uint8_t* decompressed_chunk,
    size_t decomp_chunk_size) {
    HSD_sink_res sink_res;
    HSD_poll_res poll_res;

    /* 
    First, try to output data from decoder to the output buffer. 
    If the we could fill the output buffer, return
    If the output buffer is not full, keep polling the decoder 
        until it has no more data to output.
    Then, read more data from the input and sink it to the decoder.
    Repeat until the input is exhausted or output buffer is full.
    */

    bool failed = false;
    bool can_sink_more = true;
    bool can_read_more = true;

    do {
        do {
            size_t poll_size = 0;
            poll_res = heatshrink_decoder_poll(
                sd->decoder, decompressed_chunk, decomp_chunk_size, &poll_size);
            if(poll_res < 0) {
                return false;
            }

            decomp_chunk_size -= poll_size;
            decompressed_chunk += poll_size;
        } while((poll_res == HSDR_POLL_MORE) && decomp_chunk_size);

        if(!decomp_chunk_size) {
            break;
        }

        if(can_read_more && (sd->decode_buffer_position < sd->decode_buffer_size)) {
            size_t read_size = read_cb(
                read_context,
                &sd->decode_buffer[sd->decode_buffer_position],
                sd->decode_buffer_size - sd->decode_buffer_position);
            sd->decode_buffer_position += read_size;
            can_read_more = read_size > 0;
        }

        while(sd->decode_buffer_position && can_sink_more) {
            size_t sink_size = 0;
            sink_res = heatshrink_decoder_sink(
                sd->decoder, sd->decode_buffer, sd->decode_buffer_position, &sink_size);
            can_sink_more = sink_res == HSDR_SINK_OK;
            if(sink_res < 0) {
                failed = true;
                break;
            }
            sd->decode_buffer_position -= sink_size;

            /* If some data was left in the buffer, move it to the beginning */
            if(sink_size && sd->decode_buffer_position) {
                memmove(
                    sd->decode_buffer, &sd->decode_buffer[sink_size], sd->decode_buffer_position);
            }
        }
    } while(!failed);

    return decomp_chunk_size == 0;
}

bool compress_stream_decoder_read(
    CompressStreamDecoder* instance,
    uint8_t* data_out,
    size_t data_out_size) {
    furi_check(instance);
    furi_check(data_out);

    if(compress_decode_stream_chunk(
           instance, instance->read_cb, instance->read_context, data_out, data_out_size)) {
        instance->stream_position += data_out_size;
        return true;
    }
    return false;
}

bool compress_stream_decoder_seek(CompressStreamDecoder* instance, size_t position) {
    furi_check(instance);

    /* Check if requested position is ahead of current position 
       we can't rewind the input stream */
    furi_check(position >= instance->stream_position);

    /* Read and discard data up to requested position */
    uint8_t* dummy_buffer = malloc(instance->decode_buffer_size);
    bool success = true;

    while(instance->stream_position < position) {
        size_t bytes_to_read = position - instance->stream_position;
        if(bytes_to_read > instance->decode_buffer_size) {
            bytes_to_read = instance->decode_buffer_size;
        }
        if(!compress_stream_decoder_read(instance, dummy_buffer, bytes_to_read)) {
            success = false;
            break;
        }
    }

    free(dummy_buffer);
    return success;
}

size_t compress_stream_decoder_tell(CompressStreamDecoder* instance) {
    furi_check(instance);
    return instance->stream_position;
}

bool compress_stream_decoder_rewind(CompressStreamDecoder* instance) {
    furi_check(instance);

    /* Reset decoder and read buffer */
    heatshrink_decoder_reset(instance->decoder);
    instance->stream_position = 0;
    instance->decode_buffer_position = 0;

    return true;
}
