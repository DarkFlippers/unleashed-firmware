#include "felica_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

const FelicaData* felica_poller_get_data(FelicaPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static FelicaPoller* felica_poller_alloc(Nfc* nfc) {
    furi_assert(nfc);

    FelicaPoller* instance = malloc(sizeof(FelicaPoller));
    instance->nfc = nfc;
    instance->tx_buffer = bit_buffer_alloc(FELICA_POLLER_MAX_BUFFER_SIZE);
    instance->rx_buffer = bit_buffer_alloc(FELICA_POLLER_MAX_BUFFER_SIZE);

    nfc_config(instance->nfc, NfcModePoller, NfcTechFelica);
    nfc_set_guard_time_us(instance->nfc, FELICA_GUARD_TIME_US);
    nfc_set_fdt_poll_fc(instance->nfc, FELICA_FDT_POLL_FC);
    nfc_set_fdt_poll_poll_us(instance->nfc, FELICA_POLL_POLL_MIN_US);
    instance->data = felica_alloc();

    instance->felica_event.data = &instance->felica_event_data;
    instance->general_event.protocol = NfcProtocolFelica;
    instance->general_event.event_data = &instance->felica_event;
    instance->general_event.instance = instance;

    return instance;
}

static void felica_poller_free(FelicaPoller* instance) {
    furi_assert(instance);

    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(instance->data);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    felica_free(instance->data);
    free(instance);
}

static void
    felica_poller_set_callback(FelicaPoller* instance, NfcGenericCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand felica_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    FelicaPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(nfc_event->type == NfcEventTypePollerReady) {
        if(instance->state != FelicaPollerStateActivated) {
            FelicaError error = felica_poller_activate(instance, instance->data);
            if(error == FelicaErrorNone) {
                instance->felica_event.type = FelicaPollerEventTypeReady;
                instance->felica_event_data.error = error;
                command = instance->callback(instance->general_event, instance->context);
            } else {
                instance->felica_event.type = FelicaPollerEventTypeError;
                instance->felica_event_data.error = error;
                command = instance->callback(instance->general_event, instance->context);
                // Add delay to switch context
                furi_delay_ms(100);
            }
        } else {
            instance->felica_event.type = FelicaPollerEventTypeReady;
            instance->felica_event_data.error = FelicaErrorNone;
            command = instance->callback(instance->general_event, instance->context);
        }
    }

    return command;
}

static bool felica_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolInvalid);

    bool protocol_detected = false;
    FelicaPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    furi_assert(instance->state == FelicaPollerStateIdle);

    if(nfc_event->type == NfcEventTypePollerReady) {
        FelicaError error = felica_poller_activate(instance, instance->data);
        protocol_detected = (error == FelicaErrorNone);
    }

    return protocol_detected;
}

const NfcPollerBase nfc_poller_felica = {
    .alloc = (NfcPollerAlloc)felica_poller_alloc,
    .free = (NfcPollerFree)felica_poller_free,
    .set_callback = (NfcPollerSetCallback)felica_poller_set_callback,
    .run = (NfcPollerRun)felica_poller_run,
    .detect = (NfcPollerDetect)felica_poller_detect,
    .get_data = (NfcPollerGetData)felica_poller_get_data,
};
