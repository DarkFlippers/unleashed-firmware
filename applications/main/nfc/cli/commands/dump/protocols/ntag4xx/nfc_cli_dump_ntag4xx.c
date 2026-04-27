#include "nfc_cli_dump_ntag4xx.h"
#include <nfc/protocols/ntag4xx/ntag4xx_poller.h>

#define TAG "NTAG4XX"

NfcCommand nfc_cli_dump_poller_callback_ntag4xx(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolNtag4xx);
    furi_assert(event.event_data);

    NfcCliDumpContext* instance = context;
    const Ntag4xxPollerEvent* ntag4xx_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(ntag4xx_event->type == Ntag4xxPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolNtag4xx, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorNone;
        command = NfcCommandStop;
    } else if(ntag4xx_event->type == Ntag4xxPollerEventTypeReadFailed) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandReset;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
