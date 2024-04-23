#include "infrared_protocol_pioneer_i.h"

const InfraredCommonProtocolSpec infrared_protocol_pioneer = {
    .timings =
        {
            .preamble_mark = INFRARED_PIONEER_PREAMBLE_MARK,
            .preamble_space = INFRARED_PIONEER_PREAMBLE_SPACE,
            .bit1_mark = INFRARED_PIONEER_BIT1_MARK,
            .bit1_space = INFRARED_PIONEER_BIT1_SPACE,
            .bit0_mark = INFRARED_PIONEER_BIT0_MARK,
            .bit0_space = INFRARED_PIONEER_BIT0_SPACE,
            .preamble_tolerance = INFRARED_PIONEER_PREAMBLE_TOLERANCE,
            .bit_tolerance = INFRARED_PIONEER_BIT_TOLERANCE,
            .silence_time = INFRARED_PIONEER_SILENCE,
            .min_split_time = INFRARED_PIONEER_MIN_SPLIT_TIME,
        },
    .databit_len[0] = 33,
    .databit_len[1] = 32,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_pioneer_interpret,
    .decode_repeat = NULL,
    .encode_repeat = infrared_encoder_pioneer_encode_repeat,
};

static const InfraredProtocolVariant infrared_protocol_variant_pioneer = {
    .name = "Pioneer",
    .address_length = 8,
    .command_length = 8,
    .frequency = INFRARED_PIONEER_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_PIONEER_DUTY_CYCLE,
    .repeat_count = INFRARED_PIONEER_REPEAT_COUNT_MIN,
};

const InfraredProtocolVariant* infrared_protocol_pioneer_get_variant(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolPioneer)
        return &infrared_protocol_variant_pioneer;
    else
        return NULL;
}
