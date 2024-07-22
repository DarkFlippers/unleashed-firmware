#include "mf_plus_poller_i.h"
#include "mf_plus_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "MfPlusPoller"

#define MF_PLUS_BUF_SIZE        (64U)
#define MF_PLUS_RESULT_BUF_SIZE (512U)

typedef NfcCommand (*MfPlusPollerReadHandler)(MfPlusPoller* instance);

const MfPlusData* mf_plus_poller_get_data(MfPlusPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

MfPlusPoller* mf_plus_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller) {
    furi_assert(iso14443_4a_poller);

    MfPlusPoller* instance = malloc(sizeof(MfPlusPoller));

    instance->iso14443_4a_poller = iso14443_4a_poller;

    instance->data = mf_plus_alloc();

    instance->tx_buffer = bit_buffer_alloc(MF_PLUS_BUF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(MF_PLUS_BUF_SIZE);
    instance->input_buffer = bit_buffer_alloc(MF_PLUS_BUF_SIZE);
    instance->result_buffer = bit_buffer_alloc(MF_PLUS_RESULT_BUF_SIZE);

    instance->general_event.protocol = NfcProtocolMfPlus;
    instance->general_event.event_data = &instance->mfp_event;
    instance->general_event.instance = instance;

    instance->mfp_event.data = &instance->mfp_event_data;

    return instance;
}

static NfcCommand mf_plus_poller_handler_idle(MfPlusPoller* instance) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_reset(instance->result_buffer);
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    iso14443_4a_copy(
        instance->data->iso14443_4a_data,
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller));

    instance->state = MfPlusPollerStateReadVersion;
    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_read_version(MfPlusPoller* instance) {
    MfPlusError error = mf_plus_poller_read_version(instance, &instance->data->version);
    if(error == MfPlusErrorNone) {
        instance->state = MfPlusPollerStateParseVersion;
    } else {
        instance->state = MfPlusPollerStateParseIso4;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_parse_version(MfPlusPoller* instance) {
    furi_assert(instance);

    MfPlusError error = mf_plus_get_type_from_version(
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller), instance->data);
    if(error == MfPlusErrorNone) {
        instance->state = MfPlusPollerStateReadSuccess;
    } else {
        instance->error = error;
        instance->state = MfPlusPollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_parse_iso4(MfPlusPoller* instance) {
    furi_assert(instance);

    MfPlusError error = mf_plus_get_type_from_iso4(
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller), instance->data);
    if(error == MfPlusErrorNone) {
        instance->state = MfPlusPollerStateReadSuccess;
    } else {
        instance->error = error;
        instance->state = MfPlusPollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_read_failed(MfPlusPoller* instance) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Read failed");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);

    instance->mfp_event.type = MfPlusPollerEventTypeReadFailed;
    instance->mfp_event.data->error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->state = MfPlusPollerStateIdle;

    return command;
}

static NfcCommand mf_plus_poller_handler_read_success(MfPlusPoller* instance) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Read success");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);

    instance->mfp_event.type = MfPlusPollerEventTypeReadSuccess;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    return command;
}

static const MfPlusPollerReadHandler mf_plus_poller_read_handler[MfPlusPollerStateNum] = {
    [MfPlusPollerStateIdle] = mf_plus_poller_handler_idle,
    [MfPlusPollerStateReadVersion] = mf_plus_poller_handler_read_version,
    [MfPlusPollerStateParseVersion] = mf_plus_poller_handler_parse_version,
    [MfPlusPollerStateParseIso4] = mf_plus_poller_handler_parse_iso4,
    [MfPlusPollerStateReadFailed] = mf_plus_poller_handler_read_failed,
    [MfPlusPollerStateReadSuccess] = mf_plus_poller_handler_read_success,
};

static void mf_plus_poller_set_callback(
    MfPlusPoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand mf_plus_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol = NfcProtocolIso14443_4a);
    furi_assert(event.event_data);

    MfPlusPoller* instance = context;
    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        command = mf_plus_poller_read_handler[instance->state](instance);
    } else if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeError) {
        instance->mfp_event.type = MfPlusPollerEventTypeReadFailed;
        command = instance->callback(instance->general_event, instance->context);
    }

    return command;
}

void mf_plus_poller_free(MfPlusPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    bit_buffer_free(instance->input_buffer);
    bit_buffer_free(instance->result_buffer);
    mf_plus_free(instance->data);
    free(instance);
}

static bool mf_plus_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol = NfcProtocolIso14443_4a);
    furi_assert(event.event_data);

    MfPlusPoller* instance = context;
    Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;

    MfPlusError error = MfPlusErrorUnknown;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        error = mf_plus_poller_read_version(instance, &instance->data->version);
        if(error == MfPlusErrorNone) {
            error = mf_plus_get_type_from_version(
                iso14443_4a_poller_get_data(instance->iso14443_4a_poller), instance->data);
        } else {
            error = mf_plus_get_type_from_iso4(
                iso14443_4a_poller_get_data(instance->iso14443_4a_poller), instance->data);
        }
    }

    return error == MfPlusErrorNone;
}

const NfcPollerBase mf_plus_poller = {
    .alloc = (NfcPollerAlloc)mf_plus_poller_alloc,
    .free = (NfcPollerFree)mf_plus_poller_free,
    .set_callback = (NfcPollerSetCallback)mf_plus_poller_set_callback,
    .run = (NfcPollerRun)mf_plus_poller_run,
    .detect = (NfcPollerDetect)mf_plus_poller_detect,
    .get_data = (NfcPollerGetData)mf_plus_poller_get_data,
};
