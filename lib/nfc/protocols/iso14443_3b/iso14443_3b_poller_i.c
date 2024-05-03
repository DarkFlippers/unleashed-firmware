#include "iso14443_3b_poller_i.h"

#include <nfc/helpers/iso14443_crc.h>

#define TAG "Iso14443_3bPoller"

#define ISO14443_3B_ATTRIB_FRAME_SIZE_256 (0x08)

static Iso14443_3bError iso14443_3b_poller_process_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return Iso14443_3bErrorNone;
    case NfcErrorTimeout:
        return Iso14443_3bErrorTimeout;
    default:
        return Iso14443_3bErrorNotPresent;
    }
}

static Iso14443_3bError iso14443_3b_poller_prepare_trx(Iso14443_3bPoller* instance) {
    furi_assert(instance);

    if(instance->state == Iso14443_3bPollerStateIdle) {
        return iso14443_3b_poller_activate(instance, NULL);
    }

    return Iso14443_3bErrorNone;
}

static Iso14443_3bError iso14443_3b_poller_frame_exchange(
    Iso14443_3bPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_assert(instance);

    const size_t tx_bytes = bit_buffer_get_size_bytes(tx_buffer);
    furi_assert(
        tx_bytes <= bit_buffer_get_capacity_bytes(instance->tx_buffer) - ISO14443_CRC_SIZE);

    bit_buffer_copy(instance->tx_buffer, tx_buffer);
    iso14443_crc_append(Iso14443CrcTypeB, instance->tx_buffer);

    Iso14443_3bError ret = Iso14443_3bErrorNone;

    do {
        NfcError error =
            nfc_poller_trx(instance->nfc, instance->tx_buffer, instance->rx_buffer, fwt);
        if(error != NfcErrorNone) {
            ret = iso14443_3b_poller_process_error(error);
            break;
        }

        bit_buffer_copy(rx_buffer, instance->rx_buffer);
        if(!iso14443_crc_check(Iso14443CrcTypeB, instance->rx_buffer)) {
            ret = Iso14443_3bErrorWrongCrc;
            break;
        }

        iso14443_crc_trim(rx_buffer);
    } while(false);

    return ret;
}

Iso14443_3bError iso14443_3b_poller_activate(Iso14443_3bPoller* instance, Iso14443_3bData* data) {
    furi_check(instance);
    furi_check(instance->nfc);
    furi_check(data);

    iso14443_3b_reset(data);

    Iso14443_3bError ret;

    do {
        instance->state = Iso14443_3bPollerStateColResInProgress;

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        // Send REQB
        bit_buffer_append_byte(instance->tx_buffer, 0x05);
        bit_buffer_append_byte(instance->tx_buffer, 0x00);
        bit_buffer_append_byte(instance->tx_buffer, 0x08);

        ret = iso14443_3b_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, ISO14443_3B_FDT_POLL_FC);
        if(ret != Iso14443_3bErrorNone) {
            instance->state = Iso14443_3bPollerStateColResFailed;
            break;
        }

        typedef struct {
            uint8_t flag;
            uint8_t uid[ISO14443_3B_UID_SIZE];
            uint8_t app_data[ISO14443_3B_APP_DATA_SIZE];
            Iso14443_3bProtocolInfo protocol_info;
        } Iso14443_3bAtqBLayout;

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(Iso14443_3bAtqBLayout)) {
            FURI_LOG_D(TAG, "Unexpected REQB response");
            instance->state = Iso14443_3bPollerStateColResFailed;
            ret = Iso14443_3bErrorCommunication;
            break;
        }

        instance->state = Iso14443_3bPollerStateActivationInProgress;

        const Iso14443_3bAtqBLayout* atqb =
            (const Iso14443_3bAtqBLayout*)bit_buffer_get_data(instance->rx_buffer);

        memcpy(data->uid, atqb->uid, ISO14443_3B_UID_SIZE);
        memcpy(data->app_data, atqb->app_data, ISO14443_3B_APP_DATA_SIZE);

        data->protocol_info = atqb->protocol_info;

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        // Send ATTRIB
        uint8_t cid = 0;
        bit_buffer_append_byte(instance->tx_buffer, 0x1d);
        bit_buffer_append_bytes(instance->tx_buffer, data->uid, ISO14443_3B_UID_SIZE);
        bit_buffer_append_byte(instance->tx_buffer, 0x00);
        bit_buffer_append_byte(instance->tx_buffer, ISO14443_3B_ATTRIB_FRAME_SIZE_256);
        bit_buffer_append_byte(instance->tx_buffer, 0x01);
        bit_buffer_append_byte(instance->tx_buffer, cid);

        ret = iso14443_3b_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, iso14443_3b_get_fwt_fc_max(data));
        if(ret != Iso14443_3bErrorNone) {
            instance->state = Iso14443_3bPollerStateActivationFailed;
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != 1) {
            FURI_LOG_W(
                TAG,
                "Unexpected ATTRIB response length: %zu",
                bit_buffer_get_size_bytes(instance->rx_buffer));
        }

        uint8_t cid_received = bit_buffer_get_byte(instance->rx_buffer, 0);
        // 15 bit is RFU
        if((cid_received & 0x7f) != cid) {
            FURI_LOG_D(TAG, "Incorrect CID in ATTRIB response: %02X", cid_received);
            instance->state = Iso14443_3bPollerStateActivationFailed;
            ret = Iso14443_3bErrorCommunication;
            break;
        }

        instance->state = Iso14443_3bPollerStateActivated;
    } while(false);

    return ret;
}

Iso14443_3bError iso14443_3b_poller_halt(Iso14443_3bPoller* instance) {
    furi_check(instance);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    bit_buffer_append_byte(instance->tx_buffer, 0x50);
    bit_buffer_append_bytes(instance->tx_buffer, instance->data->uid, ISO14443_3B_UID_SIZE);

    Iso14443_3bError ret;

    do {
        ret = iso14443_3b_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, ISO14443_3B_FDT_POLL_FC);
        if(ret != Iso14443_3bErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(uint8_t) ||
           bit_buffer_get_byte(instance->rx_buffer, 0) != 0) {
            ret = Iso14443_3bErrorCommunication;
            break;
        }

        instance->state = Iso14443_3bPollerStateIdle;
    } while(false);

    return ret;
}

Iso14443_3bError iso14443_3b_poller_send_frame(
    Iso14443_3bPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_check(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    Iso14443_3bError ret;

    do {
        ret = iso14443_3b_poller_prepare_trx(instance);
        if(ret != Iso14443_3bErrorNone) break;

        ret = iso14443_3b_poller_frame_exchange(
            instance, tx_buffer, rx_buffer, iso14443_3b_get_fwt_fc_max(instance->data));
    } while(false);

    return ret;
}
