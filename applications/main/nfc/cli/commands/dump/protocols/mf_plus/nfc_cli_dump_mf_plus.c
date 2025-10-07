#include "nfc_cli_dump_mf_plus.h"
#include <nfc/protocols/mf_plus/mf_plus_poller.h>

#define TAG "MFPLUS"

NfcCommand nfc_cli_dump_poller_callback_mf_plus(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolMfPlus);
    furi_assert(event.event_data);

    NfcCliDumpContext* instance = context;
    const MfPlusPollerEvent* mf_plus_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(mf_plus_event->type == MfPlusPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfPlus, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorNone;
        command = NfcCommandStop;
    } else if(mf_plus_event->type == MfPlusPollerEventTypeReadFailed) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandReset;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
