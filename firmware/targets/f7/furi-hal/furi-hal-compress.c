#include <furi-hal-compress.h>

#include <furi.h>
#include <lib/heatshrink/heatshrink_encoder.h>
#include <lib/heatshrink/heatshrink_decoder.h>

#define TAG "FuriHalCompress"

#define FURI_HAL_COMPRESS_ICON_ENCODED_BUFF_SIZE (2*512)
#define FURI_HAL_COMPRESS_ICON_DECODED_BUFF_SIZE (1024)

#define FURI_HAL_COMPRESS_EXP_BUFF_SIZE (1 << FURI_HAL_COMPRESS_EXP_BUFF_SIZE_LOG)

typedef struct {
    uint8_t is_compressed;
    uint8_t reserved;
    uint16_t compressed_buff_size;
} FuriHalCompressHeader;

typedef struct {
    heatshrink_decoder* decoder;
    uint8_t compress_buff[FURI_HAL_COMPRESS_EXP_BUFF_SIZE + FURI_HAL_COMPRESS_ICON_ENCODED_BUFF_SIZE];
    uint8_t decoded_buff[FURI_HAL_COMPRESS_ICON_DECODED_BUFF_SIZE];
} FuriHalCompressIcon;

struct FuriHalCompress {
    heatshrink_encoder* encoder;
    heatshrink_decoder* decoder;
    uint8_t *compress_buff;
    uint16_t compress_buff_size;
};

static FuriHalCompressIcon* icon_decoder;

static void furi_hal_compress_reset(FuriHalCompress* compress) {
    furi_assert(compress);
    heatshrink_encoder_reset(compress->encoder);
    heatshrink_decoder_reset(compress->decoder);
    memset(compress->compress_buff, 0, compress->compress_buff_size);
}

