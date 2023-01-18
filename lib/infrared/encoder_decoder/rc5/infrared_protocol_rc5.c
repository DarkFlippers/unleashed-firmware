#include "infrared_protocol_rc5_i.h"

const InfraredCommonProtocolSpec infrared_protocol_rc5 = {
    .timings =
        {
            .preamble_mark = 0,
            .preamble_space = 0,
            .bit1_mark = INFRARED_RC5_BIT,
            .preamble_tolerance = 0,
            .bit_tolerance = INFRARED_RC5_BIT_TOLERANCE,
            .silence_time = INFRARED_RC5_SILENCE,
            .min_split_time = INFRARED_RC5_MIN_SPLIT_TIME,
        },
    .databit_len[0] = 1 + 1 + 1 + 5 +
                      6, // start_bit + start_bit/command_bit + toggle_bit + 5 address + 6 command
    .manchester_start_from_space = true,
    .decode = infrared_common_decode_manchester,
    .encode = infrared_common_encode_manchester,
    .interpret = infrared_decoder_rc5_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};

static const InfraredProtocolVariant infrared_protocol_variant_rc5 = {
    .name = "RC5",
    .address_length = 5,
    .command_length = 6,
    .frequency = INFRARED_RC5_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_RC5_DUTY_CYCLE,
    .repeat_count = INFRARED_RC5_REPEAT_COUNT_MIN,
};

static const InfraredProtocolVariant infrared_protocol_variant_rc5x = {
    .name = "RC5X",
    .address_length = 5,
    .command_length = 7,
    .frequency = INFRARED_RC5_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_RC5_DUTY_CYCLE,
    .repeat_count = INFRARED_RC5_REPEAT_COUNT_MIN,
};

const InfraredProtocolVariant* infrared_protocol_rc5_get_variant(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolRC5)
        return &infrared_protocol_variant_rc5;
    else if(protocol == InfraredProtocolRC5X)
        return &infrared_protocol_variant_rc5x;
    else
        return NULL;
}
