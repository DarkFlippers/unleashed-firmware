#include "nfc_cli_apdu_iso14443_4a.h"
#include "../../../helpers/nfc_cli_format.h"

#include <nfc/protocols/iso14443_4a/iso14443_4a.h>
#include <nfc/protocols/iso14443_4a/iso14443_4a_poller.h>

#define TAG "ISO14A_4A"

#define BIT_BUFFER_EMPTY(buffer) ((bit_buffer_get_size_bytes(buffer) == 0))

static NfcCliApduError nfc_cli_apdu_iso14443_4a_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return NfcCliApduErrorNone;
    case Iso14443_4aErrorTimeout:
        return NfcCliApduErrorTimeout;
    case Iso14443_4aErrorNotPresent:
        return NfcCliApduErrorNotPresent;
    default:
        return NfcCliApduErrorProtocol;
    }
}

NfcCommand
    nfc_cli_apdu_iso14443_4a_handler(NfcGenericEvent event, NfcCliApduRequestResponse* instance) {
    Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        Iso14443_4aError err = iso14443_4a_poller_send_block(
            event.instance, instance->tx_buffer, instance->rx_buffer);
        instance->result = nfc_cli_apdu_iso14443_4a_process_error(err);
        if(err != Iso14443_4aErrorNone) command = NfcCommandStop;
    }

    return command;
}
