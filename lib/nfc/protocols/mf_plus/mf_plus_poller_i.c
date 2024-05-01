#include "mf_plus_poller_i.h"

#include <furi.h>

#include "mf_plus_i.h"

#define TAG "MfPlusPoller"

MfPlusError mf_plus_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return MfPlusErrorNone;
    case Iso14443_4aErrorNotPresent:
        return MfPlusErrorNotPresent;
    case Iso14443_4aErrorTimeout:
        return MfPlusErrorTimeout;
    default:
        return MfPlusErrorProtocol;
    }
}

MfPlusError
    mf_plus_send_chunk(MfPlusPoller* instance, const BitBuffer* tx_buffer, BitBuffer* rx_buffer) {
    furi_assert(instance);
    furi_assert(instance->iso14443_4a_poller);
    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);

    MfPlusError error = MfPlusErrorNone;

    do {
        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
            instance->iso14443_4a_poller, tx_buffer, instance->rx_buffer);

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            error = mf_plus_process_error(iso14443_4a_error);
            break;
        }

        bit_buffer_reset(instance->tx_buffer);

        if(bit_buffer_get_size_bytes(instance->rx_buffer) > sizeof(uint8_t)) {
            bit_buffer_copy_right(rx_buffer, instance->rx_buffer, sizeof(uint8_t));
        } else {
            bit_buffer_reset(rx_buffer);
        }
    } while(false);

    return error;
}

MfPlusError mf_plus_poller_read_version(MfPlusPoller* instance, MfPlusVersion* data) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_GET_VERSION);

    MfPlusError error;

    do {
        error = mf_plus_send_chunk(instance, instance->input_buffer, instance->result_buffer);

        if(error != MfPlusErrorNone) break;

        if(!mf_plus_version_parse(data, instance->result_buffer)) {
            error = MfPlusErrorProtocol;
        }
    } while(false);

    return error;
}
