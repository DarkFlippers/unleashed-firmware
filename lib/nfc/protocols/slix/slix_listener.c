#include "slix_listener_i.h"

#include <furi.h>
#include <nfc/protocols/nfc_listener_base.h>

#define TAG "SlixListener"

#define SLIX_LISTENER_BUF_SIZE (64U)

static SlixListener* slix_listener_alloc(Iso15693_3Listener* iso15693_3_listener, SlixData* data) {
    furi_assert(iso15693_3_listener);

    SlixListener* instance = malloc(sizeof(SlixListener));
    instance->iso15693_3_listener = iso15693_3_listener;
    instance->data = data;

    instance->tx_buffer = bit_buffer_alloc(SLIX_LISTENER_BUF_SIZE);

    instance->slix_event.data = &instance->slix_event_data;
    instance->generic_event.protocol = NfcProtocolSlix;
    instance->generic_event.instance = instance;
    instance->generic_event.event_data = &instance->slix_event;

    slix_listener_init_iso15693_3_extensions(instance);

    return instance;
}

static void slix_listener_free(SlixListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    furi_assert(instance->tx_buffer);

    bit_buffer_free(instance->tx_buffer);
    free(instance);
}

static void
    slix_listener_set_callback(SlixListener* instance, NfcGenericCallback callback, void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

static const SlixData* slix_listener_get_data(SlixListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static NfcCommand slix_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso15693_3);
    furi_assert(event.event_data);

    SlixListener* instance = context;
    Iso15693_3ListenerEvent* iso15693_3_event = event.event_data;
    BitBuffer* rx_buffer = iso15693_3_event->data->buffer;
    NfcCommand command = NfcCommandContinue;

    if(iso15693_3_event->type == Iso15693_3ListenerEventTypeCustomCommand) {
        const SlixError error = slix_listener_process_request(instance, rx_buffer);
        if(error == SlixErrorWrongPassword) {
            command = NfcCommandSleep;
        }
    }

    return command;
}

const NfcListenerBase nfc_listener_slix = {
    .alloc = (NfcListenerAlloc)slix_listener_alloc,
    .free = (NfcListenerFree)slix_listener_free,
    .set_callback = (NfcListenerSetCallback)slix_listener_set_callback,
    .get_data = (NfcListenerGetData)slix_listener_get_data,
    .run = (NfcListenerRun)slix_listener_run,
};
