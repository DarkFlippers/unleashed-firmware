#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_SIRC_CARRIER_FREQUENCY  40000
#define INFRARED_SIRC_DUTY_CYCLE         0.33
#define INFRARED_SIRC_PREAMBLE_MARK      2400
#define INFRARED_SIRC_PREAMBLE_SPACE     600
#define INFRARED_SIRC_BIT1_MARK          1200
#define INFRARED_SIRC_BIT1_SPACE         600
#define INFRARED_SIRC_BIT0_MARK          600
#define INFRARED_SIRC_BIT0_SPACE         600
#define INFRARED_SIRC_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_SIRC_BIT_TOLERANCE      120 // us
#define INFRARED_SIRC_SILENCE            10000
#define INFRARED_SIRC_MIN_SPLIT_TIME     (INFRARED_SIRC_SILENCE - 1000)
#define INFRARED_SIRC_REPEAT_PERIOD      45000
#define INFRARED_SIRC_REPEAT_COUNT_MIN   3

extern const InfraredCommonProtocolSpec infrared_protocol_sirc;

bool infrared_decoder_sirc_interpret(InfraredCommonDecoder* decoder);
InfraredStatus infrared_encoder_sirc_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level);
