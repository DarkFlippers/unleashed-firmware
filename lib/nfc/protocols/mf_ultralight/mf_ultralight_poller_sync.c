#include "mf_ultralight_poller_sync.h"
#include "mf_ultralight_poller_i.h"

#include <nfc/nfc_poller.h>

#include <furi.h>

#define MF_ULTRALIGHT_POLLER_COMPLETE_EVENT (1UL << 0)

typedef enum {
    MfUltralightPollerCmdTypeReadPage,
    MfUltralightPollerCmdTypeWritePage,
    MfUltralightPollerCmdTypeReadVersion,
    MfUltralightPollerCmdTypeReadSignature,
    MfUltralightPollerCmdTypeReadCounter,
    MfUltralightPollerCmdTypeReadTearingFlag,

    MfUltralightPollerCmdTypeNum,
} MfUltralightPollerCmdType;

typedef struct {
    MfUltralightPollerCmdType cmd_type;
    FuriThreadId thread_id;
    MfUltralightError error;
    MfUltralightPollerContextData data;
    const MfUltralightPollerAuthContext* auth_context;
} MfUltralightPollerContext;

typedef MfUltralightError (*MfUltralightPollerCmdHandler)(
    MfUltralightPoller* poller,
    MfUltralightPollerContextData* data);

MfUltralightError mf_ultralight_poller_read_page_handler(
    MfUltralightPoller* poller,
    MfUltralightPollerContextData* data) {
    return mf_ultralight_poller_read_page(poller, data->read_cmd.start_page, &data->read_cmd.data);
}

MfUltralightError mf_ultralight_poller_write_page_handler(
    MfUltralightPoller* poller,
    MfUltralightPollerContextData* data) {
    return mf_ultralight_poller_write_page(
        poller, data->write_cmd.page_to_write, &data->write_cmd.page);
}

MfUltralightError mf_ultralight_poller_read_version_handler(
    MfUltralightPoller* poller,
    MfUltralightPollerContextData* data) {
    return mf_ultralight_poller_read_version(poller, &data->version);
}

MfUltralightError mf_ultralight_poller_read_signature_handler(
    MfUltralightPoller* poller,
    MfUltralightPollerContextData* data) {
    return mf_ultralight_poller_read_signature(poller, &data->signature);
}

MfUltralightError mf_ultralight_poller_read_counter_handler(
    MfUltralightPoller* poller,
    MfUltralightPollerContextData* data) {
    return mf_ultralight_poller_read_counter(
        poller, data->counter_cmd.counter_num, &data->counter_cmd.data);
}

MfUltralightError mf_ultralight_poller_read_tearing_flag_handler(
    MfUltralightPoller* poller,
    MfUltralightPollerContextData* data) {
    return mf_ultralight_poller_read_tearing_flag(
        poller, data->tearing_flag_cmd.tearing_flag_num, &data->tearing_flag_cmd.data);
}

static const MfUltralightPollerCmdHandler
    mf_ultralight_poller_cmd_handlers[MfUltralightPollerCmdTypeNum] = {
        [MfUltralightPollerCmdTypeReadPage] = mf_ultralight_poller_read_page_handler,
        [MfUltralightPollerCmdTypeWritePage] = mf_ultralight_poller_write_page_handler,
        [MfUltralightPollerCmdTypeReadVersion] = mf_ultralight_poller_read_version_handler,
        [MfUltralightPollerCmdTypeReadSignature] = mf_ultralight_poller_read_signature_handler,
        [MfUltralightPollerCmdTypeReadCounter] = mf_ultralight_poller_read_counter_handler,
        [MfUltralightPollerCmdTypeReadTearingFlag] =
            mf_ultralight_poller_read_tearing_flag_handler,
};

static NfcCommand mf_ultralight_poller_cmd_callback(NfcGenericEventEx event, void* context) {
    furi_assert(event.poller);
    furi_assert(event.parent_event_data);
    furi_assert(context);

    MfUltralightPollerContext* poller_context = context;
    Iso14443_3aPollerEvent* iso14443_3a_event = event.parent_event_data;
    MfUltralightPoller* mfu_poller = event.poller;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        poller_context->error = mf_ultralight_poller_cmd_handlers[poller_context->cmd_type](
            mfu_poller, &poller_context->data);
    } else if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeError) {
        poller_context->error = mf_ultralight_process_error(iso14443_3a_event->data->error);
    }

    furi_thread_flags_set(poller_context->thread_id, MF_ULTRALIGHT_POLLER_COMPLETE_EVENT);

    return NfcCommandStop;
}

static MfUltralightError
    mf_ultralight_poller_cmd_execute(Nfc* nfc, MfUltralightPollerContext* poller_ctx) {
    furi_assert(poller_ctx->cmd_type < MfUltralightPollerCmdTypeNum);

    poller_ctx->thread_id = furi_thread_get_current_id();

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolMfUltralight);
    nfc_poller_start_ex(poller, mf_ultralight_poller_cmd_callback, poller_ctx);
    furi_thread_flags_wait(MF_ULTRALIGHT_POLLER_COMPLETE_EVENT, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(MF_ULTRALIGHT_POLLER_COMPLETE_EVENT);

    nfc_poller_stop(poller);
    nfc_poller_free(poller);

    return poller_ctx->error;
}

MfUltralightError
    mf_ultralight_poller_sync_read_page(Nfc* nfc, uint16_t page, MfUltralightPage* data) {
    furi_check(nfc);
    furi_check(data);

    MfUltralightPollerContext poller_context = {
        .cmd_type = MfUltralightPollerCmdTypeReadPage,
        .data.read_cmd.start_page = page,
    };

    MfUltralightError error = mf_ultralight_poller_cmd_execute(nfc, &poller_context);

    if(error == MfUltralightErrorNone) {
        *data = poller_context.data.read_cmd.data.page[0];
    }

    return error;
}

