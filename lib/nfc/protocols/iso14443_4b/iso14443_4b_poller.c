#include "iso14443_4b_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "Iso14443_4bPoller"

#define ISO14443_4A_POLLER_BUF_SIZE (256U)

typedef NfcCommand (*Iso14443_4bPollerStateHandler)(Iso14443_4bPoller* instance);

const Iso14443_4bData* iso14443_4b_poller_get_data(Iso14443_4bPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

static Iso14443_4bPoller* iso14443_4b_poller_alloc(Iso14443_3bPoller* iso14443_3b_poller) {
    Iso14443_4bPoller* instance = malloc(sizeof(Iso14443_4bPoller));
    instance->iso14443_3b_poller = iso14443_3b_poller;
    instance->data = iso14443_4b_alloc();
    instance->iso14443_4_layer = iso14443_4_layer_alloc();
    instance->tx_buffer = bit_buffer_alloc(ISO14443_4A_POLLER_BUF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(ISO14443_4A_POLLER_BUF_SIZE);

    instance->iso14443_4b_event.data = &instance->iso14443_4b_event_data;

    instance->general_event.protocol = NfcProtocolIso14443_4b;
    instance->general_event.event_data = &instance->iso14443_4b_event;
    instance->general_event.instance = instance;

    return instance;
}

static void iso14443_4b_poller_free(Iso14443_4bPoller* instance) {
    furi_assert(instance);

    iso14443_4b_free(instance->data);
    iso14443_4_layer_free(instance->iso14443_4_layer);
    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    free(instance);
}

static NfcCommand iso14443_4b_poller_handler_idle(Iso14443_4bPoller* instance) {
    iso14443_3b_copy(
        instance->data->iso14443_3b_data,
        iso14443_3b_poller_get_data(instance->iso14443_3b_poller));

    iso14443_4_layer_reset(instance->iso14443_4_layer);
    instance->poller_state = Iso14443_4bPollerStateReady;
    return NfcCommandContinue;
}

static NfcCommand iso14443_4b_poller_handler_error(Iso14443_4bPoller* instance) {
    iso14443_3b_poller_halt(instance->iso14443_3b_poller);
    instance->iso14443_4b_event_data.error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->poller_state = Iso14443_4bPollerStateIdle;
    return command;
}

static NfcCommand iso14443_4b_poller_handler_ready(Iso14443_4bPoller* instance) {
    instance->iso14443_4b_event.type = Iso14443_4bPollerEventTypeReady;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    return command;
}

static const Iso14443_4bPollerStateHandler
    iso14443_4b_poller_state_handler[Iso14443_4bPollerStateNum] = {
        [Iso14443_4bPollerStateIdle] = iso14443_4b_poller_handler_idle,
        [Iso14443_4bPollerStateError] = iso14443_4b_poller_handler_error,
        [Iso14443_4bPollerStateReady] = iso14443_4b_poller_handler_ready,
};

static void iso14443_4b_poller_set_callback(
    Iso14443_4bPoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand iso14443_4b_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_3b);

    Iso14443_4bPoller* instance = context;
    furi_assert(instance);
    furi_assert(instance->callback);

    Iso14443_3bPollerEvent* iso14443_3b_event = event.event_data;
    furi_assert(iso14443_3b_event);

    NfcCommand command = NfcCommandContinue;

    if(iso14443_3b_event->type == Iso14443_3bPollerEventTypeReady) {
        command = iso14443_4b_poller_state_handler[instance->poller_state](instance);
    } else if(iso14443_3b_event->type == Iso14443_3bPollerEventTypeError) {
        instance->iso14443_4b_event.type = Iso14443_4bPollerEventTypeError;
        command = instance->callback(instance->general_event, instance->context);
    }

    return command;
}

static bool iso14443_4b_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_3b);

    const Iso14443_4bPoller* instance = context;
    furi_assert(instance);

    const Iso14443_3bPollerEvent* iso14443_3b_event = event.event_data;
    furi_assert(iso14443_3b_event);
    iso14443_3b_copy(
        instance->data->iso14443_3b_data,
        iso14443_3b_poller_get_data(instance->iso14443_3b_poller));

    bool protocol_detected = false;

    if(iso14443_3b_event->type == Iso14443_3bPollerEventTypeReady) {
        protocol_detected = iso14443_3b_supports_iso14443_4(instance->data->iso14443_3b_data);
    }

    return protocol_detected;
}

const NfcPollerBase nfc_poller_iso14443_4b = {
    .alloc = (NfcPollerAlloc)iso14443_4b_poller_alloc,
    .free = (NfcPollerFree)iso14443_4b_poller_free,
    .set_callback = (NfcPollerSetCallback)iso14443_4b_poller_set_callback,
    .run = (NfcPollerRun)iso14443_4b_poller_run,
    .detect = (NfcPollerDetect)iso14443_4b_poller_detect,
    .get_data = (NfcPollerGetData)iso14443_4b_poller_get_data,
};
