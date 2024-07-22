#include "iso14443_3a_poller_sync.h"

#include "iso14443_3a_poller_i.h" // IWYU pragma: keep
#include <nfc/nfc_poller.h>

#include <furi/furi.h>

#define ISO14443_3A_POLLER_FLAG_COMMAND_COMPLETE (1UL << 0)

typedef struct {
    Iso14443_3aPoller* instance;
    FuriThreadId thread_id;
    Iso14443_3aError error;
    Iso14443_3aData data;
} Iso14443_3aPollerContext;

NfcCommand iso14443_3a_poller_read_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);

    Iso14443_3aPollerContext* poller_context = context;
    Iso14443_3aPoller* iso14443_3a_poller = event.instance;
    Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        iso14443_3a_copy(&poller_context->data, iso14443_3a_poller->data);
    }
    poller_context->error = iso14443_3a_event->data->error;

    furi_thread_flags_set(poller_context->thread_id, ISO14443_3A_POLLER_FLAG_COMMAND_COMPLETE);

    return NfcCommandStop;
}

Iso14443_3aError iso14443_3a_poller_sync_read(Nfc* nfc, Iso14443_3aData* iso14443_3a_data) {
    furi_check(nfc);
    furi_check(iso14443_3a_data);

    Iso14443_3aPollerContext poller_context = {};
    poller_context.thread_id = furi_thread_get_current_id();

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolIso14443_3a);
    nfc_poller_start(poller, iso14443_3a_poller_read_callback, &poller_context);
    furi_thread_flags_wait(
        ISO14443_3A_POLLER_FLAG_COMMAND_COMPLETE, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(ISO14443_3A_POLLER_FLAG_COMMAND_COMPLETE);

    nfc_poller_stop(poller);
    nfc_poller_free(poller);

    if(poller_context.error == Iso14443_3aErrorNone) {
        *iso14443_3a_data = poller_context.data;
    }

    return poller_context.error;
}
