#include "nfc_cli_dump_felica.h"
#include <nfc/protocols/felica/felica_poller.h>

NfcCommand nfc_cli_dump_poller_callback_felica(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolFelica);

    NfcCliDumpContext* instance = context;
    const FelicaPollerEvent* felica_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(felica_event->type == FelicaPollerEventTypeReady ||
       felica_event->type == FelicaPollerEventTypeIncomplete) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolFelica, nfc_poller_get_data(instance->poller));
        command = NfcCommandStop;
        instance->result = NfcCliDumpErrorNone;
    } else if(felica_event->type == FelicaPollerEventTypeError) {
        command = NfcCommandStop;
        instance->result = NfcCliDumpErrorFailedToRead;
    } else if(felica_event->type == FelicaPollerEventTypeRequestAuthContext) {
        FelicaAuthenticationContext* ctx = felica_event->data->auth_context;
        const NfcCliDumpAuthContext* dump_auth_ctx = &instance->auth_ctx;
        ctx->skip_auth = dump_auth_ctx->skip_auth;
        ctx->card_key = dump_auth_ctx->key.felica_key;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
