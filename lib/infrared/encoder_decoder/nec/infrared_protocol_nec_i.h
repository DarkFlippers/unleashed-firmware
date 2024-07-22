#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_NEC_PREAMBLE_MARK      9000
#define INFRARED_NEC_PREAMBLE_SPACE     4500
#define INFRARED_NEC_BIT1_MARK          560
#define INFRARED_NEC_BIT1_SPACE         1690
#define INFRARED_NEC_BIT0_MARK          560
#define INFRARED_NEC_BIT0_SPACE         560
#define INFRARED_NEC_REPEAT_PERIOD      110000
#define INFRARED_NEC_SILENCE            INFRARED_NEC_REPEAT_PERIOD
#define INFRARED_NEC_MIN_SPLIT_TIME     INFRARED_NEC_REPEAT_PAUSE_MIN
#define INFRARED_NEC_REPEAT_PAUSE_MIN   4000
#define INFRARED_NEC_REPEAT_PAUSE_MAX   150000
#define INFRARED_NEC_REPEAT_COUNT_MIN   1
#define INFRARED_NEC_REPEAT_MARK        9000
#define INFRARED_NEC_REPEAT_SPACE       2250
#define INFRARED_NEC_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_NEC_BIT_TOLERANCE      120 // us

extern const InfraredCommonProtocolSpec infrared_protocol_nec;

bool infrared_decoder_nec_interpret(InfraredCommonDecoder* decoder);
InfraredStatus infrared_decoder_nec_decode_repeat(InfraredCommonDecoder* decoder);
InfraredStatus infrared_encoder_nec_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level);
