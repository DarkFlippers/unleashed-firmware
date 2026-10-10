#include "texkom_listener_i.h"

#include <nfc/protocols/nfc_listener_base.h>

#define TAG "TexkomListener"

static TexkomListener* texkom_listener_alloc(Nfc* nfc, TexkomData* data) {
    furi_assert(nfc);
    furi_assert(data);

    TexkomListener* instance = malloc(sizeof(TexkomListener));
    instance->nfc = nfc;
    instance->data = data;
    instance->tx_buffer = bit_buffer_alloc(TEXKOM_FRAME_SIZE);

    nfc_config(instance->nfc, NfcModeListener, NfcTechTexkom);

    return instance;
}

static void texkom_listener_free(TexkomListener* instance) {
    furi_assert(instance);

    bit_buffer_free(instance->tx_buffer);
    free(instance);
}

static void texkom_listener_set_callback(
    TexkomListener* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

static const TexkomData* texkom_listener_get_data(const TexkomListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static NfcCommand texkom_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);

    TexkomListener* instance = context;
    const NfcEvent* nfc_event = event.event_data;

    // There is no activation to wait for and no command to answer: a Texkom tag simply repeats
    // its frame. The HAL ticks at the rate one repetition should go out.
    if(nfc_event->type == NfcEventTypeListenerTick) {
        bit_buffer_copy_bytes(instance->tx_buffer, instance->data->frame, TEXKOM_FRAME_SIZE);

        const NfcError error = nfc_listener_tx(instance->nfc, instance->tx_buffer);
        if(error != NfcErrorNone) {
            FURI_LOG_E(TAG, "Failed to transmit the frame: %d", error);
        }
    }

    return NfcCommandContinue;
}

const NfcListenerBase nfc_listener_texkom = {
    .alloc = (NfcListenerAlloc)texkom_listener_alloc,
    .free = (NfcListenerFree)texkom_listener_free,
    .set_callback = (NfcListenerSetCallback)texkom_listener_set_callback,
    .get_data = (NfcListenerGetData)texkom_listener_get_data,
    .run = (NfcListenerRun)texkom_listener_run,
};
