#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_RCA_PREAMBLE_MARK  4000
#define INFRARED_RCA_PREAMBLE_SPACE 4000
#define INFRARED_RCA_BIT1_MARK      500
#define INFRARED_RCA_BIT1_SPACE     2000
#define INFRARED_RCA_BIT0_MARK      500
#define INFRARED_RCA_BIT0_SPACE     1000
#define INFRARED_RCA_REPEAT_PERIOD  8000
#define INFRARED_RCA_SILENCE        INFRARED_RCA_REPEAT_PERIOD

#define INFRARED_RCA_MIN_SPLIT_TIME     INFRARED_RCA_REPEAT_PAUSE_MIN
#define INFRARED_RCA_REPEAT_PAUSE_MIN   4000
#define INFRARED_RCA_REPEAT_PAUSE_MAX   150000
#define INFRARED_RCA_REPEAT_COUNT_MIN   1
#define INFRARED_RCA_REPEAT_MARK        INFRARED_RCA_PREAMBLE_MARK
#define INFRARED_RCA_REPEAT_SPACE       INFRARED_RCA_PREAMBLE_SPACE
#define INFRARED_RCA_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_RCA_BIT_TOLERANCE      120 // us

extern const InfraredCommonProtocolSpec infrared_protocol_rca;

bool infrared_decoder_rca_interpret(InfraredCommonDecoder* decoder);
InfraredStatus infrared_decoder_rca_decode_repeat(InfraredCommonDecoder* decoder);
InfraredStatus infrared_encoder_rca_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level);
