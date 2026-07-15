#include "nfc_cli_dump_st25tb.h"
#include <nfc/protocols/st25tb/st25tb_poller.h>

NfcCommand nfc_cli_dump_poller_callback_st25tb(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolSt25tb);

    NfcCliDumpContext* instance = context;
    const St25tbPollerEvent* st25tb_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(st25tb_event->type == St25tbPollerEventTypeRequestMode) {
        st25tb_event->data->mode_request.mode = St25tbPollerModeRead;
    } else if(st25tb_event->type == St25tbPollerEventTypeSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolSt25tb, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorNone;
        command = NfcCommandStop;
    } else if(st25tb_event->type == St25tbPollerEventTypeFailure) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
