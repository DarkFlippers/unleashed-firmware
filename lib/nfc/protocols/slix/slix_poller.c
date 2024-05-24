#include "slix_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "SlixPoller"

#define SLIX_POLLER_BUF_SIZE (64U)

typedef NfcCommand (*SlixPollerStateHandler)(SlixPoller* instance);

const SlixData* slix_poller_get_data(SlixPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

static SlixPoller* slix_poller_alloc(Iso15693_3Poller* iso15693_3_poller) {
    SlixPoller* instance = malloc(sizeof(SlixPoller));
    instance->iso15693_3_poller = iso15693_3_poller;
    instance->data = slix_alloc();
    instance->tx_buffer = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(SLIX_POLLER_BUF_SIZE);

    instance->slix_event.data = &instance->slix_event_data;

    instance->general_event.protocol = NfcProtocolSlix;
    instance->general_event.event_data = &instance->slix_event;
    instance->general_event.instance = instance;

    return instance;
}

static void slix_poller_free(SlixPoller* instance) {
    furi_assert(instance);

    slix_free(instance->data);
    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    free(instance);
}

static NfcCommand slix_poller_handler_idle(SlixPoller* instance) {
    iso15693_3_copy(
        instance->data->iso15693_3_data, iso15693_3_poller_get_data(instance->iso15693_3_poller));
    instance->type = slix_get_type(instance->data);
    if(instance->type >= SlixTypeCount) {
        instance->error = SlixErrorNotSupported;
        instance->poller_state = SlixPollerStateError;
    } else {
        instance->poller_state = SlixPollerStateGetNxpSysInfo;
    }
    return NfcCommandContinue;
}

static NfcCommand slix_poller_handler_get_nfc_system_info(SlixPoller* instance) {
    if(slix_type_has_features(instance->type, SLIX_TYPE_FEATURE_NFC_SYSTEM_INFO)) {
        instance->error = slix_poller_get_nxp_system_info(instance, &instance->data->system_info);
        if(instance->error == SlixErrorNone) {
            instance->poller_state = SlixPollerStateReadSignature;
        } else {
            instance->poller_state = SlixPollerStateError;
        }
    } else {
        instance->poller_state = SlixPollerStateReadSignature;
    }

    return NfcCommandContinue;
}

static NfcCommand slix_poller_handler_read_signature(SlixPoller* instance) {
    if(slix_type_has_features(instance->type, SLIX_TYPE_FEATURE_SIGNATURE)) {
        instance->error = slix_poller_read_signature(instance, &instance->data->signature);
        if(instance->error == SlixErrorNone) {
            instance->poller_state = SlixPollerStateCheckPrivacyPassword;
        } else {
            instance->poller_state = SlixPollerStateError;
        }
    } else {
        instance->poller_state = SlixPollerStateCheckPrivacyPassword;
    }

    return NfcCommandContinue;
}

static NfcCommand slix_poller_handler_check_privacy_password(SlixPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    do {
        if(!slix_type_has_features(instance->type, SLIX_TYPE_FEATURE_PRIVACY)) {
            instance->poller_state = SlixPollerStateReady;
            break;
        }
        if(instance->privacy_password_checked) {
            instance->poller_state = SlixPollerStateReady;
            break;
        }

        instance->slix_event.type = SlixPollerEventTypePrivacyUnlockRequest;
        command = instance->callback(instance->general_event, instance->context);

        if(!instance->slix_event_data.privacy_password.password_set) {
            instance->poller_state = SlixPollerStateReady;
            break;
        }

        SlixPassword pwd = instance->slix_event_data.privacy_password.password;
        FURI_LOG_I(TAG, "Trying to check privacy password: %08lX", pwd);

        instance->error = slix_poller_get_random_number(instance, &instance->random_number);
        if(instance->error != SlixErrorNone) {
            instance->poller_state = SlixPollerStateReady;
            break;
        }

        instance->error = slix_poller_set_password(
            instance, SlixPasswordTypePrivacy, pwd, instance->random_number);
        if(instance->error != SlixErrorNone) {
            command = NfcCommandReset;
            break;
        }

        FURI_LOG_I(TAG, "Found privacy password");
        instance->data->passwords[SlixPasswordTypePrivacy] = pwd;
        instance->privacy_password_checked = true;
        instance->poller_state = SlixPollerStateReady;
    } while(false);

    return command;
}

static NfcCommand slix_poller_handler_privacy_unlock(SlixPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    instance->poller_state = SlixPollerStateError;

    instance->slix_event.type = SlixPollerEventTypePrivacyUnlockRequest;
    command = instance->callback(instance->general_event, instance->context);

    bool slix_unlocked = false;
    do {
        if(!instance->slix_event_data.privacy_password.password_set) break;
        SlixPassword pwd = instance->slix_event_data.privacy_password.password;
        FURI_LOG_I(TAG, "Trying to disable privacy mode with password: %08lX", pwd);

        instance->error = slix_poller_get_random_number(instance, &instance->random_number);
        if(instance->error != SlixErrorNone) break;

        instance->error = slix_poller_set_password(
            instance, SlixPasswordTypePrivacy, pwd, instance->random_number);
        if(instance->error != SlixErrorNone) {
            command = NfcCommandReset;
            break;
        }

        FURI_LOG_I(TAG, "Privacy mode disabled");
        instance->data->passwords[SlixPasswordTypePrivacy] = pwd;
        instance->privacy_password_checked = true;
        instance->poller_state = SlixPollerStateIdle;
        slix_unlocked = true;
    } while(false);

    if(!slix_unlocked) {
        instance->error = SlixErrorTimeout;
        instance->poller_state = SlixPollerStateError;
        furi_delay_ms(100);
    }

    return command;
}

static NfcCommand slix_poller_handler_error(SlixPoller* instance) {
    instance->slix_event_data.error = instance->error;
    instance->slix_event.type = SlixPollerEventTypeError;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->poller_state = SlixPollerStateIdle;
    return command;
}

static NfcCommand slix_poller_handler_ready(SlixPoller* instance) {
    instance->slix_event.type = SlixPollerEventTypeReady;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    return command;
}

static const SlixPollerStateHandler slix_poller_state_handler[SlixPollerStateNum] = {
    [SlixPollerStateIdle] = slix_poller_handler_idle,
    [SlixPollerStateError] = slix_poller_handler_error,
    [SlixPollerStateGetNxpSysInfo] = slix_poller_handler_get_nfc_system_info,
    [SlixPollerStateReadSignature] = slix_poller_handler_read_signature,
    [SlixPollerStateCheckPrivacyPassword] = slix_poller_handler_check_privacy_password,
    [SlixPollerStatePrivacyUnlock] = slix_poller_handler_privacy_unlock,
    [SlixPollerStateReady] = slix_poller_handler_ready,
};

static void
    slix_poller_set_callback(SlixPoller* instance, NfcGenericCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand slix_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso15693_3);

    SlixPoller* instance = context;
    furi_assert(instance);
    furi_assert(instance->callback);

    Iso15693_3PollerEvent* iso15693_3_event = event.event_data;
    furi_assert(iso15693_3_event);

    NfcCommand command = NfcCommandContinue;

    if(iso15693_3_event->type == Iso15693_3PollerEventTypeReady) {
        command = slix_poller_state_handler[instance->poller_state](instance);
    } else if(iso15693_3_event->type == Iso15693_3PollerEventTypeError) {
        instance->poller_state = SlixPollerStatePrivacyUnlock;
        command = slix_poller_state_handler[instance->poller_state](instance);
    }

    return command;
}

static bool slix_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso15693_3);

    SlixPoller* instance = context;
    furi_assert(instance);

    const Iso15693_3PollerEvent* iso15693_3_event = event.event_data;
    furi_assert(iso15693_3_event);
    iso15693_3_copy(
        instance->data->iso15693_3_data, iso15693_3_poller_get_data(instance->iso15693_3_poller));

    bool protocol_detected = false;

    if(iso15693_3_event->type == Iso15693_3PollerEventTypeReady) {
        protocol_detected = (slix_get_type(instance->data) < SlixTypeCount);
    }

    return protocol_detected;
}

const NfcPollerBase nfc_poller_slix = {
    .alloc = (NfcPollerAlloc)slix_poller_alloc,
    .free = (NfcPollerFree)slix_poller_free,
    .set_callback = (NfcPollerSetCallback)slix_poller_set_callback,
    .run = (NfcPollerRun)slix_poller_run,
    .detect = (NfcPollerDetect)slix_poller_detect,
    .get_data = (NfcPollerGetData)slix_poller_get_data,
};
