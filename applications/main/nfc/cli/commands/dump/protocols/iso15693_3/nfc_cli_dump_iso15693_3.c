#include "nfc_cli_dump_iso15693_3.h"
#include <nfc/protocols/iso15693_3/iso15693_3_poller.h>

NfcCommand nfc_cli_dump_poller_callback_iso15693_3(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso15693_3);

    NfcCliDumpContext* instance = context;
    const Iso15693_3PollerEvent* iso15693_3_event = event.event_data;

    NfcCommand command = NfcCommandContinue;
    if(iso15693_3_event->type == Iso15693_3PollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso15693_3, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandStop;
    } else if(iso15693_3_event->type == Iso15693_3PollerEventTypeError) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
