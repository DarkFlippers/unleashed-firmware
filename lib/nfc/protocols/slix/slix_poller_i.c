#include "slix_poller_i.h"
#include <bit_lib/bit_lib.h>

#include <furi.h>

#include "slix_i.h"

#define TAG "SlixPoller"

static void slix_poller_prepare_request(SlixPoller* instance, uint8_t command, bool skip_uid) {
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    uint8_t flags = ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI;
    if(!skip_uid) {
        flags |= ISO15693_3_REQ_FLAG_T4_ADDRESSED;
    }

    bit_buffer_append_byte(instance->tx_buffer, flags);
    bit_buffer_append_byte(instance->tx_buffer, command);
    bit_buffer_append_byte(instance->tx_buffer, SLIX_NXP_MANUFACTURER_CODE);

    if(!skip_uid) {
        iso15693_3_append_uid(instance->data->iso15693_3_data, instance->tx_buffer);
    }
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

    slix_poller_prepare_request(instance, SLIX_CMD_GET_NXP_SYSTEM_INFORMATION, false);

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

    slix_poller_prepare_request(instance, SLIX_CMD_READ_SIGNATURE, false);

    SlixError error = SlixErrorNone;

    do {
        error = slix_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC * 2);
        if(error != SlixErrorNone) break;
        error = slix_read_signature_response_parse(instance->data->signature, instance->rx_buffer);
    } while(false);

    return error;
}

SlixError slix_poller_get_random_number(SlixPoller* instance, SlixRandomNumber* data) {
    furi_assert(instance);
    furi_assert(data);

    slix_poller_prepare_request(instance, SLIX_CMD_GET_RANDOM_NUMBER, true);

    SlixError error = SlixErrorNone;

    do {
        error = slix_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC);
        if(error != SlixErrorNone) break;

        error = slix_get_random_number_response_parse(data, instance->rx_buffer);
    } while(false);

    return error;
}

SlixError slix_poller_set_password(
    SlixPoller* instance,
    SlixPasswordType type,
    SlixPassword password,
    SlixRandomNumber random_number) {
    furi_assert(instance);

    bool skip_uid = (type == SlixPasswordTypePrivacy);
    slix_poller_prepare_request(instance, SLIX_CMD_SET_PASSWORD, skip_uid);

    uint8_t password_type = (0x01 << type);
    bit_buffer_append_byte(instance->tx_buffer, password_type);

    uint8_t rn_l = random_number >> 8;
    uint8_t rn_h = random_number;
    uint32_t double_rand_num = (rn_h << 24) | (rn_l << 16) | (rn_h << 8) | rn_l;
    uint32_t xored_password = double_rand_num ^ password;
    uint8_t xored_password_arr[4] = {};
    bit_lib_num_to_bytes_be(xored_password, 4, xored_password_arr);
    bit_buffer_append_bytes(instance->tx_buffer, xored_password_arr, 4);

    SlixError error = SlixErrorNone;

    do {
        error = slix_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, SLIX_POLLER_SET_PASSWORD_FWT);
        if(error != SlixErrorNone) break;

        size_t rx_len = bit_buffer_get_size_bytes(instance->rx_buffer);
        if(rx_len != 1) {
            error = SlixErrorWrongPassword;
        }
    } while(false);

    return error;
}
