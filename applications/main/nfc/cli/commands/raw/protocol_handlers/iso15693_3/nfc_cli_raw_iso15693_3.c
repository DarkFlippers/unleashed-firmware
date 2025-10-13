#include "nfc_cli_raw_iso15693_3.h"
#include "../../../helpers/nfc_cli_format.h"

#include <nfc/helpers/iso13239_crc.h>
#include <nfc/protocols/iso15693_3/iso15693_3.h>
#include <nfc/protocols/iso15693_3/iso15693_3_poller_i.h>

#define TAG "ISO15"

#define BIT_BUFFER_EMPTY(buffer) ((bit_buffer_get_size_bytes(buffer) == 0))

static NfcCliRawError nfc_cli_raw_iso15693_3_process_error(Iso15693_3Error error) {
    switch(error) {
    case Iso15693_3ErrorNone:
        return NfcCliRawErrorNone;
    case Iso15693_3ErrorTimeout:
        return NfcCliRawErrorTimeout;
    case Iso15693_3ErrorWrongCrc:
        return NfcCliRawErrorWrongCrc;
    case Iso15693_3ErrorNotPresent:
        return NfcCliRawErrorNotPresent;
    default:
        return NfcCliRawErrorProtocol;
    }
}

static Iso15693_3Error nfc_cli_raw_iso15693_3_poller_process_nfc_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return Iso15693_3ErrorNone;
    case NfcErrorTimeout:
        return Iso15693_3ErrorTimeout;
    default:
        return Iso15693_3ErrorNotPresent;
    }
}

static inline void iso15693_3_format_activation_data(const uint8_t* data, FuriString* output) {
    nfc_cli_format_array(data, ISO15693_3_UID_SIZE, "UID: ", output);
}

static inline NfcCliRawError
    nfc_cli_raw_iso15693_3_activate(NfcGenericInstance* poller, FuriString* activation_string) {
    FURI_LOG_D(TAG, "Activating...");

    Iso15693_3Poller* iso15_poller = poller;
    uint8_t uid[ISO15693_3_UID_SIZE] = {0};

    Iso15693_3Error error = iso15693_3_poller_inventory(iso15_poller, uid);
    if(error == Iso15693_3ErrorNone) {
        iso15693_3_format_activation_data(uid, activation_string);
    }
    return nfc_cli_raw_iso15693_3_process_error(error);
}

static inline NfcCliRawError nfc_cli_raw_iso15693_3_txrx(
    NfcGenericInstance* poller,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t timeout) {
    FURI_LOG_D(TAG, "TxRx");

    Iso15693_3Poller* iso15_poller = poller;

    bit_buffer_reset(rx_buffer);

    Iso15693_3Error error = Iso15693_3ErrorNone;

    NfcError nfc_error = nfc_poller_trx(iso15_poller->nfc, tx_buffer, rx_buffer, timeout);
    if(nfc_error != NfcErrorNone) {
        error = nfc_cli_raw_iso15693_3_poller_process_nfc_error(nfc_error);
    } else if(!iso13239_crc_check(Iso13239CrcTypeDefault, rx_buffer)) {
        error = Iso15693_3ErrorWrongCrc;
    }

    return nfc_cli_raw_iso15693_3_process_error(error);
}

NfcCommand nfc_cli_raw_iso15693_3_handler(
    NfcGenericInstance* poller,
    const NfcCliRawRequest* request,
    NfcCliRawResponse* const response) {
    do {
        if(request->select) {
            response->result =
                nfc_cli_raw_iso15693_3_activate(poller, response->activation_string);
        }

        if(response->result != NfcCliRawErrorNone) break;
        if(BIT_BUFFER_EMPTY(request->tx_buffer)) break;

        if(request->append_crc) {
            FURI_LOG_D(TAG, "Add CRC");
            iso13239_crc_append(Iso13239CrcTypeDefault, request->tx_buffer);
        }

        uint32_t timeout = request->timeout > 0 ? request->timeout : ISO15693_3_FDT_POLL_FC;
        response->result =
            nfc_cli_raw_iso15693_3_txrx(poller, request->tx_buffer, response->rx_buffer, timeout);
    } while(false);
    return request->keep_field ? NfcCommandContinue : NfcCommandStop;
}
