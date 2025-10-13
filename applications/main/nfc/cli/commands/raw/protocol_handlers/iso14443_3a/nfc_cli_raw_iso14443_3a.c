#include "nfc_cli_raw_iso14443_3a.h"
#include "../../../helpers/nfc_cli_format.h"

#include <nfc/helpers/iso14443_crc.h>
#include <nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <nfc/protocols/iso14443_3a/iso14443_3a_poller.h>

#define TAG "ISO14A"

#define BIT_BUFFER_EMPTY(buffer) ((bit_buffer_get_size_bytes(buffer) == 0))

static NfcCliRawError nfc_cli_raw_iso14443_3a_process_error(Iso14443_3aError error) {
    switch(error) {
    case Iso14443_3aErrorNone:
        return NfcCliRawErrorNone;
    case Iso14443_3aErrorTimeout:
        return NfcCliRawErrorTimeout;
    case Iso14443_3aErrorWrongCrc:
        return NfcCliRawErrorWrongCrc;
    case Iso14443_3aErrorNotPresent:
        return NfcCliRawErrorNotPresent;
    default:
        return NfcCliRawErrorProtocol;
    }
}

static void iso14443_3a_format_activation_data(const Iso14443_3aData* data, FuriString* output) {
    nfc_cli_format_array(data->uid, data->uid_len, "UID: ", output);
    furi_string_cat_printf(
        output, " ATQA: %02X%02X SAK: %02X", data->atqa[0], data->atqa[1], data->sak);
}

static inline NfcCliRawError
    nfc_cli_raw_iso14443_3a_activate(NfcGenericInstance* poller, FuriString* activation_string) {
    Iso14443_3aData iso3_data = {};
    FURI_LOG_D(TAG, "Activating...");

    Iso14443_3aError error = iso14443_3a_poller_activate(poller, &iso3_data);
    if(error == Iso14443_3aErrorNone)
        iso14443_3a_format_activation_data(&iso3_data, activation_string);

    return nfc_cli_raw_iso14443_3a_process_error(error);
}

static inline NfcCliRawError nfc_cli_raw_iso14443_3a_txrx(
    NfcGenericInstance* poller,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t timeout) {
    FURI_LOG_D(TAG, "TxRx");
    bit_buffer_reset(rx_buffer);
    Iso14443_3aError error = iso14443_3a_poller_txrx(poller, tx_buffer, rx_buffer, timeout);
    return nfc_cli_raw_iso14443_3a_process_error(error);
}

NfcCommand nfc_cli_raw_iso14443_3a_handler(
    NfcGenericInstance* poller,
    const NfcCliRawRequest* request,
    NfcCliRawResponse* const response) {
    do {
        response->result = NfcCliRawErrorNone;
        if(request->select) {
            response->result =
                nfc_cli_raw_iso14443_3a_activate(poller, response->activation_string);
        }

        if(response->result != NfcCliRawErrorNone) break;
        if(BIT_BUFFER_EMPTY(request->tx_buffer)) break;

        if(request->append_crc) {
            FURI_LOG_D(TAG, "Add CRC");
            iso14443_crc_append(Iso14443CrcTypeA, request->tx_buffer);
        }

        uint32_t timeout = request->timeout > 0 ? request->timeout : ISO14443_3A_FDT_LISTEN_FC;
        response->result =
            nfc_cli_raw_iso14443_3a_txrx(poller, request->tx_buffer, response->rx_buffer, timeout);
    } while(false);

    return request->keep_field ? NfcCommandContinue : NfcCommandStop;
}
