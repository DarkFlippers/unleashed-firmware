#include "nfc_cli_apdu_iso14443_4b.h"
#include "../../../helpers/nfc_cli_format.h"

#include <nfc/protocols/iso14443_4b/iso14443_4b.h>
#include <nfc/protocols/iso14443_4b/iso14443_4b_poller.h>

#define TAG "ISO14A_4B"

#define BIT_BUFFER_EMPTY(buffer) ((bit_buffer_get_size_bytes(buffer) == 0))

static NfcCliApduError nfc_cli_apdu_iso14443_4b_process_error(Iso14443_4bError error) {
    switch(error) {
    case Iso14443_4bErrorNone:
        return NfcCliApduErrorNone;
    case Iso14443_4bErrorTimeout:
        return NfcCliApduErrorTimeout;
    case Iso14443_4bErrorNotPresent:
        return NfcCliApduErrorNotPresent;
    default:
        return NfcCliApduErrorProtocol;
    }
}

NfcCommand
    nfc_cli_apdu_iso14443_4b_handler(NfcGenericEvent event, NfcCliApduRequestResponse* instance) {
    Iso14443_4bPollerEvent* iso14443_4b_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(iso14443_4b_event->type == Iso14443_4bPollerEventTypeReady) {
        Iso14443_4bError err = iso14443_4b_poller_send_block(
            event.instance, instance->tx_buffer, instance->rx_buffer);
        instance->result = nfc_cli_apdu_iso14443_4b_process_error(err);
        if(err != Iso14443_4bErrorNone) command = NfcCommandStop;
    }

    return command;
}
