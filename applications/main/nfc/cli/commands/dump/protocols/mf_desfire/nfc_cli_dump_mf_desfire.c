#include "nfc_cli_dump_mf_desfire.h"
#include <nfc/protocols/mf_desfire/mf_desfire_poller.h>

#define TAG "MFDES"

NfcCommand nfc_cli_dump_poller_callback_mf_desfire(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfDesfire);

    NfcCommand command = NfcCommandContinue;

    NfcCliDumpContext* instance = context;
    const MfDesfirePollerEvent* mf_desfire_event = event.event_data;

    if(mf_desfire_event->type == MfDesfirePollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfDesfire, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorNone;
        furi_semaphore_release(instance->sem_done);
        command = NfcCommandStop;
    } else if(mf_desfire_event->type == MfDesfirePollerEventTypeReadFailed) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandReset;
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
