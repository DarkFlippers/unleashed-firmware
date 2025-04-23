#include "mf_desfire_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "MfDesfirePoller"

#define MF_DESFIRE_BUF_SIZE        (64U)
#define MF_DESFIRE_RESULT_BUF_SIZE (512U)

typedef NfcCommand (*MfDesfirePollerReadHandler)(MfDesfirePoller* instance);

const MfDesfireData* mf_desfire_poller_get_data(MfDesfirePoller* instance) {
    furi_assert(instance);

    return instance->data;
}

static MfDesfirePoller* mf_desfire_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller) {
    MfDesfirePoller* instance = malloc(sizeof(MfDesfirePoller));
    instance->iso14443_4a_poller = iso14443_4a_poller;
    instance->data = mf_desfire_alloc();
    instance->tx_buffer = bit_buffer_alloc(MF_DESFIRE_BUF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(MF_DESFIRE_BUF_SIZE);
    instance->input_buffer = bit_buffer_alloc(MF_DESFIRE_BUF_SIZE);
    instance->result_buffer = bit_buffer_alloc(MF_DESFIRE_RESULT_BUF_SIZE);

    instance->mf_desfire_event.data = &instance->mf_desfire_event_data;

    instance->general_event.protocol = NfcProtocolMfDesfire;
    instance->general_event.event_data = &instance->mf_desfire_event;
    instance->general_event.instance = instance;

    return instance;
}

static void mf_desfire_poller_free(MfDesfirePoller* instance) {
    furi_assert(instance);

    mf_desfire_free(instance->data);
    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    bit_buffer_free(instance->input_buffer);
    bit_buffer_free(instance->result_buffer);
    free(instance);
}

static NfcCommand mf_desfire_poller_handler_idle(MfDesfirePoller* instance) {
    bit_buffer_reset(instance->input_buffer);
    bit_buffer_reset(instance->result_buffer);
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    iso14443_4a_copy(
        instance->data->iso14443_4a_data,
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller));

    instance->state = MfDesfirePollerStateReadVersion;
    return NfcCommandContinue;
}

