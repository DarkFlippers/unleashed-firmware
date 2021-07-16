#include "irda_common_i.h"
#include "irda_protocol_defs_i.h"

const IrdaCommonProtocolSpec protocol_nec = {
    .timings = {
        .preamble_mark = IRDA_NEC_PREAMBULE_MARK,
        .preamble_space = IRDA_NEC_PREAMBULE_SPACE,
        .bit1_mark = IRDA_NEC_BIT1_MARK,
        .bit1_space = IRDA_NEC_BIT1_SPACE,
        .bit0_mark = IRDA_NEC_BIT0_MARK,
        .bit0_space = IRDA_NEC_BIT0_SPACE,
        .preamble_tolerance = IRDA_NEC_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_NEC_BIT_TOLERANCE,
        .silence_time = IRDA_NEC_SILENCE,
    },
    .databit_len = 32,
    .decode = irda_common_decode_pdwm,
    .encode = irda_common_encode_pdwm,
    .interpret = irda_decoder_nec_interpret,
    .decode_repeat = irda_decoder_nec_decode_repeat,
    .encode_repeat = irda_encoder_nec_encode_repeat,
};

const IrdaCommonProtocolSpec protocol_necext = {
    .timings = {
        .preamble_mark = IRDA_NEC_PREAMBULE_MARK,
        .preamble_space = IRDA_NEC_PREAMBULE_SPACE,
        .bit1_mark = IRDA_NEC_BIT1_MARK,
        .bit1_space = IRDA_NEC_BIT1_SPACE,
        .bit0_mark = IRDA_NEC_BIT0_MARK,
        .bit0_space = IRDA_NEC_BIT0_SPACE,
        .preamble_tolerance = IRDA_NEC_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_NEC_BIT_TOLERANCE,
        .silence_time = IRDA_NEC_SILENCE,
    },
    .databit_len = 32,
    .decode = irda_common_decode_pdwm,
    .encode = irda_common_encode_pdwm,
    .interpret = irda_decoder_necext_interpret,
    .decode_repeat = irda_decoder_nec_decode_repeat,
    .encode_repeat = irda_encoder_nec_encode_repeat,
};

const IrdaCommonProtocolSpec protocol_samsung32 = {
    .timings = {
        .preamble_mark = IRDA_SAMSUNG_PREAMBULE_MARK,
        .preamble_space = IRDA_SAMSUNG_PREAMBULE_SPACE,
        .bit1_mark = IRDA_SAMSUNG_BIT1_MARK,
        .bit1_space = IRDA_SAMSUNG_BIT1_SPACE,
        .bit0_mark = IRDA_SAMSUNG_BIT0_MARK,
        .bit0_space = IRDA_SAMSUNG_BIT0_SPACE,
        .preamble_tolerance = IRDA_SAMSUNG_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_SAMSUNG_BIT_TOLERANCE,
        .silence_time = IRDA_SAMSUNG_SILENCE,
    },
    .databit_len = 32,
    .decode = irda_common_decode_pdwm,
    .encode = irda_common_encode_pdwm,
    .interpret = irda_decoder_samsung32_interpret,
    .decode_repeat = irda_decoder_samsung32_decode_repeat,
    .encode_repeat = irda_encoder_samsung32_encode_repeat,
};

const IrdaCommonProtocolSpec protocol_rc6 = {
    .timings = {
        .preamble_mark = IRDA_RC6_PREAMBULE_MARK,
        .preamble_space = IRDA_RC6_PREAMBULE_SPACE,
        .bit1_mark = IRDA_RC6_BIT,
        .preamble_tolerance = IRDA_RC6_PREAMBLE_TOLERANCE,
        .bit_tolerance = IRDA_RC6_BIT_TOLERANCE,
        .silence_time = IRDA_RC6_SILENCE,
    },
    .databit_len = 1 + 3 + 1 + 8 + 8,   // start_bit + 3 mode bits, + 1 toggle bit (x2 timing) + 8 address + 8 command
    .manchester_inverse_level = false,
    .decode = irda_decoder_rc6_decode_manchester,
    .encode = irda_encoder_rc6_encode_manchester,
    .interpret = irda_decoder_rc6_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};

