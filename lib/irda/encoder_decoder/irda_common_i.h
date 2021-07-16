#pragma once

#include <stdint.h>
#include "irda.h"
#include "irda_i.h"


#define MATCH_BIT_TIMING(x, v, delta)       (  ((x) < (v + delta)) \
                                            && ((x) > (v - delta)))

#define MATCH_PREAMBLE_TIMING(x, v, delta)  (  ((x) < ((v) * (1 + (delta)))) \
                                            && ((x) > ((v) * (1 - (delta)))))

typedef struct IrdaCommonDecoder IrdaCommonDecoder;
typedef struct IrdaCommonEncoder IrdaCommonEncoder;

typedef IrdaStatus (*IrdaCommonDecode)(IrdaCommonDecoder*);
typedef bool (*IrdaCommonInterpret)(IrdaCommonDecoder*);
typedef IrdaStatus (*IrdaCommonEncode)(IrdaCommonEncoder* encoder, uint32_t* out, bool* polarity);

typedef struct {
    IrdaTimings timings;
    bool     manchester_inverse_level;
    uint32_t databit_len;
    IrdaCommonDecode decode;
    IrdaCommonDecode decode_repeat;
    IrdaCommonInterpret interpret;
    IrdaCommonEncode encode;
    IrdaCommonEncode encode_repeat;
} IrdaCommonProtocolSpec;

typedef enum {
    IrdaCommonDecoderStateWaitPreamble,
    IrdaCommonDecoderStateDecode,
    IrdaCommonDecoderStateProcessRepeat,
} IrdaCommonStateDecoder;

typedef enum {
    IrdaCommonEncoderStateSpace,
    IrdaCommonEncoderStatePreamble,
    IrdaCommonEncoderStateEncode,
    IrdaCommonEncoderStateEncodeRepeat,
} IrdaCommonStateEncoder;

struct IrdaCommonDecoder {
    const IrdaCommonProtocolSpec* protocol;
    IrdaCommonStateDecoder state;
    IrdaMessage message;
    uint32_t timings[6];
    uint8_t timings_cnt;
    void* context;
    bool switch_detect;
    uint32_t level;
    uint16_t databit_cnt;
    uint8_t data[];
};

struct IrdaCommonEncoder {
    const IrdaCommonProtocolSpec* protocol;
    IrdaCommonStateEncoder state;
    bool switch_detect;
    uint32_t bits_encoded;
    uint32_t timings_encoded;
    void* context;
    uint8_t data[];
};


static inline void shift_left_array(uint32_t *array, uint32_t len, uint32_t shift) {
    for (int i = 0; i < len; ++i)
        array[i] = array[i + shift];
}


IrdaMessage* irda_common_decode(IrdaCommonDecoder *decoder, bool level, uint32_t duration);
IrdaStatus irda_common_decode_pdwm(IrdaCommonDecoder* decoder);
IrdaStatus irda_common_decode_manchester(IrdaCommonDecoder* decoder);
void irda_common_decoder_set_context(void* decoder, void* context);
void* irda_common_decoder_alloc(const IrdaCommonProtocolSpec *protocol);
void irda_common_decoder_free(void* decoder);
void irda_common_decoder_reset(void* decoder);

IrdaStatus irda_common_encode(IrdaCommonEncoder* encoder, uint32_t* duration, bool* polarity);
IrdaStatus irda_common_encode_pdwm(IrdaCommonEncoder* encoder, uint32_t* duration, bool* polarity);
IrdaStatus irda_common_encode_manchester(IrdaCommonEncoder* encoder, uint32_t* duration, bool* polarity);
void irda_common_encoder_set_context(void* decoder, void* context);
void* irda_common_encoder_alloc(const IrdaCommonProtocolSpec* protocol);
void irda_common_encoder_free(IrdaCommonEncoder* encoder);
void irda_common_encoder_reset(IrdaCommonEncoder* encoder);

