#include "nfc_cli_raw_felica.h"
#include "../../../helpers/nfc_cli_format.h"

#include <nfc/helpers/felica_crc.h>
#include <nfc/protocols/felica/felica.h>
#include <nfc/protocols/felica/felica_poller_i.h>

#define TAG "FELICA"

#define BIT_BUFFER_EMPTY(buffer) ((bit_buffer_get_size_bytes(buffer) == 0))

static inline void felica_format_activation_data(const FelicaData* data, FuriString* output) {
    nfc_cli_format_array(data->idm.data, FELICA_IDM_SIZE, "IDm: ", output);
    nfc_cli_format_array(data->pmm.data, FELICA_PMM_SIZE, " PMm: ", output);
}

static NfcCliRawError nfc_cli_raw_felica_process_error(FelicaError error) {
    switch(error) {
    case FelicaErrorNone:
        return NfcCliRawErrorNone;
    case FelicaErrorTimeout:
        return NfcCliRawErrorTimeout;
    case FelicaErrorWrongCrc:
        return NfcCliRawErrorWrongCrc;
    case FelicaErrorNotPresent:
        return NfcCliRawErrorNotPresent;
    default:
        return NfcCliRawErrorProtocol;
    }
}

static FelicaError nfc_cli_raw_felica_poller_process_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return FelicaErrorNone;
    case NfcErrorTimeout:
        return FelicaErrorTimeout;
    default:
        return FelicaErrorNotPresent;
    }
}

static inline NfcCliRawError
    nfc_cli_raw_felica_activate(NfcGenericInstance* poller, FuriString* activation_string) {
    FelicaData felica_data = {};
    FelicaPoller* felica_poller = poller;
    FURI_LOG_D(TAG, "Activating...");

    FelicaError error = felica_poller_activate(felica_poller, &felica_data);
    if(error == FelicaErrorNone) {
        felica_format_activation_data(&felica_data, activation_string);
    }

    return nfc_cli_raw_felica_process_error(error);
}

static inline NfcCliRawError nfc_cli_raw_felica_txrx(
    NfcGenericInstance* poller,
    BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t timeout) {
    FURI_LOG_D(TAG, "TxRx");
    FelicaPoller* felica_poller = poller;

    bit_buffer_reset(rx_buffer);

    FelicaError error = FelicaErrorNone;

    NfcError nfc_error = nfc_poller_trx(felica_poller->nfc, tx_buffer, rx_buffer, timeout);
    if(nfc_error != NfcErrorNone) {
        error = nfc_cli_raw_felica_poller_process_error(nfc_error);
    } else if(!felica_crc_check(rx_buffer)) {
        error = FelicaErrorWrongCrc;
    }

    return nfc_cli_raw_felica_process_error(error);
}

NfcCommand nfc_cli_raw_felica_handler(
    NfcGenericInstance* poller,
    const NfcCliRawRequest* request,
    NfcCliRawResponse* const response) {
    do {
        if(request->select) {
            response->result = nfc_cli_raw_felica_activate(poller, response->activation_string);
        }

        if(response->result != NfcCliRawErrorNone) break;
        if(BIT_BUFFER_EMPTY(request->tx_buffer)) break;

        if(request->append_crc) {
            FURI_LOG_D(TAG, "Add CRC");
            felica_crc_append(request->tx_buffer);
        }

        uint32_t timeout = request->timeout > 0 ? request->timeout : FELICA_FDT_POLL_FC;
        response->result =
            nfc_cli_raw_felica_txrx(poller, request->tx_buffer, response->rx_buffer, timeout);
    } while(false);
    return request->keep_field ? NfcCommandContinue : NfcCommandStop;
}
