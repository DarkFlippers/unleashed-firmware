#include "mf_plus_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "MfPlusPoller"

#define MF_PLUS_BUF_SIZE (64U)
#define MF_PLUS_RESULT_BUF_SIZE (512U)

typedef NfcCommand (*MfPlusPollerReadHandler)(MfPlusPoller* instance);

const MfPlusData* mf_plus_poller_get_data(MfPlusPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

bool mf_plus_poller_get_type_from_iso4(const Iso14443_4aData* iso4_data, MfPlusData* mf_plus_data) {
    furi_assert(iso4_data);
    furi_assert(mf_plus_data);

    switch(iso4_data->iso14443_3a_data->sak) {
    case 0x08:
        if(memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x00\x35\xC7", 7) == 0) {
            // Mifare Plus S 2K SL1
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->size = MfPlusSize2K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;
            return true;
        } else if(memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x01\xBC\xD6", 7) == 0) {
            // Mifare Plus X 2K SL1
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->size = MfPlusSize2K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;
            return true;
        } else if(
            memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x00\xF6\xD1", 7) == 0 ||
            memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x01\xF6\xD1", 7) == 0) {
            // Mifare Plus SE 1K SL1
            mf_plus_data->type = MfPlusTypeSE;
            mf_plus_data->size = MfPlusSize1K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;
            return true;
        } else {
            return false;
        }
    case 0x18:
        if(memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x00\x35\xC7", 7) == 0) {
            // Mifare Plus S 4K SL1
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->size = MfPlusSize4K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;
            return true;
        } else if(memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x01\xBC\xD6", 7) == 0) {
            // Mifare Plus X 4K SL1
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->size = MfPlusSize4K;
            mf_plus_data->security_level = MfPlusSecurityLevel1;
            return true;
        } else {
            return false;
        }
    case 0x20:
        if(memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x00\x35\xC7", 7) == 0) {
            // Mifare Plus S 2/4K SL3
            mf_plus_data->type = MfPlusTypeS;
            mf_plus_data->security_level = MfPlusSecurityLevel3;

            if(iso4_data->iso14443_3a_data->atqa[1] == 0x04) {
                //
                mf_plus_data->size = MfPlusSize2K;
            } else if(iso4_data->iso14443_3a_data->atqa[1] == 0x02) {
                mf_plus_data->size = MfPlusSize4K;
            } else {
                return false;
            }
            return true;

        } else if(memcmp(iso4_data->ats_data.t1_tk, "\xC1\x05\x2F\x2F\x01\xBC\xD6", 7) == 0) {
            mf_plus_data->type = MfPlusTypeX;
            mf_plus_data->security_level = MfPlusSecurityLevel3;

            if(iso4_data->iso14443_3a_data->atqa[1] == 0x04) {
                mf_plus_data->size = MfPlusSize2K;
            } else if(iso4_data->iso14443_3a_data->atqa[1] == 0x02) {
                mf_plus_data->size = MfPlusSize4K;
            } else {
                return false;
            }

            return true;
        } else {
            return false;
        }
    }

    return false;
}

MfPlusPoller* mf_plus_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller) {
    furi_assert(iso14443_4a_poller);

    MfPlusPoller* instance = malloc(sizeof(MfPlusPoller));
    furi_assert(instance);

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
    instance->error = mf_plus_poller_read_version(instance, &instance->data->version);
    if(instance->error == MfPlusErrorNone) {
        instance->state = MfPlusPollerStateReadSuccess;
    } else {
        instance->state = MfPlusPollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_read_failed(MfPlusPoller* instance) {
    furi_assert(instance);
    FURI_LOG_D(TAG, "Read failed");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);
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
    instance->state = MfPlusPollerStateIdle;
    return command;
}

static const MfPlusPollerReadHandler mf_plus_poller_read_handler[MfPlusPollerStateNum] = {
    [MfPlusPollerStateIdle] = mf_plus_poller_handler_idle,
    [MfPlusPollerStateReadVersion] = mf_plus_poller_handler_read_version,
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
    furi_assert(event.protocol = NfcProtocolIso14443_4a);

    MfPlusPoller* instance = context;
    furi_assert(instance);

    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;
    furi_assert(iso14443_4a_event);

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
    furi_assert(event.protocol = NfcProtocolIso14443_4a);

    MfPlusPoller* instance = context;
    furi_assert(instance);

    Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;
    furi_assert(iso14443_4a_event);

    bool detected = false;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        MfPlusVersion version = {};
        const MfPlusError error = mf_plus_poller_read_version(instance, &version);
        if(error == MfPlusErrorNone) {
            if(version.hw_major == 0x02 || version.hw_major == 0x82) {
                detected = true;
            }
        } else {
            detected = mf_plus_poller_get_type_from_iso4(
                iso14443_4a_poller_get_data(instance->iso14443_4a_poller), instance->data);
        }
    }

    return detected;
}

const NfcPollerBase mf_plus_poller = {
    .alloc = (NfcPollerAlloc)mf_plus_poller_alloc,
    .free = (NfcPollerFree)mf_plus_poller_free,
    .set_callback = (NfcPollerSetCallback)mf_plus_poller_set_callback,
    .run = (NfcPollerRun)mf_plus_poller_run,
    .detect = (NfcPollerDetect)mf_plus_poller_detect,
    .get_data = (NfcPollerGetData)mf_plus_poller_get_data,
};
