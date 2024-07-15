#include "iso14443_4a_poller_i.h"

#include <furi.h>

#include "iso14443_4a_i.h"

#define TAG "Iso14443_4aPoller"

#define ISO14443_4A_FSDI_256                (0x8U)
#define ISO14443_4A_SEND_BLOCK_MAX_ATTEMPTS (20)
#define ISO14443_4A_FWT_MAX                 (4096UL << 14)
#define ISO14443_4A_WTXM_MASK               (0x3FU)
#define ISO14443_4A_WTXM_MAX                (0x3BU)
#define ISO14443_4A_SWTX                    (0xF2U)

Iso14443_4aError iso14443_4a_poller_halt(Iso14443_4aPoller* instance) {
    furi_check(instance);

    iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    instance->poller_state = Iso14443_4aPollerStateIdle;

    return Iso14443_4aErrorNone;
}

Iso14443_4aError
    iso14443_4a_poller_read_ats(Iso14443_4aPoller* instance, Iso14443_4aAtsData* data) {
    furi_check(instance);
    furi_check(data);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_byte(instance->tx_buffer, ISO14443_4A_CMD_READ_ATS);
    bit_buffer_append_byte(instance->tx_buffer, ISO14443_4A_FSDI_256 << 4);

    Iso14443_4aError error = Iso14443_4aErrorNone;

    do {
        const Iso14443_3aError iso14443_3a_error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            ISO14443_4A_POLLER_ATS_FWT_FC);

        if(iso14443_3a_error != Iso14443_3aErrorNone) {
            FURI_LOG_E(TAG, "ATS request failed");
            error = iso14443_4a_process_error(iso14443_3a_error);
            break;

        } else if(!iso14443_4a_ats_parse(data, instance->rx_buffer)) {
            FURI_LOG_E(TAG, "Failed to parse ATS response");
            error = Iso14443_4aErrorProtocol;
            break;
        }

    } while(false);

    return error;
}

Iso14443_4aError iso14443_4a_poller_send_block(
    Iso14443_4aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_check(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    bit_buffer_reset(instance->tx_buffer);
    iso14443_4_layer_encode_block(instance->iso14443_4_layer, tx_buffer, instance->tx_buffer);

    Iso14443_4aError error = Iso14443_4aErrorNone;

    do {
        Iso14443_3aError iso14443_3a_error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            iso14443_4a_get_fwt_fc_max(instance->data));

        if(iso14443_3a_error != Iso14443_3aErrorNone) {
            error = iso14443_4a_process_error(iso14443_3a_error);
            break;
        }

        if(bit_buffer_starts_with_byte(instance->rx_buffer, ISO14443_4A_SWTX)) {
            do {
                uint8_t wtxm = bit_buffer_get_byte(instance->rx_buffer, 1) & ISO14443_4A_WTXM_MASK;
                if(wtxm > ISO14443_4A_WTXM_MAX) {
                    return Iso14443_4aErrorProtocol;
                }

                bit_buffer_reset(instance->tx_buffer);
                bit_buffer_copy_left(instance->tx_buffer, instance->rx_buffer, 1);
                bit_buffer_append_byte(instance->tx_buffer, wtxm);

                iso14443_3a_error = iso14443_3a_poller_send_standard_frame(
                    instance->iso14443_3a_poller,
                    instance->tx_buffer,
                    instance->rx_buffer,
                    MAX(iso14443_4a_get_fwt_fc_max(instance->data) * wtxm, ISO14443_4A_FWT_MAX));

                if(iso14443_3a_error != Iso14443_3aErrorNone) {
                    error = iso14443_4a_process_error(iso14443_3a_error);
                    return error;
                }

            } while(bit_buffer_starts_with_byte(instance->rx_buffer, ISO14443_4A_SWTX));
        }

        if(!iso14443_4_layer_decode_block(
               instance->iso14443_4_layer, rx_buffer, instance->rx_buffer)) {
            error = Iso14443_4aErrorProtocol;
            break;
        }
    } while(false);

    return error;
}

Iso14443_4aError iso14443_4a_poller_send_block_pwt_ext(
    Iso14443_4aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_assert(instance);

    uint8_t attempts_left = ISO14443_4A_SEND_BLOCK_MAX_ATTEMPTS;
    bit_buffer_reset(instance->tx_buffer);
    iso14443_4_layer_encode_block(instance->iso14443_4_layer, tx_buffer, instance->tx_buffer);

    Iso14443_4aError error = Iso14443_4aErrorNone;

    do {
        bit_buffer_reset(instance->rx_buffer);
        Iso14443_3aError iso14443_3a_error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            iso14443_4a_get_fwt_fc_max(instance->data));

        if(iso14443_3a_error != Iso14443_3aErrorNone) {
            FURI_LOG_T(
                TAG, "Attempt: %u", ISO14443_4A_SEND_BLOCK_MAX_ATTEMPTS + 1 - attempts_left);
            FURI_LOG_RAW_T("RAW RX(%d):", bit_buffer_get_size_bytes(instance->rx_buffer));
            for(size_t x = 0; x < bit_buffer_get_size_bytes(instance->rx_buffer); x++) {
                FURI_LOG_RAW_T("%02X ", bit_buffer_get_byte(instance->rx_buffer, x));
            }
            FURI_LOG_RAW_T("\r\n");

            error = iso14443_4a_process_error(iso14443_3a_error);
            break;

        } else {
            error = iso14443_4_layer_decode_block_pwt_ext(
                instance->iso14443_4_layer, rx_buffer, instance->rx_buffer);
            if(error == Iso14443_4aErrorSendExtra) {
                if(--attempts_left == 0) break;
                // Send response for Control message
                if(bit_buffer_get_size_bytes(rx_buffer))
                    bit_buffer_copy(instance->tx_buffer, rx_buffer);
                continue;
            }
            break;
        }
    } while(true);

    return error;
}
