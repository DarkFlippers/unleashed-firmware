#include "iso14443_3a_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "ISO14443_3A"

const Iso14443_3aData* iso14443_3a_poller_get_data(Iso14443_3aPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static Iso14443_3aPoller* iso14443_3a_poller_alloc(Nfc* nfc) {
    furi_assert(nfc);

    Iso14443_3aPoller* instance = malloc(sizeof(Iso14443_3aPoller));
    instance->nfc = nfc;
    instance->tx_buffer = bit_buffer_alloc(ISO14443_3A_POLLER_MAX_BUFFER_SIZE);
    instance->rx_buffer = bit_buffer_alloc(ISO14443_3A_POLLER_MAX_BUFFER_SIZE);

    nfc_config(instance->nfc, NfcModePoller, NfcTechIso14443a);
    nfc_set_guard_time_us(instance->nfc, ISO14443_3A_GUARD_TIME_US);
    nfc_set_fdt_poll_fc(instance->nfc, ISO14443_3A_FDT_POLL_FC);
    nfc_set_fdt_poll_poll_us(instance->nfc, ISO14443_3A_POLL_POLL_MIN_US);
    instance->data = iso14443_3a_alloc();

    instance->iso14443_3a_event.data = &instance->iso14443_3a_event_data;
    instance->general_event.protocol = NfcProtocolIso14443_3a;
    instance->general_event.event_data = &instance->iso14443_3a_event;
    instance->general_event.instance = instance;

    return instance;
}

static void iso14443_3a_poller_free_new(Iso14443_3aPoller* iso14443_3a_poller) {
    furi_assert(iso14443_3a_poller);

    Iso14443_3aPoller* instance = iso14443_3a_poller;
    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(instance->data);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    iso14443_3a_free(instance->data);
    free(instance);
}

static void iso14443_3a_poller_set_callback(
    Iso14443_3aPoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand iso14443_3a_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    Iso14443_3aPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(nfc_event->type == NfcEventTypePollerReady) {
        if(instance->state != Iso14443_3aPollerStateActivated) {
            Iso14443_3aData data = {};
            Iso14443_3aError error = iso14443_3a_poller_activate(instance, &data);
            if(error == Iso14443_3aErrorNone) {
                instance->state = Iso14443_3aPollerStateActivated;
                instance->iso14443_3a_event.type = Iso14443_3aPollerEventTypeReady;
                instance->iso14443_3a_event_data.error = error;
                command = instance->callback(instance->general_event, instance->context);
            } else {
                instance->iso14443_3a_event.type = Iso14443_3aPollerEventTypeError;
                instance->iso14443_3a_event_data.error = error;
                command = instance->callback(instance->general_event, instance->context);
                // Add delay to switch context
                furi_delay_ms(100);
            }
        } else {
            instance->iso14443_3a_event.type = Iso14443_3aPollerEventTypeReady;
            instance->iso14443_3a_event_data.error = Iso14443_3aErrorNone;
            command = instance->callback(instance->general_event, instance->context);
        }
    }

    if(command == NfcCommandReset) {
        instance->state = Iso14443_3aPollerStateIdle;
    }

    return command;
}

static bool iso14443_3a_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolInvalid);

    bool protocol_detected = false;
    Iso14443_3aPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    furi_assert(instance->state == Iso14443_3aPollerStateIdle);

    if(nfc_event->type == NfcEventTypePollerReady) {
        Iso14443_3aError error = iso14443_3a_poller_activate(instance, NULL);
        protocol_detected = (error == Iso14443_3aErrorNone);
    }

    return protocol_detected;
}

const NfcPollerBase nfc_poller_iso14443_3a = {
    .alloc = (NfcPollerAlloc)iso14443_3a_poller_alloc,
    .free = (NfcPollerFree)iso14443_3a_poller_free_new,
    .set_callback = (NfcPollerSetCallback)iso14443_3a_poller_set_callback,
    .run = (NfcPollerRun)iso14443_3a_poller_run,
    .detect = (NfcPollerDetect)iso14443_3a_poller_detect,
    .get_data = (NfcPollerGetData)iso14443_3a_poller_get_data,
};
