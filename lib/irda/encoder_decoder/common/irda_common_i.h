#pragma once

#include <stdint.h>
#include "irda.h"
#include "irda_i.h"


#define MATCH_TIMING(x, v, delta)       (  ((x) < (v + delta)) \
                                        && ((x) > (v - delta)))

typedef struct IrdaCommonDecoder IrdaCommonDecoder;
typedef struct IrdaCommonEncoder IrdaCommonEncoder;

typedef IrdaStatus (*IrdaCommonDecode)(IrdaCommonDecoder*, bool, uint32_t);
typedef IrdaStatus (*IrdaCommonDecodeRepeat)(IrdaCommonDecoder*);
typedef bool (*IrdaCommonInterpret)(IrdaCommonDecoder*);
typedef IrdaStatus (*IrdaCommonEncode)(IrdaCommonEncoder* encoder, uint32_t* out, bool* polarity);

typedef struct {
    IrdaTimings timings;
    bool     manchester_start_from_space;
    bool     no_stop_bit;
    uint8_t  databit_len[4];
    IrdaCommonDecode decode;
    IrdaCommonDecodeRepeat decode_repeat;
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
    IrdaCommonEncoderStateSilence,
    IrdaCommonEncoderStatePreamble,
    IrdaCommonEncoderStateEncode,
    IrdaCommonEncoderStateEncodeRepeat,
} IrdaCommonStateEncoder;

struct IrdaCommonDecoder {
    const IrdaCommonProtocolSpec* protocol;
    void* context;
    uint32_t timings[6];
    IrdaMessage message;
    IrdaCommonStateDecoder state;
    uint8_t timings_cnt;
    bool switch_detect;
    bool level;
    uint16_t databit_cnt;
    uint8_t data[];
};

struct IrdaCommonEncoder {
    const IrdaCommonProtocolSpec* protocol;
    IrdaCommonStateEncoder state;
    bool switch_detect;
    uint8_t bits_to_encode;
    uint8_t bits_encoded;
    uint32_t timings_sum;
    uint32_t timings_encoded;
    void* context;
    uint8_t data[];
};

IrdaMessage* irda_common_decode(IrdaCommonDecoder *decoder, bool level, uint32_t duration);
IrdaStatus irda_common_decode_pdwm(IrdaCommonDecoder* decoder, bool level, uint32_t timing);
IrdaStatus irda_common_decode_manchester(IrdaCommonDecoder* decoder, bool level, uint32_t timing);
void* irda_common_decoder_alloc(const IrdaCommonProtocolSpec *protocol);
void irda_common_decoder_free(IrdaCommonDecoder* decoder);
void irda_common_decoder_reset(IrdaCommonDecoder* decoder);
IrdaMessage* irda_common_decoder_check_ready(IrdaCommonDecoder* decoder);

IrdaStatus irda_common_encode(IrdaCommonEncoder* encoder, uint32_t* duration, bool* polarity);
IrdaStatus irda_common_encode_pdwm(IrdaCommonEncoder* encoder, uint32_t* duration, bool* polarity);
IrdaStatus irda_common_encode_manchester(IrdaCommonEncoder* encoder, uint32_t* duration, bool* polarity);
void* irda_common_encoder_alloc(const IrdaCommonProtocolSpec* protocol);
void irda_common_encoder_free(IrdaCommonEncoder* encoder);
void irda_common_encoder_reset(IrdaCommonEncoder* encoder);

