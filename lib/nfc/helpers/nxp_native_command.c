#include "nxp_native_command.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller_i.h>

#define TAG "NxpNativeCommand"

Iso14443_4aError nxp_native_command_iso14443_4a_poller(
    Iso14443_4aPoller* iso14443_4a_poller,
    NxpNativeCommandStatus* status_code,
    const BitBuffer* input_buffer,
    BitBuffer* result_buffer,
    NxpNativeCommandMode command_mode,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_check(iso14443_4a_poller);
    furi_check(tx_buffer);
    furi_check(rx_buffer);
    furi_check(input_buffer);
    furi_check(result_buffer);
    furi_check(command_mode < NxpNativeCommandModeMAX);

    Iso14443_4aError error = Iso14443_4aErrorNone;
    *status_code = NXP_NATIVE_COMMAND_STATUS_OPERATION_OK;

    do {
        bit_buffer_reset(tx_buffer);
        if(command_mode == NxpNativeCommandModePlain) {
            bit_buffer_append(tx_buffer, input_buffer);
        } else if(command_mode == NxpNativeCommandModeIsoWrapped) {
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_CLA);
            bit_buffer_append_byte(tx_buffer, bit_buffer_get_byte(input_buffer, 0));
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_P1);
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_P2);
            if(bit_buffer_get_size_bytes(input_buffer) > 1) {
                bit_buffer_append_byte(tx_buffer, bit_buffer_get_size_bytes(input_buffer) - 1);
                bit_buffer_append_right(tx_buffer, input_buffer, 1);
            }
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_LE);
        }

        bit_buffer_reset(rx_buffer);
        error = iso14443_4a_poller_send_block(iso14443_4a_poller, tx_buffer, rx_buffer);

        if(error != Iso14443_4aErrorNone) {
            break;
        }

        bit_buffer_reset(tx_buffer);
        if(command_mode == NxpNativeCommandModePlain) {
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_STATUS_ADDITIONAL_FRAME);
        } else if(command_mode == NxpNativeCommandModeIsoWrapped) {
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_CLA);
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_STATUS_ADDITIONAL_FRAME);
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_P1);
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_P2);
            bit_buffer_append_byte(tx_buffer, NXP_NATIVE_COMMAND_ISO_LE);
        }

        size_t response_len = bit_buffer_get_size_bytes(rx_buffer);
        *status_code = NXP_NATIVE_COMMAND_STATUS_LENGTH_ERROR;
        bit_buffer_reset(result_buffer);
        if(command_mode == NxpNativeCommandModePlain && response_len >= sizeof(uint8_t)) {
            *status_code = bit_buffer_get_byte(rx_buffer, 0);
            if(response_len > sizeof(uint8_t)) {
                bit_buffer_copy_right(result_buffer, rx_buffer, sizeof(uint8_t));
            }
        } else if(
            command_mode == NxpNativeCommandModeIsoWrapped &&
            response_len >= 2 * sizeof(uint8_t) &&
            bit_buffer_get_byte(rx_buffer, response_len - 2) == NXP_NATIVE_COMMAND_ISO_SW1) {
            *status_code = bit_buffer_get_byte(rx_buffer, response_len - 1);
            if(response_len > 2 * sizeof(uint8_t)) {
                bit_buffer_copy_left(result_buffer, rx_buffer, response_len - 2 * sizeof(uint8_t));
            }
        }

        while(*status_code == NXP_NATIVE_COMMAND_STATUS_ADDITIONAL_FRAME) {
            bit_buffer_reset(rx_buffer);
            error = iso14443_4a_poller_send_block(iso14443_4a_poller, tx_buffer, rx_buffer);

            if(error != Iso14443_4aErrorNone) {
                break;
            }

            const size_t rx_size = bit_buffer_get_size_bytes(rx_buffer);
            const size_t rx_capacity_remaining = bit_buffer_get_capacity_bytes(result_buffer) -
                                                 bit_buffer_get_size_bytes(result_buffer);

            if(command_mode == NxpNativeCommandModePlain) {
                *status_code = rx_size >= 1 ? bit_buffer_get_byte(rx_buffer, 0) :
                                              NXP_NATIVE_COMMAND_STATUS_LENGTH_ERROR;
                if(rx_size <= rx_capacity_remaining + 1) {
                    bit_buffer_append_right(result_buffer, rx_buffer, sizeof(uint8_t));
                } else {
                    FURI_LOG_W(TAG, "RX buffer overflow: ignoring %zu bytes", rx_size - 1);
                }
            } else if(command_mode == NxpNativeCommandModeIsoWrapped) {
                if(rx_size >= 2 &&
                   bit_buffer_get_byte(rx_buffer, rx_size - 2) == NXP_NATIVE_COMMAND_ISO_SW1) {
                    *status_code = bit_buffer_get_byte(rx_buffer, rx_size - 1);
                } else {
                    *status_code = NXP_NATIVE_COMMAND_STATUS_LENGTH_ERROR;
                }
                if(rx_size <= rx_capacity_remaining + 2) {
                    bit_buffer_set_size_bytes(rx_buffer, rx_size - 2);
                    bit_buffer_append(result_buffer, rx_buffer);
                } else {
                    FURI_LOG_W(TAG, "RX buffer overflow: ignoring %zu bytes", rx_size - 2);
                }
            }
        }
    } while(false);

    return error;
}