MfUltralightError
    mf_ultralight_poller_sync_write_page(Nfc* nfc, uint16_t page, MfUltralightPage* data) {
    furi_check(nfc);
    furi_check(data);

    MfUltralightPollerContext poller_context = {
        .cmd_type = MfUltralightPollerCmdTypeWritePage,
        .data.write_cmd =
            {
                .page_to_write = page,
                .page = *data,
            },
    };

    MfUltralightError error = mf_ultralight_poller_cmd_execute(nfc, &poller_context);

    return error;
}

MfUltralightError mf_ultralight_poller_sync_read_version(Nfc* nfc, MfUltralightVersion* data) {
    furi_check(nfc);
    furi_check(data);

    MfUltralightPollerContext poller_context = {
        .cmd_type = MfUltralightPollerCmdTypeReadVersion,
    };

    MfUltralightError error = mf_ultralight_poller_cmd_execute(nfc, &poller_context);

    if(error == MfUltralightErrorNone) {
        *data = poller_context.data.version;
    }

    return error;
}

MfUltralightError mf_ultralight_poller_sync_read_signature(Nfc* nfc, MfUltralightSignature* data) {
    furi_check(nfc);
    furi_check(data);

    MfUltralightPollerContext poller_context = {
        .cmd_type = MfUltralightPollerCmdTypeReadSignature,
    };

    MfUltralightError error = mf_ultralight_poller_cmd_execute(nfc, &poller_context);

    if(error == MfUltralightErrorNone) {
        *data = poller_context.data.signature;
    }

    return error;
}

MfUltralightError mf_ultralight_poller_sync_read_counter(
    Nfc* nfc,
    uint8_t counter_num,
    MfUltralightCounter* data) {
    furi_check(nfc);
    furi_check(data);

    MfUltralightPollerContext poller_context = {
        .cmd_type = MfUltralightPollerCmdTypeReadCounter,
        .data.counter_cmd.counter_num = counter_num,
    };

    MfUltralightError error = mf_ultralight_poller_cmd_execute(nfc, &poller_context);

    if(error == MfUltralightErrorNone) {
        *data = poller_context.data.counter_cmd.data;
    }

    return error;
}

MfUltralightError mf_ultralight_poller_sync_read_tearing_flag(
    Nfc* nfc,
    uint8_t flag_num,
    MfUltralightTearingFlag* data) {
    furi_check(nfc);
    furi_check(data);

    MfUltralightPollerContext poller_context = {
        .cmd_type = MfUltralightPollerCmdTypeReadTearingFlag,
        .data.tearing_flag_cmd.tearing_flag_num = flag_num,
    };

    MfUltralightError error = mf_ultralight_poller_cmd_execute(nfc, &poller_context);

    if(error == MfUltralightErrorNone) {
        *data = poller_context.data.tearing_flag_cmd.data;
    }

    return error;
}

static NfcCommand mf_ultralight_poller_read_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.instance);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfUltralight);

    NfcCommand command = NfcCommandContinue;
    MfUltralightPollerContext* poller_context = context;
    MfUltralightPoller* mfu_poller = event.instance;
    MfUltralightPollerEvent* mfu_event = event.event_data;

    if(mfu_event->type == MfUltralightPollerEventTypeReadSuccess) {
        mf_ultralight_copy(poller_context->data.data, mf_ultralight_poller_get_data(mfu_poller));
        poller_context->error = MfUltralightErrorNone;
        command = NfcCommandStop;
    } else if(mfu_event->type == MfUltralightPollerEventTypeReadFailed) {
        poller_context->error = mfu_event->data->error;
        command = NfcCommandStop;
    } else if(mfu_event->type == MfUltralightPollerEventTypeAuthRequest) {
        if(poller_context->auth_context != NULL) {
            mfu_event->data->auth_context = *poller_context->auth_context;
        } else {
            mfu_event->data->auth_context.skip_auth = true;
            if(mfu_poller->data->type == MfUltralightTypeMfulC) {
                mfu_event->data->auth_context.skip_auth = false;
                memset(
                    mfu_poller->auth_context.tdes_key.data,
                    0x00,
                    MF_ULTRALIGHT_C_AUTH_DES_KEY_SIZE);
            }
        }
    }

    if(command == NfcCommandStop) {
        furi_thread_flags_set(poller_context->thread_id, MF_ULTRALIGHT_POLLER_COMPLETE_EVENT);
    }

    return command;
}

MfUltralightError mf_ultralight_poller_sync_read_card(
    Nfc* nfc,
    MfUltralightData* data,
    const MfUltralightPollerAuthContext* auth_context) {
    furi_check(nfc);
    furi_check(data);

    MfUltralightPollerContext poller_context = {};
    poller_context.thread_id = furi_thread_get_current_id();
    poller_context.data.data = mf_ultralight_alloc();
    poller_context.auth_context = auth_context;

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolMfUltralight);
    nfc_poller_start(poller, mf_ultralight_poller_read_callback, &poller_context);
    furi_thread_flags_wait(MF_ULTRALIGHT_POLLER_COMPLETE_EVENT, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(MF_ULTRALIGHT_POLLER_COMPLETE_EVENT);

    nfc_poller_stop(poller);
    nfc_poller_free(poller);

    if(poller_context.error == MfUltralightErrorNone) {
        mf_ultralight_copy(data, poller_context.data.data);
    }

    mf_ultralight_free(poller_context.data.data);

    return poller_context.error;
}
