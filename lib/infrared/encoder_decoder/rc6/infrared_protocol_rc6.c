#include "infrared_protocol_rc6_i.h"

const InfraredCommonProtocolSpec infrared_protocol_rc6 = {
    .timings =
        {
            .preamble_mark = INFRARED_RC6_PREAMBLE_MARK,
            .preamble_space = INFRARED_RC6_PREAMBLE_SPACE,
            .bit1_mark = INFRARED_RC6_BIT,
            .preamble_tolerance = INFRARED_RC6_PREAMBLE_TOLERANCE,
            .bit_tolerance = INFRARED_RC6_BIT_TOLERANCE,
            .silence_time = INFRARED_RC6_SILENCE,
            .min_split_time = INFRARED_RC6_MIN_SPLIT_TIME,
        },
    .databit_len[0] =
        1 + 3 + 1 + 8 +
        8, // start_bit + 3 mode bits, + 1 toggle bit (x2 timing) + 8 address + 8 command
    .manchester_start_from_space = false,
    .decode = infrared_decoder_rc6_decode_manchester,
    .encode = infrared_encoder_rc6_encode_manchester,
    .interpret = infrared_decoder_rc6_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};

static const InfraredProtocolVariant infrared_protocol_variant_rc6 = {
    .name = "RC6",
    .address_length = 8,
    .command_length = 8,
    .frequency = INFRARED_RC6_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_RC6_DUTY_CYCLE,
    .repeat_count = INFRARED_RC6_REPEAT_COUNT_MIN,
};

const InfraredProtocolVariant* infrared_protocol_rc6_get_variant(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolRC6)
        return &infrared_protocol_variant_rc6;
    else
        return NULL;
}
