#include "nfc_cli_dump_iso14443_4a.h"
#include <nfc/protocols/iso14443_4a/iso14443_4a_poller.h>

NfcCommand nfc_cli_dump_poller_callback_iso14443_4a(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4a);

    NfcCliDumpContext* instance = context;
    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;

    NfcCommand command = NfcCommandContinue;
    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso14443_4a, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorNone;
        command = NfcCommandStop;
    } else if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeError) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
