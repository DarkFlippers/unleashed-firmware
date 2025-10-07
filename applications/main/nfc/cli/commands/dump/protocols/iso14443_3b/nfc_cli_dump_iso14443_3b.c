#include "nfc_cli_dump_iso14443_3b.h"
#include <nfc/protocols/iso14443_3b/iso14443_3b_poller.h>

NfcCommand nfc_cli_dump_poller_callback_iso14443_3b(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_3b);

    NfcCliDumpContext* instance = context;
    const Iso14443_3bPollerEvent* iso14443_3b_event = event.event_data;

    NfcCommand command = NfcCommandContinue;
    if(iso14443_3b_event->type == Iso14443_3bPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso14443_3b, nfc_poller_get_data(instance->poller));

        instance->result = NfcCliDumpErrorNone;
        command = NfcCommandStop;
    } else if(iso14443_3b_event->type == Iso14443_3bPollerEventTypeError) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return NfcCommandContinue;
}
