#include "iso14443_4a_i.h"

bool iso14443_4a_ats_parse(Iso14443_4aAtsData* data, const BitBuffer* buf) {
    bool can_parse = false;

    do {
        const size_t buf_size = bit_buffer_get_size_bytes(buf);
        if(buf_size == 0) break;

        size_t current_index = 0;

        const uint8_t tl = bit_buffer_get_byte(buf, current_index++);
        if(tl != buf_size) break;

        data->tl = tl;

        if(tl > 1) {
            const uint8_t t0 = bit_buffer_get_byte(buf, current_index++);

            const bool has_ta_1 = t0 & ISO14443_4A_ATS_T0_TA1;
            const bool has_tb_1 = t0 & ISO14443_4A_ATS_T0_TB1;
            const bool has_tc_1 = t0 & ISO14443_4A_ATS_T0_TC1;

            const uint8_t buf_size_min =
                2 + (has_ta_1 ? 1 : 0) + (has_tb_1 ? 1 : 0) + (has_tc_1 ? 1 : 0);

            if(buf_size < buf_size_min) break;

            data->t0 = t0;

            if(has_ta_1) {
                data->ta_1 = bit_buffer_get_byte(buf, current_index++);
            }
            if(has_tb_1) {
                data->tb_1 = bit_buffer_get_byte(buf, current_index++);
            }
            if(has_tc_1) {
                data->tc_1 = bit_buffer_get_byte(buf, current_index++);
            }

            const uint8_t t1_tk_size = buf_size - buf_size_min;

            if(t1_tk_size > 0) {
                simple_array_init(data->t1_tk, t1_tk_size);
                bit_buffer_write_bytes_mid(
                    buf, simple_array_get_data(data->t1_tk), current_index, t1_tk_size);
            }
        }

        can_parse = true;
    } while(false);

    return can_parse;
}

Iso14443_4aError iso14443_4a_process_error(Iso14443_3aError error) {
    switch(error) {
    case Iso14443_3aErrorNone:
        return Iso14443_4aErrorNone;
    case Iso14443_3aErrorNotPresent:
        return Iso14443_4aErrorNotPresent;
    case Iso14443_3aErrorColResFailed:
    case Iso14443_3aErrorCommunication:
    case Iso14443_3aErrorWrongCrc:
        return Iso14443_4aErrorProtocol;
    case Iso14443_3aErrorTimeout:
        return Iso14443_4aErrorTimeout;
    default:
        return Iso14443_4aErrorProtocol;
    }
}
