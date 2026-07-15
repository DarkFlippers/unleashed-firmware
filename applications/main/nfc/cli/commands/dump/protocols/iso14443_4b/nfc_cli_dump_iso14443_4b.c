#include "nfc_cli_dump_iso14443_4b.h"
#include <nfc/protocols/iso14443_4b/iso14443_4b_poller.h>

NfcCommand nfc_cli_dump_poller_callback_iso14443_4b(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4b);

    NfcCliDumpContext* instance = context;
    const Iso14443_4bPollerEvent* iso14443_4b_event = event.event_data;

    NfcCommand command = NfcCommandContinue;
    if(iso14443_4b_event->type == Iso14443_4bPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso14443_4b, nfc_poller_get_data(instance->poller));
        command = NfcCommandStop;
    } else if(iso14443_4b_event->type == Iso14443_4bPollerEventTypeError) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
