#include "nfc_cli_raw_iso14443_3b.h"
#include "../../../helpers/nfc_cli_format.h"

#include <nfc/helpers/iso14443_crc.h>
#include <nfc/protocols/iso14443_3b/iso14443_3b_i.h>
#include <nfc/protocols/iso14443_3b/iso14443_3b_poller_i.h>

#define TAG "ISO14B"

#define BIT_BUFFER_EMPTY(buffer) ((bit_buffer_get_size_bytes(buffer) == 0))

static NfcCliRawError nfc_cli_raw_iso14443_3b_process_error(Iso14443_3bError error) {
    switch(error) {
    case Iso14443_3bErrorNone:
        return NfcCliRawErrorNone;
    case Iso14443_3bErrorTimeout:
        return NfcCliRawErrorTimeout;
    case Iso14443_3bErrorWrongCrc:
        return NfcCliRawErrorWrongCrc;
    case Iso14443_3bErrorNotPresent:
        return NfcCliRawErrorNotPresent;
    default:
        return NfcCliRawErrorProtocol;
    }
}

static Iso14443_3bError nfc_cli_raw_iso14443_3b_poller_process_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return Iso14443_3bErrorNone;
    case NfcErrorTimeout:
        return Iso14443_3bErrorTimeout;
    default:
        return Iso14443_3bErrorNotPresent;
    }
}

static void iso14443_3b_format_activation_data(const Iso14443_3bData* data, FuriString* output) {
    nfc_cli_format_array(data->uid, ISO14443_3B_UID_SIZE, "UID: ", output);

    const Iso14443_3bProtocolInfo* info = &data->protocol_info;
    furi_string_cat_printf(
        output,
        " BitRate: %d, Protocol: %d, Max Frame Size: %d, Fo: %d, Adc: %d, Fwi: %d",
        info->bit_rate_capability,
        info->protocol_type,
        info->max_frame_size,
        info->fo,
        info->adc,
        info->fwi);
}

static inline NfcCliRawError nfc_cli_raw_iso14443_3b_activate(
    NfcGenericInstance* poller,
    Iso14443_3bData* iso3b_data,
    FuriString* activation_string) {
    FURI_LOG_D(TAG, "Activating...");

    Iso14443_3bError error = iso14443_3b_poller_activate(poller, iso3b_data);
    if(error == Iso14443_3bErrorNone)
        iso14443_3b_format_activation_data(iso3b_data, activation_string);

    return nfc_cli_raw_iso14443_3b_process_error(error);
}

static inline NfcCliRawError nfc_cli_raw_iso14443_3b_txrx(
    NfcGenericInstance* poller,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t timeout) {
    FURI_LOG_D(TAG, "TxRx");
    Iso14443_3bPoller* iso14b_poller = poller;

    bit_buffer_reset(rx_buffer);

    Iso14443_3bError error = Iso14443_3bErrorNone;

    NfcError nfc_error = nfc_poller_trx(iso14b_poller->nfc, tx_buffer, rx_buffer, timeout);
    if(nfc_error != NfcErrorNone) {
        error = nfc_cli_raw_iso14443_3b_poller_process_error(nfc_error);
    } else if(!iso14443_crc_check(Iso14443CrcTypeB, rx_buffer)) {
        error = Iso14443_3bErrorWrongCrc;
    }

    return nfc_cli_raw_iso14443_3b_process_error(error);
}

NfcCommand nfc_cli_raw_iso14443_3b_handler(
    NfcGenericInstance* poller,
    const NfcCliRawRequest* request,
    NfcCliRawResponse* const response) {
    Iso14443_3bData iso3b_data = {0};
    bool activated = false;
    do {
        response->result = NfcCliRawErrorNone;
        if(request->select) {
            response->result =
                nfc_cli_raw_iso14443_3b_activate(poller, &iso3b_data, response->activation_string);
            activated = response->result == NfcCliRawErrorNone;
        }

        if(response->result != NfcCliRawErrorNone) break;
        if(BIT_BUFFER_EMPTY(request->tx_buffer)) break;

        uint32_t timeout = ISO14443_3B_FDT_POLL_FC;
        if(request->timeout > 0) {
            timeout = request->timeout;
        } else if(activated) {
            timeout = iso14443_3b_get_fwt_fc_max(&iso3b_data);
        }

        if(request->append_crc) {
            FURI_LOG_D(TAG, "Add CRC");
            iso14443_crc_append(Iso14443CrcTypeB, request->tx_buffer);
        }

        response->result =
            nfc_cli_raw_iso14443_3b_txrx(poller, request->tx_buffer, response->rx_buffer, timeout);
    } while(false);
    return request->keep_field ? NfcCommandContinue : NfcCommandStop;
}
