#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_RC6_CARRIER_FREQUENCY 36000
#define INFRARED_RC6_DUTY_CYCLE        0.33

#define INFRARED_RC6_PREAMBLE_MARK      2666
#define INFRARED_RC6_PREAMBLE_SPACE     889
#define INFRARED_RC6_BIT                444 // half of time-quant for 1 bit
#define INFRARED_RC6_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_RC6_BIT_TOLERANCE      120 // us
/* protocol allows 2700 silence, but it is hard to send 1 message without repeat */
#define INFRARED_RC6_SILENCE            (2700 * 10)
#define INFRARED_RC6_MIN_SPLIT_TIME     2700
#define INFRARED_RC6_REPEAT_COUNT_MIN   1

extern const InfraredCommonProtocolSpec infrared_protocol_rc6;

bool infrared_decoder_rc6_interpret(InfraredCommonDecoder* decoder);
InfraredStatus infrared_decoder_rc6_decode_manchester(
    InfraredCommonDecoder* decoder,
    bool level,
    uint32_t timing);
InfraredStatus infrared_encoder_rc6_encode_manchester(
    InfraredCommonEncoder* encoder_ptr,
    uint32_t* duration,
    bool* polarity);
