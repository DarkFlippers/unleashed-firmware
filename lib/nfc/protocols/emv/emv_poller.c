#include "emv_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "EMVPoller"

// MAX Le is 255 bytes + 2 for CRC
#define EMV_BUF_SIZE (512U)

typedef NfcCommand (*EmvPollerReadHandler)(EmvPoller* instance);

const EmvData* emv_poller_get_data(EmvPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

static EmvPoller* emv_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller) {
    EmvPoller* instance = malloc(sizeof(EmvPoller));
    instance->iso14443_4a_poller = iso14443_4a_poller;
    instance->data = emv_alloc();
    instance->tx_buffer = bit_buffer_alloc(EMV_BUF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(EMV_BUF_SIZE);

    instance->state = EmvPollerStateIdle;

    instance->emv_event.data = &instance->emv_event_data;

    instance->general_event.protocol = NfcProtocolEmv;
    instance->general_event.event_data = &instance->emv_event;
    instance->general_event.instance = instance;

    return instance;
}

static void emv_poller_free(EmvPoller* instance) {
    furi_assert(instance);

    emv_free(instance->data);
    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    free(instance);
}

static NfcCommand emv_poller_handler_idle(EmvPoller* instance) {
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    iso14443_4a_copy(
        instance->data->iso14443_4a_data,
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller));

    instance->state = EmvPollerStateSelectPPSE;
    return NfcCommandContinue;
}

static NfcCommand emv_poller_handler_select_ppse(EmvPoller* instance) {
    instance->error = emv_poller_select_ppse(instance);

    if(instance->error == EmvErrorNone) {
        FURI_LOG_D(TAG, "Select PPSE success");
        instance->state = EmvPollerStateSelectApplication;
    } else {
        FURI_LOG_E(TAG, "Failed to select PPSE");
        instance->state = EmvPollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand emv_poller_handler_select_application(EmvPoller* instance) {
    instance->error = emv_poller_select_application(instance);

    if(instance->error == EmvErrorNone) {
        FURI_LOG_D(TAG, "Select application success");
    } else {
        FURI_LOG_E(TAG, "Failed to select application");
        // We have to try GPO request with empty tag
    }
    instance->state = EmvPollerStateGetProcessingOptions;

    return NfcCommandContinue;
}

static NfcCommand emv_poller_handler_get_processing_options(EmvPoller* instance) {
    instance->error = emv_poller_get_processing_options(instance);

    if(instance->error == EmvErrorNone) {
        FURI_LOG_D(TAG, "Get processing options success");
    } else {
        FURI_LOG_E(TAG, "Failed to get processing options");
    }

    // Read another informations
    instance->state = EmvPollerStateReadFiles;
    return NfcCommandContinue;
}

static NfcCommand emv_poller_handler_read_files(EmvPoller* instance) {
    // Search PAN
    emv_poller_read_afl(instance, false, &instance->records_mask);
    emv_poller_read_log_entry(instance);

    instance->state = EmvPollerStateReadExtra;
    return NfcCommandContinue;
}

static NfcCommand emv_poller_handler_read_extra_data(EmvPoller* instance) {
    emv_poller_get_last_online_atc(instance);
    emv_poller_get_pin_try_counter(instance);

    // Search cardholder name. This operation may break communication with the card, so it should be the last one
    emv_poller_read_afl(instance, true, &instance->records_mask);

    instance->state = EmvPollerStateReadSuccess;
    return NfcCommandContinue;
}

static NfcCommand emv_poller_handler_read_fail(EmvPoller* instance) {
    FURI_LOG_D(TAG, "Read failed");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);
    instance->emv_event.data->error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->state = EmvPollerStateIdle;
    return command;
}

static NfcCommand emv_poller_handler_read_success(EmvPoller* instance) {
    FURI_LOG_D(TAG, "Read success");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);
    instance->emv_event.type = EmvPollerEventTypeReadSuccess;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    return command;
}

static const EmvPollerReadHandler emv_poller_read_handler[EmvPollerStateNum] = {
    [EmvPollerStateIdle] = emv_poller_handler_idle,
    [EmvPollerStateSelectPPSE] = emv_poller_handler_select_ppse,
    [EmvPollerStateSelectApplication] = emv_poller_handler_select_application,
    [EmvPollerStateGetProcessingOptions] = emv_poller_handler_get_processing_options,
    [EmvPollerStateReadFiles] = emv_poller_handler_read_files,
    [EmvPollerStateReadExtra] = emv_poller_handler_read_extra_data,
    [EmvPollerStateReadFailed] = emv_poller_handler_read_fail,
    [EmvPollerStateReadSuccess] = emv_poller_handler_read_success,
};

static void
    emv_poller_set_callback(EmvPoller* instance, NfcGenericCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand emv_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4a);

    EmvPoller* instance = context;
    furi_assert(instance);
    furi_assert(instance->callback);

    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;
    furi_assert(iso14443_4a_event);

    NfcCommand command = NfcCommandContinue;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        command = emv_poller_read_handler[instance->state](instance);
    } else if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeError) {
        instance->emv_event.type = EmvPollerEventTypeReadFailed;
        command = instance->callback(instance->general_event, instance->context);
    }

    return command;
}

static bool emv_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4a);

    EmvPoller* instance = context;
    furi_assert(instance);

    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;
    furi_assert(iso14443_4a_event);

    bool protocol_detected = false;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        const EmvError error = emv_poller_select_ppse(instance);
        protocol_detected = (error == EmvErrorNone) && (instance->data->emv_application.aid_len);
    }

    return protocol_detected;
}

const NfcPollerBase emv_poller = {
    .alloc = (NfcPollerAlloc)emv_poller_alloc,
    .free = (NfcPollerFree)emv_poller_free,
    .set_callback = (NfcPollerSetCallback)emv_poller_set_callback,
    .run = (NfcPollerRun)emv_poller_run,
    .detect = (NfcPollerDetect)emv_poller_detect,
    .get_data = (NfcPollerGetData)emv_poller_get_data,
};
