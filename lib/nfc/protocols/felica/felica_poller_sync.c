#include "felica_poller_sync.h"

#include "felica_poller_i.h"
#include <nfc/nfc_poller.h>

#include <furi/furi.h>

#define FELICA_POLLER_FLAG_COMMAND_COMPLETE (1UL << 0)

typedef struct {
    FelicaAuthenticationContext auth_ctx;
    FuriThreadId thread_id;
    FelicaError error;
    FelicaData data;
} Felica_PollerContext;

NfcCommand felica_poller_read_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolFelica);

    Felica_PollerContext* poller_context = context;
    FelicaPoller* felica_poller = event.instance;

    FelicaPollerEvent* felica_event = event.event_data;

    if(felica_event->type == FelicaPollerEventTypeReady ||
       felica_event->type == FelicaPollerEventTypeIncomplete) {
        felica_copy(&poller_context->data, felica_poller->data);
    } else if(felica_event->type == FelicaPollerEventTypeRequestAuthContext) {
        felica_event->data->auth_context->skip_auth = poller_context->auth_ctx.skip_auth;
        memcpy(
            felica_event->data->auth_context->card_key.data,
            poller_context->auth_ctx.card_key.data,
            FELICA_DATA_BLOCK_SIZE);
    }

    furi_thread_flags_set(poller_context->thread_id, FELICA_POLLER_FLAG_COMMAND_COMPLETE);

    return NfcCommandStop;
}

FelicaError felica_poller_sync_read(Nfc* nfc, FelicaData* data, const FelicaCardKey* card_key) {
    furi_check(nfc);
    furi_check(data);

    Felica_PollerContext poller_context = {};
    if(card_key == NULL) {
        poller_context.auth_ctx.skip_auth = true;
    } else {
        poller_context.auth_ctx.skip_auth = false;
        memcpy(poller_context.auth_ctx.card_key.data, card_key->data, FELICA_DATA_BLOCK_SIZE);
    }

    poller_context.thread_id = furi_thread_get_current_id();
    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolFelica);
    nfc_poller_start(poller, felica_poller_read_callback, &poller_context);
    furi_thread_flags_wait(FELICA_POLLER_FLAG_COMMAND_COMPLETE, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(FELICA_POLLER_FLAG_COMMAND_COMPLETE);

    nfc_poller_stop(poller);
    nfc_poller_free(poller);

    if(poller_context.error == FelicaErrorNone) {
        *data = poller_context.data;
    }

    return poller_context.error;
}
