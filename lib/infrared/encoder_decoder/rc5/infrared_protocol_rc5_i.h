#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_RC5_CARRIER_FREQUENCY 36000
#define INFRARED_RC5_DUTY_CYCLE        0.33

#define INFRARED_RC5_PREAMBLE_MARK      0
#define INFRARED_RC5_PREAMBLE_SPACE     0
#define INFRARED_RC5_BIT                888 // half of time-quant for 1 bit
#define INFRARED_RC5_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_RC5_BIT_TOLERANCE      120 // us
/* protocol allows 2700 silence, but it is hard to send 1 message without repeat */
#define INFRARED_RC5_SILENCE            (2700 * 10)
#define INFRARED_RC5_MIN_SPLIT_TIME     2700
#define INFRARED_RC5_REPEAT_COUNT_MIN   1

extern const InfraredCommonProtocolSpec infrared_protocol_rc5;

bool infrared_decoder_rc5_interpret(InfraredCommonDecoder* decoder);
