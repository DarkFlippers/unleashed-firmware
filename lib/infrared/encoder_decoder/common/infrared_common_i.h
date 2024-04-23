#pragma once

#include <stdint.h>
#include "infrared.h"
#include "infrared_i.h"

#define MATCH_TIMING(x, v, delta) (((x) < ((v) + (delta))) && ((x) > ((v) - (delta))))

typedef struct InfraredCommonDecoder InfraredCommonDecoder;
typedef struct InfraredCommonEncoder InfraredCommonEncoder;

typedef InfraredStatus (*InfraredCommonDecode)(InfraredCommonDecoder*, bool, uint32_t);
typedef InfraredStatus (*InfraredCommonDecodeRepeat)(InfraredCommonDecoder*);
typedef bool (*InfraredCommonInterpret)(InfraredCommonDecoder*);
typedef InfraredStatus (
    *InfraredCommonEncode)(InfraredCommonEncoder* encoder, uint32_t* out, bool* polarity);

typedef struct {
    InfraredTimings timings;
    bool manchester_start_from_space;
    uint8_t databit_len[4];
    InfraredCommonDecode decode;
    InfraredCommonDecodeRepeat decode_repeat;
    InfraredCommonInterpret interpret;
    InfraredCommonEncode encode;
    InfraredCommonEncode encode_repeat;
} InfraredCommonProtocolSpec;

typedef enum {
    InfraredCommonDecoderStateWaitPreamble,
    InfraredCommonDecoderStateDecode,
    InfraredCommonDecoderStateProcessRepeat,
} InfraredCommonStateDecoder;

typedef enum {
    InfraredCommonEncoderStateSilence,
    InfraredCommonEncoderStatePreamble,
    InfraredCommonEncoderStateEncode,
    InfraredCommonEncoderStateEncodeRepeat,
} InfraredCommonStateEncoder;

struct InfraredCommonDecoder {
    const InfraredCommonProtocolSpec* protocol;
    void* context;
    uint32_t timings[6];
    InfraredMessage message;
    InfraredCommonStateDecoder state;
    uint8_t timings_cnt;
    bool switch_detect;
    bool level;
    uint16_t databit_cnt;
    uint8_t data[];
};

struct InfraredCommonEncoder {
    const InfraredCommonProtocolSpec* protocol;
    InfraredCommonStateEncoder state;
    bool switch_detect;
    uint8_t bits_to_encode;
    uint8_t bits_encoded;
    uint32_t timings_sum;
    uint32_t timings_encoded;
    void* context;
    uint8_t data[];
};

InfraredMessage*
    infrared_common_decode(InfraredCommonDecoder* decoder, bool level, uint32_t duration);
InfraredStatus
    infrared_common_decode_pdwm(InfraredCommonDecoder* decoder, bool level, uint32_t timing);
InfraredStatus
    infrared_common_decode_manchester(InfraredCommonDecoder* decoder, bool level, uint32_t timing);
void* infrared_common_decoder_alloc(const InfraredCommonProtocolSpec* protocol);
void infrared_common_decoder_free(InfraredCommonDecoder* decoder);
void infrared_common_decoder_reset(InfraredCommonDecoder* decoder);
InfraredMessage* infrared_common_decoder_check_ready(InfraredCommonDecoder* decoder);

InfraredStatus
    infrared_common_encode(InfraredCommonEncoder* encoder, uint32_t* duration, bool* polarity);
InfraredStatus
    infrared_common_encode_pdwm(InfraredCommonEncoder* encoder, uint32_t* duration, bool* polarity);
InfraredStatus infrared_common_encode_manchester(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* polarity);
void* infrared_common_encoder_alloc(const InfraredCommonProtocolSpec* protocol);
void infrared_common_encoder_free(InfraredCommonEncoder* encoder);
void infrared_common_encoder_reset(InfraredCommonEncoder* encoder);
