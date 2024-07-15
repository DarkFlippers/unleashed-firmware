#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_PIONEER_CARRIER_FREQUENCY  40000
#define INFRARED_PIONEER_DUTY_CYCLE         0.33
#define INFRARED_PIONEER_PREAMBLE_MARK      8500
#define INFRARED_PIONEER_PREAMBLE_SPACE     4225
#define INFRARED_PIONEER_BIT1_MARK          500
#define INFRARED_PIONEER_BIT1_SPACE         1500
#define INFRARED_PIONEER_BIT0_MARK          500
#define INFRARED_PIONEER_BIT0_SPACE         500
#define INFRARED_PIONEER_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_PIONEER_BIT_TOLERANCE      120 // us
#define INFRARED_PIONEER_SILENCE            26000
#define INFRARED_PIONEER_MIN_SPLIT_TIME     (INFRARED_PIONEER_SILENCE)
#define INFRARED_PIONEER_REPEAT_COUNT_MIN   2

extern const InfraredCommonProtocolSpec infrared_protocol_pioneer;

bool infrared_decoder_pioneer_interpret(InfraredCommonDecoder* decoder);
InfraredStatus infrared_encoder_pioneer_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level);
