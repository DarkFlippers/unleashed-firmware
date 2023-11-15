#include "iso14443_4a_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "Iso14443_4aPoller"

#define ISO14443_4A_POLLER_BUF_SIZE (256U)

typedef NfcCommand (*Iso14443_4aPollerStateHandler)(Iso14443_4aPoller* instance);

const Iso14443_4aData* iso14443_4a_poller_get_data(Iso14443_4aPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

static Iso14443_4aPoller* iso14443_4a_poller_alloc(Iso14443_3aPoller* iso14443_3a_poller) {
    Iso14443_4aPoller* instance = malloc(sizeof(Iso14443_4aPoller));
    instance->iso14443_3a_poller = iso14443_3a_poller;
    instance->data = iso14443_4a_alloc();
    instance->iso14443_4_layer = iso14443_4_layer_alloc();
    instance->tx_buffer = bit_buffer_alloc(ISO14443_4A_POLLER_BUF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(ISO14443_4A_POLLER_BUF_SIZE);

    instance->iso14443_4a_event.data = &instance->iso14443_4a_event_data;

    instance->general_event.protocol = NfcProtocolIso14443_4a;
    instance->general_event.event_data = &instance->iso14443_4a_event;
    instance->general_event.instance = instance;

    return instance;
}

static void iso14443_4a_poller_free(Iso14443_4aPoller* instance) {
    furi_assert(instance);

    iso14443_4a_free(instance->data);
    iso14443_4_layer_free(instance->iso14443_4_layer);
    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    free(instance);
}

static NfcCommand iso14443_4a_poller_handler_idle(Iso14443_4aPoller* instance) {
    iso14443_3a_copy(
        instance->data->iso14443_3a_data,
        iso14443_3a_poller_get_data(instance->iso14443_3a_poller));

    iso14443_4_layer_reset(instance->iso14443_4_layer);

    instance->poller_state = Iso14443_4aPollerStateReadAts;
    return NfcCommandContinue;
}

static NfcCommand iso14443_4a_poller_handler_read_ats(Iso14443_4aPoller* instance) {
    Iso14443_4aError error = iso14443_4a_poller_read_ats(instance, &instance->data->ats_data);
    if(error == Iso14443_4aErrorNone) {
        FURI_LOG_D(TAG, "Read ATS success");
        instance->poller_state = Iso14443_4aPollerStateReady;
    } else {
        FURI_LOG_D(TAG, "Failed to read ATS");
        instance->poller_state = Iso14443_4aPollerStateError;
    }

    return NfcCommandContinue;
}

static NfcCommand iso14443_4a_poller_handler_error(Iso14443_4aPoller* instance) {
    iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    instance->iso14443_4a_event_data.error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->poller_state = Iso14443_4aPollerStateIdle;
    return command;
}

static NfcCommand iso14443_4a_poller_handler_ready(Iso14443_4aPoller* instance) {
    instance->iso14443_4a_event.type = Iso14443_4aPollerEventTypeReady;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    return command;
}

static const Iso14443_4aPollerStateHandler
    iso14443_4a_poller_state_handler[Iso14443_4aPollerStateNum] = {
        [Iso14443_4aPollerStateIdle] = iso14443_4a_poller_handler_idle,
        [Iso14443_4aPollerStateReadAts] = iso14443_4a_poller_handler_read_ats,
        [Iso14443_4aPollerStateError] = iso14443_4a_poller_handler_error,
        [Iso14443_4aPollerStateReady] = iso14443_4a_poller_handler_ready,
};

static void iso14443_4a_poller_set_callback(
    Iso14443_4aPoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand iso14443_4a_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_3a);

    Iso14443_4aPoller* instance = context;
    furi_assert(instance);
    furi_assert(instance->callback);

    Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;
    furi_assert(iso14443_3a_event);

    NfcCommand command = NfcCommandContinue;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        command = iso14443_4a_poller_state_handler[instance->poller_state](instance);
    } else if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeError) {
        instance->iso14443_4a_event.type = Iso14443_4aPollerEventTypeError;
        command = instance->callback(instance->general_event, instance->context);
    }

    return command;
}

static bool iso14443_4a_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_3a);

    const Iso14443_4aPoller* instance = context;
    furi_assert(instance);

    const Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;
    furi_assert(iso14443_3a_event);
    iso14443_3a_copy(
        instance->data->iso14443_3a_data,
        iso14443_3a_poller_get_data(instance->iso14443_3a_poller));

    bool protocol_detected = false;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        protocol_detected = iso14443_3a_supports_iso14443_4(instance->data->iso14443_3a_data);
    }

    return protocol_detected;
}

const NfcPollerBase nfc_poller_iso14443_4a = {
    .alloc = (NfcPollerAlloc)iso14443_4a_poller_alloc,
    .free = (NfcPollerFree)iso14443_4a_poller_free,
    .set_callback = (NfcPollerSetCallback)iso14443_4a_poller_set_callback,
    .run = (NfcPollerRun)iso14443_4a_poller_run,
    .detect = (NfcPollerDetect)iso14443_4a_poller_detect,
    .get_data = (NfcPollerGetData)iso14443_4a_poller_get_data,
};
