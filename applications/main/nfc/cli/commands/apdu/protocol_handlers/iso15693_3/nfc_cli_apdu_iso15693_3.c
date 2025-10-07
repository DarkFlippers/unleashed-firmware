#include "nfc_cli_apdu_iso15693_3.h"
#include "../../../helpers/nfc_cli_format.h"

#include <nfc/protocols/iso15693_3/iso15693_3.h>
#include <nfc/protocols/iso15693_3/iso15693_3_poller.h>

#define TAG "ISO15"

#define BIT_BUFFER_EMPTY(buffer) ((bit_buffer_get_size_bytes(buffer) == 0))

static NfcCliApduError nfc_cli_apdu_iso15693_3_process_error(Iso15693_3Error error) {
    switch(error) {
    case Iso15693_3ErrorNone:
        return NfcCliApduErrorNone;
    case Iso15693_3ErrorTimeout:
        return NfcCliApduErrorTimeout;
    case Iso15693_3ErrorNotPresent:
        return NfcCliApduErrorNotPresent;
    default:
        return NfcCliApduErrorProtocol;
    }
}

NfcCommand
    nfc_cli_apdu_iso15693_3_handler(NfcGenericEvent event, NfcCliApduRequestResponse* instance) {
    Iso15693_3PollerEvent* iso15693_3_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(iso15693_3_event->type == Iso15693_3PollerEventTypeReady) {
        Iso15693_3Error err = iso15693_3_poller_send_frame(
            event.instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC);
        instance->result = nfc_cli_apdu_iso15693_3_process_error(err);
        if(err != Iso15693_3ErrorNone) command = NfcCommandStop;
    }

    return command;
}
