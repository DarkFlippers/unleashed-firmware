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

MfPlusError mf_plus_process_status_code(uint8_t status_code) {
    switch(status_code) {
    case NXP_NATIVE_COMMAND_STATUS_OPERATION_OK:
        return MfPlusErrorNone;
    default:
        return MfPlusErrorProtocol;
    }
}

MfPlusError mf_plus_poller_send_chunks(
    MfPlusPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_assert(instance);

    NxpNativeCommandStatus status_code = NXP_NATIVE_COMMAND_STATUS_OPERATION_OK;
    Iso14443_4aError iso14443_4a_error = nxp_native_command_iso14443_4a_poller(
        instance->iso14443_4a_poller,
        &status_code,
        tx_buffer,
        rx_buffer,
        NxpNativeCommandModePlain,
        instance->tx_buffer,
        instance->rx_buffer);

    if(iso14443_4a_error != Iso14443_4aErrorNone) {
        return mf_plus_process_error(iso14443_4a_error);
    }

    return mf_plus_process_status_code(status_code);
}

MfPlusError mf_plus_poller_read_version(MfPlusPoller* instance, MfPlusVersion* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_GET_VERSION);

    MfPlusError error =
        mf_plus_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);
    if(error == MfPlusErrorNone) {
        error = mf_plus_version_parse(data, instance->result_buffer);
    }

    return error;
}
