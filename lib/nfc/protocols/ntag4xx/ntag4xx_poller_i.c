#include "ntag4xx_poller_i.h"

#include <furi.h>

#include "ntag4xx_i.h"

#define TAG "Ntag4xxPoller"

Ntag4xxError ntag4xx_poller_send_chunks(
    Ntag4xxPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_check(instance);

    NxpNativeCommandStatus status_code = NXP_NATIVE_COMMAND_STATUS_OPERATION_OK;
    Iso14443_4aError iso14443_4a_error = nxp_native_command_iso14443_4a_poller(
        instance->iso14443_4a_poller,
        &status_code,
        tx_buffer,
        rx_buffer,
        NxpNativeCommandModeIsoWrapped,
        instance->tx_buffer,
        instance->rx_buffer);

    if(iso14443_4a_error != Iso14443_4aErrorNone) {
        return ntag4xx_process_error(iso14443_4a_error);
    }

    return ntag4xx_process_status_code(status_code);
}

Ntag4xxError ntag4xx_poller_read_version(Ntag4xxPoller* instance, Ntag4xxVersion* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, NTAG4XX_CMD_GET_VERSION);

    Ntag4xxError error;

    do {
        error =
            ntag4xx_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);

        if(error != Ntag4xxErrorNone) break;

        if(!ntag4xx_version_parse(data, instance->result_buffer)) {
            error = Ntag4xxErrorProtocol;
        }
    } while(false);

    return error;
}
