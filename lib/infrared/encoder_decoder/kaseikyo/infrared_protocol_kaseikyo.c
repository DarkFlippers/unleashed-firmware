#include "infrared_protocol_kaseikyo_i.h"

const InfraredCommonProtocolSpec infrared_protocol_kaseikyo = {
    .timings =
        {
            .preamble_mark = INFRARED_KASEIKYO_PREAMBLE_MARK,
            .preamble_space = INFRARED_KASEIKYO_PREAMBLE_SPACE,
            .bit1_mark = INFRARED_KASEIKYO_BIT1_MARK,
            .bit1_space = INFRARED_KASEIKYO_BIT1_SPACE,
            .bit0_mark = INFRARED_KASEIKYO_BIT0_MARK,
            .bit0_space = INFRARED_KASEIKYO_BIT0_SPACE,
            .preamble_tolerance = INFRARED_KASEIKYO_PREAMBLE_TOLERANCE,
            .bit_tolerance = INFRARED_KASEIKYO_BIT_TOLERANCE,
            .silence_time = INFRARED_KASEIKYO_SILENCE,
            .min_split_time = INFRARED_KASEIKYO_MIN_SPLIT_TIME,
        },
    .databit_len[0] = 48,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_kaseikyo_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};

static const InfraredProtocolVariant infrared_protocol_variant_kaseikyo = {
    .name = "Kaseikyo",
    .address_length = 26,
    .command_length = 10,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
    .repeat_count = INFRARED_KASEIKYO_REPEAT_COUNT_MIN,
};

const InfraredProtocolVariant* infrared_protocol_kaseikyo_get_variant(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolKaseikyo)
        return &infrared_protocol_variant_kaseikyo;
    else
        return NULL;
}
