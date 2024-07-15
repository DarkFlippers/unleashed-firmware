#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_SAMSUNG_PREAMBLE_MARK      4500
#define INFRARED_SAMSUNG_PREAMBLE_SPACE     4500
#define INFRARED_SAMSUNG_BIT1_MARK          550
#define INFRARED_SAMSUNG_BIT1_SPACE         1650
#define INFRARED_SAMSUNG_BIT0_MARK          550
#define INFRARED_SAMSUNG_BIT0_SPACE         550
#define INFRARED_SAMSUNG_REPEAT_PAUSE_MIN   30000
#define INFRARED_SAMSUNG_REPEAT_PAUSE_MAX   140000
#define INFRARED_SAMSUNG_REPEAT_PAUSE1      46000
#define INFRARED_SAMSUNG_REPEAT_PAUSE2      97000
#define INFRARED_SAMSUNG_REPEAT_COUNT_MIN   1
/* Samsung silence have to be greater than REPEAT MAX
 * otherwise there can be problems during unit tests parsing
 * of some data. Real tolerances we don't know, but in real life
 * silence time should be greater than max repeat time. This is
 * because of similar preambule timings for repeat and first messages. */
#define INFRARED_SAMSUNG_MIN_SPLIT_TIME     5000
#define INFRARED_SAMSUNG_SILENCE            145000
#define INFRARED_SAMSUNG_REPEAT_MARK        4500
#define INFRARED_SAMSUNG_REPEAT_SPACE       4500
#define INFRARED_SAMSUNG_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_SAMSUNG_BIT_TOLERANCE      120 // us

bool infrared_decoder_samsung32_interpret(InfraredCommonDecoder* decoder);
InfraredStatus infrared_decoder_samsung32_decode_repeat(InfraredCommonDecoder* decoder);
InfraredStatus infrared_encoder_samsung32_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level);

extern const InfraredCommonProtocolSpec infrared_protocol_samsung32;