static NfcCommand mf_desfire_poller_handler_read_version(MfDesfirePoller* instance) {
    instance->error = mf_desfire_poller_read_version(instance, &instance->data->version);
    if(instance->error == MfDesfireErrorNone) {
        FURI_LOG_D(TAG, "Read version success");
        instance->state = MfDesfirePollerStateReadFreeMemory;
    } else {
        FURI_LOG_E(TAG, "Failed to read version");
        iso14443_4a_poller_halt(instance->iso14443_4a_poller);
        instance->state = MfDesfirePollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_desfire_poller_handler_read_free_memory(MfDesfirePoller* instance) {
    NfcCommand command = NfcCommandContinue;

    instance->error = mf_desfire_poller_read_free_memory(instance, &instance->data->free_memory);
    if(instance->error == MfDesfireErrorNone) {
        FURI_LOG_D(TAG, "Read free memory success");
        instance->state = MfDesfirePollerStateReadMasterKeySettings;
    } else if(instance->error == MfDesfireErrorNotPresent) {
        FURI_LOG_D(TAG, "Read free memory is not present");
        instance->state = MfDesfirePollerStateReadMasterKeySettings;
        command = NfcCommandReset;
    } else if(instance->error == MfDesfireErrorCommandNotSupported) {
        FURI_LOG_D(TAG, "Read free memory is unsupported");
        instance->state = MfDesfirePollerStateReadMasterKeySettings;
    } else {
        FURI_LOG_E(TAG, "Failed to read free memory");
        iso14443_4a_poller_halt(instance->iso14443_4a_poller);
        instance->state = MfDesfirePollerStateReadFailed;
    }

    return command;
}

static NfcCommand mf_desfire_poller_handler_read_master_key_settings(MfDesfirePoller* instance) {
    instance->error =
        mf_desfire_poller_read_key_settings(instance, &instance->data->master_key_settings);
    if(instance->error == MfDesfireErrorNone) {
        FURI_LOG_D(TAG, "Read master key settings success");
        instance->state = MfDesfirePollerStateReadMasterKeyVersion;
    } else if(instance->error == MfDesfireErrorAuthentication) {
        FURI_LOG_D(TAG, "Auth is required to read master key settings and app ids");
        instance->data->master_key_settings.is_free_directory_list = false;
        instance->data->master_key_settings.max_keys = 1;
        instance->state = MfDesfirePollerStateReadMasterKeyVersion;
    } else {
        FURI_LOG_E(TAG, "Failed to read master key settings");
        iso14443_4a_poller_halt(instance->iso14443_4a_poller);
        instance->state = MfDesfirePollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_desfire_poller_handler_read_master_key_version(MfDesfirePoller* instance) {
    instance->error = mf_desfire_poller_read_key_versions(
        instance,
        instance->data->master_key_versions,
        instance->data->master_key_settings.max_keys);
    if(instance->error == MfDesfireErrorNone) {
        FURI_LOG_D(TAG, "Read master key version success");
        if(instance->data->master_key_settings.is_free_directory_list) {
            instance->state = MfDesfirePollerStateReadApplicationIds;
        } else {
            instance->state = MfDesfirePollerStateReadSuccess;
        }
    } else {
        FURI_LOG_E(TAG, "Failed to read master key version");
        iso14443_4a_poller_halt(instance->iso14443_4a_poller);
        instance->state = MfDesfirePollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_desfire_poller_handler_read_application_ids(MfDesfirePoller* instance) {
    instance->error =
        mf_desfire_poller_read_application_ids(instance, instance->data->application_ids);
    if(instance->error == MfDesfireErrorNone) {
        FURI_LOG_D(TAG, "Read application ids success");
        instance->state = MfDesfirePollerStateReadApplications;
    } else if(instance->error == MfDesfireErrorAuthentication) {
        FURI_LOG_D(TAG, "Read application ids impossible without authentication");
        instance->state = MfDesfirePollerStateReadSuccess;
    } else {
        FURI_LOG_E(TAG, "Failed to read application ids");
        iso14443_4a_poller_halt(instance->iso14443_4a_poller);
        instance->state = MfDesfirePollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_desfire_poller_handler_read_applications(MfDesfirePoller* instance) {
    instance->error = mf_desfire_poller_read_applications(
        instance, instance->data->application_ids, instance->data->applications);
    if(instance->error == MfDesfireErrorNone) {
        FURI_LOG_D(TAG, "Read applications success");
        instance->state = MfDesfirePollerStateReadSuccess;
    } else {
        FURI_LOG_E(TAG, "Failed to read applications");
        iso14443_4a_poller_halt(instance->iso14443_4a_poller);
        instance->state = MfDesfirePollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_desfire_poller_handler_read_fail(MfDesfirePoller* instance) {
    FURI_LOG_D(TAG, "Read Failed");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);
    instance->mf_desfire_event.data->error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->state = MfDesfirePollerStateIdle;
    return command;
}

static NfcCommand mf_desfire_poller_handler_read_success(MfDesfirePoller* instance) {
    FURI_LOG_D(TAG, "Read success.");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);
    instance->mf_desfire_event.type = MfDesfirePollerEventTypeReadSuccess;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    return command;
}

static const MfDesfirePollerReadHandler mf_desfire_poller_read_handler[MfDesfirePollerStateNum] = {
    [MfDesfirePollerStateIdle] = mf_desfire_poller_handler_idle,
    [MfDesfirePollerStateReadVersion] = mf_desfire_poller_handler_read_version,
    [MfDesfirePollerStateReadFreeMemory] = mf_desfire_poller_handler_read_free_memory,
    [MfDesfirePollerStateReadMasterKeySettings] =
        mf_desfire_poller_handler_read_master_key_settings,
    [MfDesfirePollerStateReadMasterKeyVersion] = mf_desfire_poller_handler_read_master_key_version,
    [MfDesfirePollerStateReadApplicationIds] = mf_desfire_poller_handler_read_application_ids,
    [MfDesfirePollerStateReadApplications] = mf_desfire_poller_handler_read_applications,
    [MfDesfirePollerStateReadFailed] = mf_desfire_poller_handler_read_fail,
    [MfDesfirePollerStateReadSuccess] = mf_desfire_poller_handler_read_success,
};

static void mf_desfire_poller_set_callback(
    MfDesfirePoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand mf_desfire_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4a);

    MfDesfirePoller* instance = context;
    furi_assert(instance);
    furi_assert(instance->callback);

    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;
    furi_assert(iso14443_4a_event);

    NfcCommand command = NfcCommandContinue;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        command = mf_desfire_poller_read_handler[instance->state](instance);
    } else if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeError) {
        instance->mf_desfire_event.type = MfDesfirePollerEventTypeReadFailed;
        command = instance->callback(instance->general_event, instance->context);
    }

    return command;
}

static bool mf_desfire_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4a);

    MfDesfirePoller* instance = context;
    furi_assert(instance);

    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;
    furi_assert(iso14443_4a_event);

    bool protocol_detected = false;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        do {
            MfDesfireKeyVersion key_version = 0;
            MfDesfireError error = mf_desfire_poller_read_key_version(instance, 0, &key_version);
            if(error != MfDesfireErrorNone) break;

            MfDesfireVersion version = {};
            error = mf_desfire_poller_read_version(instance, &version);
            if(error != MfDesfireErrorNone) break;

            protocol_detected = true;
        } while(false);
    }

    return protocol_detected;
}

const NfcPollerBase mf_desfire_poller = {
    .alloc = (NfcPollerAlloc)mf_desfire_poller_alloc,
    .free = (NfcPollerFree)mf_desfire_poller_free,
    .set_callback = (NfcPollerSetCallback)mf_desfire_poller_set_callback,
    .run = (NfcPollerRun)mf_desfire_poller_run,
    .detect = (NfcPollerDetect)mf_desfire_poller_detect,
    .get_data = (NfcPollerGetData)mf_desfire_poller_get_data,
};
