#include "irda_common_i.h"
#include "irda_protocol_defs_i.h"

const IrdaCommonProtocolSpec protocol_nec = {
    .timings = {
        .preamble_mark = IRDA_NEC_PREAMBLE_MARK,
        .preamble_space = IRDA_NEC_PREAMBLE_SPACE,
        .bit1_mark = IRDA_NEC_BIT1_MARK,
        .bit1_space = IRDA_NEC_BIT1_SPACE,
        .bit0_mark = IRDA_NEC_BIT0_MARK,
        .bit0_space = IRDA_NEC_BIT0_SPACE,
        .preamble_tolerance = IRDA_NEC_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_NEC_BIT_TOLERANCE,
        .silence_time = IRDA_NEC_SILENCE,
        .min_split_time = IRDA_NEC_MIN_SPLIT_TIME,
    },
    .databit_len[0] = 42,
    .databit_len[1] = 32,
    .no_stop_bit = false,
    .decode = irda_common_decode_pdwm,
    .encode = irda_common_encode_pdwm,
    .interpret = irda_decoder_nec_interpret,
    .decode_repeat = irda_decoder_nec_decode_repeat,
    .encode_repeat = irda_encoder_nec_encode_repeat,
};

const IrdaCommonProtocolSpec protocol_samsung32 = {
    .timings = {
        .preamble_mark = IRDA_SAMSUNG_PREAMBLE_MARK,
        .preamble_space = IRDA_SAMSUNG_PREAMBLE_SPACE,
        .bit1_mark = IRDA_SAMSUNG_BIT1_MARK,
        .bit1_space = IRDA_SAMSUNG_BIT1_SPACE,
        .bit0_mark = IRDA_SAMSUNG_BIT0_MARK,
        .bit0_space = IRDA_SAMSUNG_BIT0_SPACE,
        .preamble_tolerance = IRDA_SAMSUNG_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_SAMSUNG_BIT_TOLERANCE,
        .silence_time = IRDA_SAMSUNG_SILENCE,
    },
    .databit_len[0] = 32,
    .no_stop_bit = false,
    .decode = irda_common_decode_pdwm,
    .encode = irda_common_encode_pdwm,
    .interpret = irda_decoder_samsung32_interpret,
    .decode_repeat = irda_decoder_samsung32_decode_repeat,
    .encode_repeat = irda_encoder_samsung32_encode_repeat,
};

const IrdaCommonProtocolSpec protocol_rc6 = {
    .timings = {
        .preamble_mark = IRDA_RC6_PREAMBLE_MARK,
        .preamble_space = IRDA_RC6_PREAMBLE_SPACE,
        .bit1_mark = IRDA_RC6_BIT,
        .preamble_tolerance = IRDA_RC6_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_RC6_BIT_TOLERANCE,
        .silence_time = IRDA_RC6_SILENCE,
    },
    .databit_len[0] = 1 + 3 + 1 + 8 + 8,   // start_bit + 3 mode bits, + 1 toggle bit (x2 timing) + 8 address + 8 command
    .manchester_start_from_space = false,
    .decode = irda_decoder_rc6_decode_manchester,
    .encode = irda_encoder_rc6_encode_manchester,
    .interpret = irda_decoder_rc6_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};

const IrdaCommonProtocolSpec protocol_rc5 = {
    .timings = {
        .preamble_mark = 0,
        .preamble_space = 0,
        .bit1_mark = IRDA_RC5_BIT,
        .preamble_tolerance = 0,
        .bit_tolerance = IRDA_RC5_BIT_TOLERANCE,
        .silence_time = IRDA_RC5_SILENCE,
    },
    .databit_len[0] = 1 + 1 + 1 + 5 + 6,   // start_bit + start_bit/command_bit + toggle_bit + 5 address + 6 command
    .manchester_start_from_space = true,
    .decode = irda_common_decode_manchester,
    .encode = irda_common_encode_manchester,
    .interpret = irda_decoder_rc5_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};

const IrdaCommonProtocolSpec protocol_sirc = {
    .timings = {
        .preamble_mark = IRDA_SIRC_PREAMBLE_MARK,
        .preamble_space = IRDA_SIRC_PREAMBLE_SPACE,
        .bit1_mark = IRDA_SIRC_BIT1_MARK,
        .bit1_space = IRDA_SIRC_BIT1_SPACE,
        .bit0_mark = IRDA_SIRC_BIT0_MARK,
        .bit0_space = IRDA_SIRC_BIT0_SPACE,
        .preamble_tolerance = IRDA_SIRC_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_SIRC_BIT_TOLERANCE,
        .silence_time = IRDA_SIRC_SILENCE,
        .min_split_time = IRDA_SIRC_MIN_SPLIT_TIME,
    },
    .databit_len[0] = 20,
    .databit_len[1] = 15,
    .databit_len[2] = 12,
    .no_stop_bit = true,
    .decode = irda_common_decode_pdwm,
    .encode = irda_common_encode_pdwm,
    .interpret = irda_decoder_sirc_interpret,
    .decode_repeat = NULL,
    .encode_repeat = irda_encoder_sirc_encode_repeat,
};

