#include "infrared_protocol_rca_i.h"

const InfraredCommonProtocolSpec infrared_protocol_rca = {
    .timings =
        {
            .preamble_mark = INFRARED_RCA_PREAMBLE_MARK,
            .preamble_space = INFRARED_RCA_PREAMBLE_SPACE,
            .bit1_mark = INFRARED_RCA_BIT1_MARK,
            .bit1_space = INFRARED_RCA_BIT1_SPACE,
            .bit0_mark = INFRARED_RCA_BIT0_MARK,
            .bit0_space = INFRARED_RCA_BIT0_SPACE,
            .preamble_tolerance = INFRARED_RCA_PREAMBLE_TOLERANCE,
            .bit_tolerance = INFRARED_RCA_BIT_TOLERANCE,
            .silence_time = INFRARED_RCA_SILENCE,
            .min_split_time = INFRARED_RCA_MIN_SPLIT_TIME,
        },
    .databit_len[0] = 24,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_rca_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};

static const InfraredProtocolVariant infrared_protocol_variant_rca = {
    .name = "RCA",
    .address_length = 4,
    .command_length = 8,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
    .repeat_count = INFRARED_RCA_REPEAT_COUNT_MIN,
};

const InfraredProtocolVariant* infrared_protocol_rca_get_variant(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolRCA)
        return &infrared_protocol_variant_rca;
    else
        return NULL;
}
