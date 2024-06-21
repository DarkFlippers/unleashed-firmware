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

MfPlusError mf_plus_poller_send_chunk(
    MfPlusPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_assert(instance);
    furi_assert(instance->iso14443_4a_poller);
    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);

    Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
        instance->iso14443_4a_poller, tx_buffer, instance->rx_buffer);
    MfPlusError error = mf_plus_process_error(iso14443_4a_error);

    if(error == MfPlusErrorNone) {
        bit_buffer_copy(rx_buffer, instance->rx_buffer);
    }

    bit_buffer_reset(instance->tx_buffer);

    return error;
}

MfPlusError mf_plus_poller_read_version(MfPlusPoller* instance, MfPlusVersion* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_GET_VERSION);

    MfPlusError error =
        mf_plus_poller_send_chunk(instance, instance->input_buffer, instance->result_buffer);
    if(error == MfPlusErrorNone) {
        error = mf_plus_version_parse(data, instance->result_buffer);
    }

    return error;
}
