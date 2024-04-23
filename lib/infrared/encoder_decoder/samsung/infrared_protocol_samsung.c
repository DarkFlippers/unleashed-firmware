#include "infrared_protocol_samsung_i.h"

const InfraredCommonProtocolSpec infrared_protocol_samsung32 = {
    .timings =
        {
            .preamble_mark = INFRARED_SAMSUNG_PREAMBLE_MARK,
            .preamble_space = INFRARED_SAMSUNG_PREAMBLE_SPACE,
            .bit1_mark = INFRARED_SAMSUNG_BIT1_MARK,
            .bit1_space = INFRARED_SAMSUNG_BIT1_SPACE,
            .bit0_mark = INFRARED_SAMSUNG_BIT0_MARK,
            .bit0_space = INFRARED_SAMSUNG_BIT0_SPACE,
            .preamble_tolerance = INFRARED_SAMSUNG_PREAMBLE_TOLERANCE,
            .bit_tolerance = INFRARED_SAMSUNG_BIT_TOLERANCE,
            .silence_time = INFRARED_SAMSUNG_SILENCE,
            .min_split_time = INFRARED_SAMSUNG_MIN_SPLIT_TIME,
        },
    .databit_len[0] = 32,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_samsung32_interpret,
    .decode_repeat = infrared_decoder_samsung32_decode_repeat,
    .encode_repeat = infrared_encoder_samsung32_encode_repeat,
};

static const InfraredProtocolVariant infrared_protocol_variant_samsung32 = {
    .name = "Samsung32",
    .address_length = 8,
    .command_length = 8,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
    .repeat_count = INFRARED_SAMSUNG_REPEAT_COUNT_MIN,
};

const InfraredProtocolVariant* infrared_protocol_samsung32_get_variant(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolSamsung32)
        return &infrared_protocol_variant_samsung32;
    else
        return NULL;
}
