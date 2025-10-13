#include "nfc_cli_dump_iso14443_3a.h"
#include <nfc/protocols/iso14443_3a/iso14443_3a_poller.h>

NfcCommand nfc_cli_dump_poller_callback_iso14443_3a(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_3a);

    NfcCliDumpContext* instance = context;
    const Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;

    NfcCommand command = NfcCommandContinue;
    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso14443_3a, nfc_poller_get_data(instance->poller));
        command = NfcCommandStop;
    } else if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeError) {
        command = NfcCommandStop;
        instance->result = NfcCliDumpErrorFailedToRead;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
