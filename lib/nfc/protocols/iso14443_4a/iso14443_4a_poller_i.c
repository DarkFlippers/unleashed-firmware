#include "iso14443_4a_poller_i.h"

#include <furi.h>

#include "iso14443_4a_i.h"

#define TAG "Iso14443_4aPoller"

#define ISO14443_4A_FSDI_256  (0x8U)
#define ISO14443_4A_FWT_MAX   (4096UL << 14)
#define ISO14443_4A_WTXM_MASK (0x3FU)
#define ISO14443_4A_WTXM_MAX  (0x3BU)
#define ISO14443_4A_SWTX      (0xF2U)

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

Iso14443_4aError iso14443_4a_poller_send_chain_block(
    Iso14443_4aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    iso14443_4_layer_set_i_block(instance->iso14443_4_layer, true, false);
    Iso14443_4aError error = iso14443_4a_poller_send_block(instance, tx_buffer, rx_buffer);
    return error;
}

Iso14443_4aError iso14443_4a_poller_send_receive_ready_block(
    Iso14443_4aPoller* instance,
    bool acknowledged,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    bool CID_present = bit_buffer_get_size_bytes(tx_buffer) != 0;
    iso14443_4_layer_set_r_block(instance->iso14443_4_layer, acknowledged, CID_present);
    Iso14443_4aError error = iso14443_4a_poller_send_block(instance, tx_buffer, rx_buffer);
    return error;
}

Iso14443_4aError iso14443_4a_poller_send_supervisory_block(
    Iso14443_4aPoller* instance,
    bool deselect,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    bool CID_present = bit_buffer_get_size_bytes(tx_buffer) != 0;
    iso14443_4_layer_set_s_block(instance->iso14443_4_layer, deselect, CID_present);
    Iso14443_4aError error = iso14443_4a_poller_send_block(instance, tx_buffer, rx_buffer);
    return error;
}
