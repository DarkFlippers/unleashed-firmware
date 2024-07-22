#pragma once

#include "../common/infrared_common_i.h"

#define INFRARED_KASEIKYO_UNIT               432
#define INFRARED_KASEIKYO_PREAMBLE_MARK      (8 * INFRARED_KASEIKYO_UNIT)
#define INFRARED_KASEIKYO_PREAMBLE_SPACE     (4 * INFRARED_KASEIKYO_UNIT)
#define INFRARED_KASEIKYO_BIT1_MARK          INFRARED_KASEIKYO_UNIT
#define INFRARED_KASEIKYO_BIT1_SPACE         (3 * INFRARED_KASEIKYO_UNIT)
#define INFRARED_KASEIKYO_BIT0_MARK          INFRARED_KASEIKYO_UNIT
#define INFRARED_KASEIKYO_BIT0_SPACE         INFRARED_KASEIKYO_UNIT
#define INFRARED_KASEIKYO_REPEAT_PERIOD      130000
#define INFRARED_KASEIKYO_SILENCE            INFRARED_KASEIKYO_REPEAT_PERIOD
#define INFRARED_KASEIKYO_MIN_SPLIT_TIME     INFRARED_KASEIKYO_REPEAT_PAUSE_MIN
#define INFRARED_KASEIKYO_REPEAT_PAUSE_MIN   4000
#define INFRARED_KASEIKYO_REPEAT_PAUSE_MAX   150000
#define INFRARED_KASEIKYO_REPEAT_COUNT_MIN   1
#define INFRARED_KASEIKYO_REPEAT_MARK        INFRARED_KASEIKYO_PREAMBLE_MARK
#define INFRARED_KASEIKYO_REPEAT_SPACE       (INFRARED_KASEIKYO_REPEAT_PERIOD - 56000)
#define INFRARED_KASEIKYO_PREAMBLE_TOLERANCE 200 // us
#define INFRARED_KASEIKYO_BIT_TOLERANCE      120 // us

extern const InfraredCommonProtocolSpec infrared_protocol_kaseikyo;

bool infrared_decoder_kaseikyo_interpret(InfraredCommonDecoder* decoder);
InfraredStatus infrared_decoder_kaseikyo_decode_repeat(InfraredCommonDecoder* decoder);
InfraredStatus infrared_encoder_kaseikyo_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level);