void furi_hal_compress_icon_init() {
    icon_decoder = furi_alloc(sizeof(FuriHalCompressIcon));
    icon_decoder->decoder = heatshrink_decoder_alloc(
        icon_decoder->compress_buff,
        FURI_HAL_COMPRESS_ICON_ENCODED_BUFF_SIZE,
        FURI_HAL_COMPRESS_EXP_BUFF_SIZE_LOG,
        FURI_HAL_COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG);
    heatshrink_decoder_reset(icon_decoder->decoder);
    memset(icon_decoder->decoded_buff, 0, sizeof(icon_decoder->decoded_buff));
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_compress_icon_decode(const uint8_t* icon_data, uint8_t** decoded_buff) { 
    furi_assert(icon_data);
    furi_assert(decoded_buff);

    FuriHalCompressHeader* header = (FuriHalCompressHeader*) icon_data;
    if(header->is_compressed) {
        size_t data_processed = 0;
        heatshrink_decoder_sink(icon_decoder->decoder, (uint8_t*)&icon_data[4], header->compressed_buff_size, &data_processed);
        while (1) {
            HSD_poll_res res = heatshrink_decoder_poll(
                icon_decoder->decoder,
                icon_decoder->decoded_buff,
                sizeof(icon_decoder->decoded_buff),
                &data_processed);
            furi_assert((res == HSDR_POLL_EMPTY) || (res == HSDR_POLL_MORE));
            if (res != HSDR_POLL_MORE) {
                break;
            }
        }
        heatshrink_decoder_reset(icon_decoder->decoder);
        memset(icon_decoder->compress_buff, 0, sizeof(icon_decoder->compress_buff));
        *decoded_buff = icon_decoder->decoded_buff;
    } else {
        *decoded_buff = (uint8_t*)&icon_data[1];
    }
}

FuriHalCompress* furi_hal_compress_alloc(uint16_t compress_buff_size) {
    FuriHalCompress* compress = furi_alloc(sizeof(FuriHalCompress));
    compress->compress_buff = furi_alloc(compress_buff_size + FURI_HAL_COMPRESS_EXP_BUFF_SIZE);
    compress->encoder = heatshrink_encoder_alloc(compress->compress_buff, FURI_HAL_COMPRESS_EXP_BUFF_SIZE_LOG, FURI_HAL_COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG);
    compress->decoder = heatshrink_decoder_alloc(compress->compress_buff, compress_buff_size, FURI_HAL_COMPRESS_EXP_BUFF_SIZE_LOG, FURI_HAL_COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG);

    return compress;
}

void furi_hal_compress_free(FuriHalCompress* compress) {
    furi_assert(compress);

    heatshrink_encoder_free(compress->encoder);
    heatshrink_decoder_free(compress->decoder);
    free(compress->compress_buff);
    free(compress);
}

bool furi_hal_compress_encode(FuriHalCompress* compress, uint8_t* data_in, size_t data_in_size, uint8_t* data_out, size_t data_out_size, size_t* data_res_size) {
    furi_assert(compress);
    furi_assert(data_in);
    furi_assert(data_in_size);

    size_t sink_size = 0;
    size_t poll_size = 0;
    HSE_sink_res sink_res;
    HSE_poll_res poll_res;
    HSE_finish_res finish_res;
    bool encode_failed = false;
    size_t sunk = 0;
    size_t res_buff_size = sizeof(FuriHalCompressHeader);

    // Sink data to encoding buffer
    while((sunk < data_in_size) && !encode_failed) {
        sink_res = heatshrink_encoder_sink(compress->encoder, &data_in[sunk], data_in_size - sunk, &sink_size);
        if(sink_res != HSER_SINK_OK) {
            encode_failed = true;
            break;
        }
        sunk += sink_size;
        do {
            poll_res = heatshrink_encoder_poll(compress->encoder, &data_out[res_buff_size], data_out_size - res_buff_size, &poll_size);
            if(poll_res < 0) {
                encode_failed = true;
                break;
            }
            res_buff_size += poll_size;
        } while(poll_res == HSER_POLL_MORE);
    }

    // Notify sinking complete and poll encoded data
    finish_res = heatshrink_encoder_finish(compress->encoder);
    if(finish_res < 0) {
        encode_failed = true;
    } else {
        do {
            poll_res = heatshrink_encoder_poll(compress->encoder, &data_out[res_buff_size], data_out_size - 4 - res_buff_size, &poll_size);
            if(poll_res < 0) {
                encode_failed = true;
                break;
            }
            res_buff_size += poll_size;
            finish_res = heatshrink_encoder_finish(compress->encoder);
        } while(finish_res != HSER_FINISH_DONE);
    }

    bool result = true;
    // Write encoded data to output buffer if compression is efficient. Else - write header and original data
    if(!encode_failed && (res_buff_size < data_in_size + 1)) {
        FuriHalCompressHeader header = {.is_compressed = 0x01, .reserved = 0x00, .compressed_buff_size = res_buff_size};
        memcpy(data_out, &header, sizeof(header));
        *data_res_size = res_buff_size;
    } else if (data_out_size > data_in_size) {
        data_out[0] = 0x00;
        memcpy(&data_out[1], data_in, data_in_size);
        *data_res_size = data_in_size + 1;
    } else {
        *data_res_size = 0;
        result = false;
    }
    furi_hal_compress_reset(compress);

    return result;
}

bool furi_hal_compress_decode(FuriHalCompress* compress, uint8_t* data_in, size_t data_in_size, uint8_t* data_out, size_t data_out_size, size_t* data_res_size) {
    furi_assert(compress);
    furi_assert(data_in);
    furi_assert(data_out);
    furi_assert(data_res_size);

    bool result = false;
    bool decode_failed = false;
    HSD_sink_res sink_res;
    HSD_poll_res poll_res;
    HSD_finish_res finish_res;
    size_t sink_size = 0;
    size_t res_buff_size = 0;
    size_t poll_size = 0;

    FuriHalCompressHeader* header = (FuriHalCompressHeader*) data_in;
    if(header->is_compressed) {
        // Sink data to decoding buffer
        size_t compressed_size = header->compressed_buff_size;
        size_t sunk = sizeof(FuriHalCompressHeader);
        while(sunk < compressed_size && !decode_failed) {
            sink_res = heatshrink_decoder_sink(compress->decoder, &data_in[sunk], compressed_size - sunk, &sink_size);
            if(sink_res < 0) {
                decode_failed = true;
                break;
            }
            sunk += sink_size;
            do {
                poll_res = heatshrink_decoder_poll(compress->decoder, &data_out[res_buff_size], data_out_size, &poll_size);
                if(poll_res < 0) {
                    decode_failed = true;
                    break;
                }
                res_buff_size += poll_size;
            } while(poll_res == HSDR_POLL_MORE);
        }
        // Notify sinking complete and poll decoded data
        if(!decode_failed) {
            finish_res = heatshrink_decoder_finish(compress->decoder);
            if(finish_res < 0) {
                decode_failed = true;
            } else {
                do {
                    poll_res = heatshrink_decoder_poll(compress->decoder, &data_out[res_buff_size], data_out_size, &poll_size);
                    res_buff_size += poll_size;
                    finish_res = heatshrink_decoder_finish(compress->decoder);
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
        result = false;
    }
    furi_hal_compress_reset(compress);
    
    return result;
}
