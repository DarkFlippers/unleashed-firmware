#include "slix_poller_i.h"

#include <furi.h>

#include "slix_i.h"

#define TAG "SlixPoller"

static void slix_poller_prepare_request(SlixPoller* instance, uint8_t command) {
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    bit_buffer_append_byte(
        instance->tx_buffer,
        ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI |
            ISO15693_3_REQ_FLAG_T4_ADDRESSED);
    bit_buffer_append_byte(instance->tx_buffer, command);
    bit_buffer_append_byte(instance->tx_buffer, SLIX_NXP_MANUFACTURER_CODE);

    iso15693_3_append_uid(instance->data->iso15693_3_data, instance->tx_buffer);
}

SlixError slix_poller_send_frame(
    SlixPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_assert(instance);

    const Iso15693_3Error iso15693_3_error =
        iso15693_3_poller_send_frame(instance->iso15693_3_poller, tx_buffer, rx_buffer, fwt);
    return slix_process_iso15693_3_error(iso15693_3_error);
}

SlixError slix_poller_get_nxp_system_info(SlixPoller* instance, SlixSystemInfo* data) {
    furi_assert(instance);
    furi_assert(data);

    slix_poller_prepare_request(instance, SLIX_CMD_GET_NXP_SYSTEM_INFORMATION);

    SlixError error = SlixErrorNone;

    do {
        error = slix_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC);
        if(error != SlixErrorNone) break;
        error = slix_get_nxp_system_info_response_parse(instance->data, instance->rx_buffer);
    } while(false);

    return error;
}

SlixError slix_poller_read_signature(SlixPoller* instance, SlixSignature* data) {
    furi_assert(instance);
    furi_assert(data);

    slix_poller_prepare_request(instance, SLIX_CMD_READ_SIGNATURE);

    SlixError error = SlixErrorNone;

    do {
        error = slix_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC * 2);
        if(error != SlixErrorNone) break;
        error = slix_read_signature_response_parse(instance->data->signature, instance->rx_buffer);
    } while(false);

    return error;
}
