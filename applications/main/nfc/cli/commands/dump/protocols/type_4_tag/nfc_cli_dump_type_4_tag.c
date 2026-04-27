#include "nfc_cli_dump_type_4_tag.h"
#include <nfc/protocols/type_4_tag/type_4_tag_poller.h>

#define TAG "TYPE4TAG"

NfcCommand nfc_cli_dump_poller_callback_type_4_tag(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolType4Tag);
    furi_assert(event.event_data);

    NfcCliDumpContext* instance = context;
    const Type4TagPollerEvent* type_4_tag_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(type_4_tag_event->type == Type4TagPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolType4Tag, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorNone;
        command = NfcCommandStop;
    } else if(type_4_tag_event->type == Type4TagPollerEventTypeReadFailed) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandReset;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
