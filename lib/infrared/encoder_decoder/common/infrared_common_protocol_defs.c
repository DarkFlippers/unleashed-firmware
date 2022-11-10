#include "infrared_common_i.h"
#include "infrared_protocol_defs_i.h"

const InfraredCommonProtocolSpec protocol_nec = {
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
    .no_stop_bit = false,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_nec_interpret,
    .decode_repeat = infrared_decoder_nec_decode_repeat,
    .encode_repeat = infrared_encoder_nec_encode_repeat,
};

const InfraredCommonProtocolSpec protocol_samsung32 = {
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
    .no_stop_bit = false,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_samsung32_interpret,
    .decode_repeat = infrared_decoder_samsung32_decode_repeat,
    .encode_repeat = infrared_encoder_samsung32_encode_repeat,
};

const InfraredCommonProtocolSpec protocol_rc6 = {
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

const InfraredCommonProtocolSpec protocol_rc5 = {
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

const InfraredCommonProtocolSpec protocol_sirc = {
    .timings =
        {
            .preamble_mark = INFRARED_SIRC_PREAMBLE_MARK,
            .preamble_space = INFRARED_SIRC_PREAMBLE_SPACE,
            .bit1_mark = INFRARED_SIRC_BIT1_MARK,
            .bit1_space = INFRARED_SIRC_BIT1_SPACE,
            .bit0_mark = INFRARED_SIRC_BIT0_MARK,
            .bit0_space = INFRARED_SIRC_BIT0_SPACE,
            .preamble_tolerance = INFRARED_SIRC_PREAMBLE_TOLERANCE,
            .bit_tolerance = INFRARED_SIRC_BIT_TOLERANCE,
            .silence_time = INFRARED_SIRC_SILENCE,
            .min_split_time = INFRARED_SIRC_MIN_SPLIT_TIME,
        },
    .databit_len[0] = 20,
    .databit_len[1] = 15,
    .databit_len[2] = 12,
    .no_stop_bit = true,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_sirc_interpret,
    .decode_repeat = NULL,
    .encode_repeat = infrared_encoder_sirc_encode_repeat,
};

const InfraredCommonProtocolSpec protocol_kaseikyo = {
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
    .no_stop_bit = false,
    .decode = infrared_common_decode_pdwm,
    .encode = infrared_common_encode_pdwm,
    .interpret = infrared_decoder_kaseikyo_interpret,
    .decode_repeat = NULL,
    .encode_repeat = NULL,
};
