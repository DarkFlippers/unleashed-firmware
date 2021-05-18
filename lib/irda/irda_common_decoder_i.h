#pragma once

#include <stdint.h>
#include "irda.h"


#define MATCH_BIT_TIMING(x, v, delta)       (  ((x) < (v + delta)) \
                                            && ((x) > (v - delta)))

#define MATCH_PREAMBLE_TIMING(x, v, delta)  (  ((x) < ((v) * (1 + (delta)))) \
                                            && ((x) > ((v) * (1 - (delta)))))

typedef enum {
    DecodeStatusError,
    DecodeStatusOk,
    DecodeStatusReady,
} DecodeStatus;

typedef struct IrdaCommonDecoder IrdaCommonDecoder;

typedef DecodeStatus (*IrdaCommonDecode)(IrdaCommonDecoder*);
typedef bool (*IrdaCommonInterpret)(IrdaCommonDecoder*);
typedef DecodeStatus (*IrdaCommonDecodeRepeat)(IrdaCommonDecoder*);

typedef enum IrdaCommonState {
    IrdaCommonStateWaitPreamble,
    IrdaCommonStateDecode,
    IrdaCommonStateProcessRepeat,
} IrdaCommonState;

typedef struct {
    uint16_t preamble_mark;
    uint16_t preamble_space;
    uint16_t bit1_mark;
    uint16_t bit1_space;
    uint16_t bit0_mark;
    uint16_t bit0_space;
    float    preamble_tolerance;
    uint32_t bit_tolerance;
} IrdaCommonDecoderTimings;

typedef struct {
    IrdaCommonDecoderTimings timings;
    uint32_t databit_len;
    IrdaCommonDecode decode;
    IrdaCommonInterpret interpret;
    IrdaCommonDecodeRepeat decode_repeat;
} IrdaCommonProtocolSpec;

struct IrdaCommonDecoder {
    const IrdaCommonProtocolSpec* protocol;
    IrdaCommonState state;
    IrdaMessage message;
    uint32_t timings[6];
    uint8_t timings_cnt;
    uint32_t level;
    uint16_t databit_cnt;
    uint8_t data[];
};


static inline void shift_left_array(uint32_t *array, uint32_t len, uint32_t shift) {
    for (int i = 0; i < len; ++i)
        array[i] = array[i + shift];
}


IrdaMessage* irda_common_decode(IrdaCommonDecoder *decoder, bool level, uint32_t duration);
void* irda_common_decoder_alloc(const IrdaCommonProtocolSpec *protocol);
void irda_common_decoder_free(void* decoder);
DecodeStatus irda_common_decode_pdwm(IrdaCommonDecoder* decoder);

