#include "infrared_protocol_nec_i.h"

const InfraredCommonProtocolSpec infrared_protocol_nec = {
    .timings =
        {
            .preamble_mark = INFRARED_NEC_PREAMBLE_MARK,
            .preamble_space = INFRARED_NEC_PREAMBLE_SPACE,
            .bit1_mark = INFRARED_NEC_BIT1_MARK,
            .bit1_space = INFRARED_NEC_BIT1_SPACE,
            .bit0_mark = INFRARED_NEC_BIT0_MARK,
            .bit0_space = INFRARED_NEC_BIT0_SPACE,
            .preamble_tolerance = INFRARED_NEC_PREAMBLE_TOLERANCE,
            .bit_tolerance = INFRARED_NEC_BIT_TOLERANCE,
            .silence_time = INFRARED_NEC_SILENCE,
            .min_split_time = INFRARED_NEC_MIN_SPLIT_TIME,
        },
    .databit_len[0] = 42,
    .databit_len[1] = 32,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_nec_interpret,
    .decode_repeat = infrared_decoder_nec_decode_repeat,
    .encode_repeat = infrared_encoder_nec_encode_repeat,
};

static const InfraredProtocolVariant infrared_protocol_variant_nec = {
    .name = "NEC",
    .address_length = 8,
    .command_length = 8,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
    .repeat_count = INFRARED_NEC_REPEAT_COUNT_MIN,
};

static const InfraredProtocolVariant infrared_protocol_variant_necext = {
    .name = "NECext",
    .address_length = 16,
    .command_length = 16,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
    .repeat_count = INFRARED_NEC_REPEAT_COUNT_MIN,
};

static const InfraredProtocolVariant infrared_protocol_variant_nec42 = {
    .name = "NEC42",
    .address_length = 13,
    .command_length = 8,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
    .repeat_count = INFRARED_NEC_REPEAT_COUNT_MIN,
};

static const InfraredProtocolVariant infrared_protocol_variant_nec42ext = {
    .name = "NEC42ext",
    .address_length = 26,
    .command_length = 16,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
    .repeat_count = INFRARED_NEC_REPEAT_COUNT_MIN,
};

const InfraredProtocolVariant* infrared_protocol_nec_get_variant(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolNEC)
        return &infrared_protocol_variant_nec;
    else if(protocol == InfraredProtocolNECext)
        return &infrared_protocol_variant_necext;
    else if(protocol == InfraredProtocolNEC42)
        return &infrared_protocol_variant_nec42;
    else if(protocol == InfraredProtocolNEC42ext)
        return &infrared_protocol_variant_nec42ext;
    else
        return NULL;
}
