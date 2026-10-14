#include "texkom_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#define TAG "TexkomPoller"

static TexkomPoller* texkom_poller_alloc(Nfc* nfc) {
    furi_assert(nfc);

    TexkomPoller* instance = malloc(sizeof(TexkomPoller));
    instance->nfc = nfc;
    instance->data = texkom_alloc();
    // One byte per gap, sized for the longest frame.
    instance->rx_buffer = bit_buffer_alloc(TEXKOM_TK13_INTERVALS);

    nfc_config(instance->nfc, NfcModePoller, NfcTechTexkom);

    instance->texkom_event.type = TexkomPollerEventTypeReady;
    instance->general_event.protocol = NfcProtocolTexkom;
    instance->general_event.event_data = &instance->texkom_event;
    instance->general_event.instance = instance;

    return instance;
}

static void texkom_poller_free(TexkomPoller* instance) {
    furi_assert(instance);

    bit_buffer_free(instance->rx_buffer);
    texkom_free(instance->data);
    free(instance);
}

static void
    texkom_poller_set_callback(TexkomPoller* instance, NfcGenericCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

const TexkomData* texkom_poller_get_data(TexkomPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

/** Capture one repetition and decode it. */
static bool texkom_poller_read(TexkomPoller* instance) {
    const NfcError error = nfc_texkom_poller_rx(instance->nfc, instance->rx_buffer);
    if(error != NfcErrorNone) return false;

    const uint8_t* intervals = bit_buffer_get_data(instance->rx_buffer);
    const size_t count = bit_buffer_get_size_bytes(instance->rx_buffer);

    if(texkom_decode_intervals(instance->data, intervals, count)) return true;

    // A plausible length that no decoder accepted: either the timings are too ragged, or the
    // frame's type byte names a variant that does not use the coding it arrived in.
    FURI_LOG_D(TAG, "Captured %zu gaps, none of the codings fit", count);

    return false;
}

static NfcCommand texkom_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    TexkomPoller* instance = context;
    const NfcEvent* nfc_event = event.event_data;

    if(nfc_event->type != NfcEventTypePollerReady) return NfcCommandContinue;

    // A tag that talks unprompted gives no sign of being absent, so a failed attempt is not an
    // error worth reporting - keep listening until the caller stops us.
    if(!texkom_poller_read(instance)) return NfcCommandContinue;

    return instance->callback(instance->general_event, instance->context);
}

static bool texkom_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolInvalid);

    TexkomPoller* instance = context;
    const NfcEvent* nfc_event = event.event_data;

    if(nfc_event->type != NfcEventTypePollerReady) return false;

    return texkom_poller_read(instance);
}

const NfcPollerBase nfc_poller_texkom = {
    .alloc = (NfcPollerAlloc)texkom_poller_alloc,
    .free = (NfcPollerFree)texkom_poller_free,
    .set_callback = (NfcPollerSetCallback)texkom_poller_set_callback,
    .run = (NfcPollerRun)texkom_poller_run,
    .detect = (NfcPollerDetect)texkom_poller_detect,
    .get_data = (NfcPollerGetData)texkom_poller_get_data,
};
