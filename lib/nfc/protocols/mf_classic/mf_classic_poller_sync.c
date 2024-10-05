#include "mf_classic_poller_sync.h"
#include "mf_classic_poller_i.h"

#include <nfc/nfc_poller.h>

#include <furi.h>

#define TAG "MfClassicPoller"

#define MF_CLASSIC_POLLER_COMPLETE_EVENT (1UL << 0)

typedef enum {
    MfClassicPollerCmdTypeCollectNt,
    MfClassicPollerCmdTypeAuth,
    MfClassicPollerCmdTypeReadBlock,
    MfClassicPollerCmdTypeWriteBlock,
    MfClassicPollerCmdTypeReadValue,
    MfClassicPollerCmdTypeChangeValue,

    MfClassicPollerCmdTypeNum,
} MfClassicPollerCmdType;

typedef struct {
    MfClassicPollerCmdType cmd_type;
    FuriThreadId thread_id;
    MfClassicError error;
    MfClassicPollerContextData data;
} MfClassicPollerContext;

typedef MfClassicError (
    *MfClassicPollerCmdHandler)(MfClassicPoller* poller, MfClassicPollerContextData* data);

static MfClassicError mf_classic_poller_collect_nt_handler(
    MfClassicPoller* poller,
    MfClassicPollerContextData* data) {
    return mf_classic_poller_get_nt(
        poller,
        data->collect_nt_context.block,
        data->collect_nt_context.key_type,
        &data->collect_nt_context.nt,
        false);
}

static MfClassicError
    mf_classic_poller_auth_handler(MfClassicPoller* poller, MfClassicPollerContextData* data) {
    return mf_classic_poller_auth(
        poller,
        data->auth_context.block_num,
        &data->auth_context.key,
        data->auth_context.key_type,
        &data->auth_context,
        false);
}

static MfClassicError mf_classic_poller_read_block_handler(
    MfClassicPoller* poller,
    MfClassicPollerContextData* data) {
    MfClassicError error = MfClassicErrorNone;

    do {
        error = mf_classic_poller_auth(
            poller,
            data->read_block_context.block_num,
            &data->read_block_context.key,
            data->read_block_context.key_type,
            NULL,
            false);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_read_block(
            poller, data->read_block_context.block_num, &data->read_block_context.block);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_halt(poller);
        if(error != MfClassicErrorNone) break;

    } while(false);

    return error;
}

static MfClassicError mf_classic_poller_write_block_handler(
    MfClassicPoller* poller,
    MfClassicPollerContextData* data) {
    MfClassicError error = MfClassicErrorNone;

    do {
        error = mf_classic_poller_auth(
            poller,
            data->read_block_context.block_num,
            &data->read_block_context.key,
            data->read_block_context.key_type,
            NULL,
            false);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_write_block(
            poller, data->write_block_context.block_num, &data->write_block_context.block);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_halt(poller);
        if(error != MfClassicErrorNone) break;

    } while(false);

    return error;
}

static MfClassicError mf_classic_poller_read_value_handler(
    MfClassicPoller* poller,
    MfClassicPollerContextData* data) {
    MfClassicError error = MfClassicErrorNone;

    do {
        error = mf_classic_poller_auth(
            poller,
            data->read_value_context.block_num,
            &data->read_value_context.key,
            data->read_value_context.key_type,
            NULL,
            false);
        if(error != MfClassicErrorNone) break;

        MfClassicBlock block = {};
        error = mf_classic_poller_read_block(poller, data->read_value_context.block_num, &block);
        if(error != MfClassicErrorNone) break;

        if(!mf_classic_block_to_value(&block, &data->read_value_context.value, NULL)) {
            error = MfClassicErrorProtocol;
            break;
        }

        error = mf_classic_poller_halt(poller);
        if(error != MfClassicErrorNone) break;

    } while(false);

    return error;
}

static MfClassicError mf_classic_poller_change_value_handler(
    MfClassicPoller* poller,
    MfClassicPollerContextData* data) {
    MfClassicError error = MfClassicErrorNone;

    do {
        error = mf_classic_poller_auth(
            poller,
            data->change_value_context.block_num,
            &data->change_value_context.key,
            data->change_value_context.key_type,
            NULL,
            false);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_value_cmd(
            poller,
            data->change_value_context.block_num,
            data->change_value_context.value_cmd,
            data->change_value_context.data);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_value_transfer(poller, data->change_value_context.block_num);
        if(error != MfClassicErrorNone) break;

        MfClassicBlock block = {};
        error = mf_classic_poller_read_block(poller, data->change_value_context.block_num, &block);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_halt(poller);
        if(error != MfClassicErrorNone) break;

        if(!mf_classic_block_to_value(&block, &data->change_value_context.new_value, NULL)) {
            error = MfClassicErrorProtocol;
            break;
        }

    } while(false);

    return error;
}

static const MfClassicPollerCmdHandler mf_classic_poller_cmd_handlers[MfClassicPollerCmdTypeNum] = {
    [MfClassicPollerCmdTypeCollectNt] = mf_classic_poller_collect_nt_handler,
    [MfClassicPollerCmdTypeAuth] = mf_classic_poller_auth_handler,
    [MfClassicPollerCmdTypeReadBlock] = mf_classic_poller_read_block_handler,
    [MfClassicPollerCmdTypeWriteBlock] = mf_classic_poller_write_block_handler,
    [MfClassicPollerCmdTypeReadValue] = mf_classic_poller_read_value_handler,
    [MfClassicPollerCmdTypeChangeValue] = mf_classic_poller_change_value_handler,
};

static NfcCommand mf_classic_poller_cmd_callback(NfcGenericEventEx event, void* context) {
    furi_assert(event.poller);
    furi_assert(event.parent_event_data);
    furi_assert(context);

    MfClassicPollerContext* poller_context = context;
    Iso14443_3aPollerEvent* iso14443_3a_event = event.parent_event_data;
    MfClassicPoller* mfc_poller = event.poller;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        poller_context->error = mf_classic_poller_cmd_handlers[poller_context->cmd_type](
            mfc_poller, &poller_context->data);
    } else if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeError) {
        poller_context->error = mf_classic_process_error(iso14443_3a_event->data->error);
    }

    furi_thread_flags_set(poller_context->thread_id, MF_CLASSIC_POLLER_COMPLETE_EVENT);

    return NfcCommandStop;
}

static MfClassicError mf_classic_poller_cmd_execute(Nfc* nfc, MfClassicPollerContext* poller_ctx) {
    furi_assert(poller_ctx->cmd_type < MfClassicPollerCmdTypeNum);

    poller_ctx->thread_id = furi_thread_get_current_id();

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolMfClassic);
    nfc_poller_start_ex(poller, mf_classic_poller_cmd_callback, poller_ctx);
    furi_thread_flags_wait(MF_CLASSIC_POLLER_COMPLETE_EVENT, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(MF_CLASSIC_POLLER_COMPLETE_EVENT);

    nfc_poller_stop(poller);
    nfc_poller_free(poller);

    return poller_ctx->error;
}

MfClassicError mf_classic_poller_sync_collect_nt(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicNt* nt) {
    furi_check(nfc);

    MfClassicPollerContext poller_context = {
        .cmd_type = MfClassicPollerCmdTypeCollectNt,
        .data.collect_nt_context.block = block_num,
        .data.collect_nt_context.key_type = key_type,
    };

    MfClassicError error = mf_classic_poller_cmd_execute(nfc, &poller_context);

    if(error == MfClassicErrorNone) {
        if(nt) {
            *nt = poller_context.data.collect_nt_context.nt;
        }
    }

    return error;
}

MfClassicError mf_classic_poller_sync_auth(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicAuthContext* data) {
    furi_check(nfc);
    furi_check(key);

    MfClassicPollerContext poller_context = {
        .cmd_type = MfClassicPollerCmdTypeAuth,
        .data.auth_context.block_num = block_num,
        .data.auth_context.key = *key,
        .data.auth_context.key_type = key_type,
    };

    MfClassicError error = mf_classic_poller_cmd_execute(nfc, &poller_context);

    if(error == MfClassicErrorNone) {
        if(data) {
            *data = poller_context.data.auth_context;
        }
    }

    return error;
}

MfClassicError mf_classic_poller_sync_read_block(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicBlock* data) {
    furi_check(nfc);
    furi_check(key);
    furi_check(data);

    MfClassicPollerContext poller_context = {
        .cmd_type = MfClassicPollerCmdTypeReadBlock,
        .data.read_block_context.block_num = block_num,
        .data.read_block_context.key = *key,
        .data.read_block_context.key_type = key_type,
    };

    MfClassicError error = mf_classic_poller_cmd_execute(nfc, &poller_context);

    if(error == MfClassicErrorNone) {
        *data = poller_context.data.read_block_context.block;
    }

    return error;
}

MfClassicError mf_classic_poller_sync_write_block(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicBlock* data) {
    furi_check(nfc);
    furi_check(key);
    furi_check(data);

    MfClassicPollerContext poller_context = {
        .cmd_type = MfClassicPollerCmdTypeWriteBlock,
        .data.write_block_context.block_num = block_num,
        .data.write_block_context.key = *key,
        .data.write_block_context.key_type = key_type,
        .data.write_block_context.block = *data,
    };

    MfClassicError error = mf_classic_poller_cmd_execute(nfc, &poller_context);

    return error;
}

MfClassicError mf_classic_poller_sync_read_value(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    int32_t* value) {
    furi_check(nfc);
    furi_check(key);
    furi_check(value);

    MfClassicPollerContext poller_context = {
        .cmd_type = MfClassicPollerCmdTypeReadValue,
        .data.write_block_context.block_num = block_num,
        .data.write_block_context.key = *key,
        .data.write_block_context.key_type = key_type,
    };

    MfClassicError error = mf_classic_poller_cmd_execute(nfc, &poller_context);

    if(error == MfClassicErrorNone) {
        *value = poller_context.data.read_value_context.value;
    }

    return error;
}

MfClassicError mf_classic_poller_sync_change_value(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    int32_t data,
    int32_t* new_value) {
    furi_check(nfc);
    furi_check(key);
    furi_check(new_value);

    MfClassicValueCommand command = MfClassicValueCommandRestore;
    int32_t command_data = 0;
    if(data > 0) {
        command = MfClassicValueCommandIncrement;
        command_data = data;
    } else if(data < 0) {
        command = MfClassicValueCommandDecrement;
        command_data = -data;
    }

    MfClassicPollerContext poller_context = {
        .cmd_type = MfClassicPollerCmdTypeChangeValue,
        .data.change_value_context.block_num = block_num,
        .data.change_value_context.key = *key,
        .data.change_value_context.key_type = key_type,
        .data.change_value_context.value_cmd = command,
        .data.change_value_context.data = command_data,
    };

    MfClassicError error = mf_classic_poller_cmd_execute(nfc, &poller_context);

    if(error == MfClassicErrorNone) {
        *new_value = poller_context.data.change_value_context.new_value;
    }

    return error;
}

static bool mf_classic_poller_read_get_next_key(
    MfClassicReadContext* read_ctx,
    uint8_t* sector_num,
    MfClassicKey* key,
    MfClassicKeyType* key_type) {
    bool next_key_found = false;

    for(uint8_t i = read_ctx->current_sector; i < MF_CLASSIC_TOTAL_SECTORS_MAX; i++) {
        if(FURI_BIT(read_ctx->keys.key_a_mask, i)) {
            FURI_BIT_CLEAR(read_ctx->keys.key_a_mask, i);
            *key = read_ctx->keys.key_a[i];
            *key_type = MfClassicKeyTypeA;
            *sector_num = i;

            next_key_found = true;
            break;
        }
        if(FURI_BIT(read_ctx->keys.key_b_mask, i)) {
            FURI_BIT_CLEAR(read_ctx->keys.key_b_mask, i);
            *key = read_ctx->keys.key_b[i];
            *key_type = MfClassicKeyTypeB;
            *sector_num = i;

            next_key_found = true;
            read_ctx->current_sector = i;
            break;
        }
    }

    return next_key_found;
}

NfcCommand mf_classic_poller_read_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcCommand command = NfcCommandContinue;
    MfClassicPollerContext* poller_context = context;
    MfClassicPollerEvent* mfc_event = event.event_data;

    if(mfc_event->type == MfClassicPollerEventTypeCardLost) {
        poller_context->error = MfClassicErrorNotPresent;
        command = NfcCommandStop;
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        mfc_event->data->poller_mode.mode = MfClassicPollerModeRead;
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestReadSector) {
        MfClassicPollerEventDataReadSectorRequest* req_data =
            &mfc_event->data->read_sector_request_data;
        MfClassicKey key = {};
        MfClassicKeyType key_type = MfClassicKeyTypeA;
        uint8_t sector_num = 0;
        if(mf_classic_poller_read_get_next_key(
               &poller_context->data.read_context, &sector_num, &key, &key_type)) {
            req_data->sector_num = sector_num;
            req_data->key = key;
            req_data->key_type = key_type;
            req_data->key_provided = true;
        } else {
            req_data->key_provided = false;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_thread_flags_set(poller_context->thread_id, MF_CLASSIC_POLLER_COMPLETE_EVENT);
    }

    return command;
}

MfClassicError
    mf_classic_poller_sync_read(Nfc* nfc, const MfClassicDeviceKeys* keys, MfClassicData* data) {
    furi_check(nfc);
    furi_check(keys);
    furi_check(data);

    MfClassicError error = MfClassicErrorNone;
    MfClassicPollerContext poller_context = {};
    poller_context.thread_id = furi_thread_get_current_id();
    poller_context.data.read_context.keys = *keys;

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolMfClassic);
    nfc_poller_start(poller, mf_classic_poller_read_callback, &poller_context);
    furi_thread_flags_wait(MF_CLASSIC_POLLER_COMPLETE_EVENT, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(MF_CLASSIC_POLLER_COMPLETE_EVENT);

    nfc_poller_stop(poller);

    const MfClassicData* mfc_data = nfc_poller_get_data(poller);
    uint8_t sectors_read = 0;
    uint8_t keys_found = 0;

    mf_classic_get_read_sectors_and_keys(mfc_data, &sectors_read, &keys_found);
    if((sectors_read == 0) && (keys_found == 0)) {
        error = MfClassicErrorNotPresent;
    } else {
        mf_classic_copy(data, mfc_data);
        error = mf_classic_is_card_read(mfc_data) ? MfClassicErrorNone : MfClassicErrorPartialRead;
    }

    nfc_poller_free(poller);

    return error;
}

MfClassicError mf_classic_poller_sync_detect_type(Nfc* nfc, MfClassicType* type) {
    furi_check(nfc);
    furi_check(type);

    MfClassicError error = MfClassicErrorNone;

    const uint8_t mf_classic_verify_block[MfClassicTypeNum] = {
        [MfClassicTypeMini] = 0,
        [MfClassicType1k] = 62,
        [MfClassicType4k] = 254,
    };

    size_t i = 0;
    for(i = 0; i < COUNT_OF(mf_classic_verify_block); i++) {
        error = mf_classic_poller_sync_collect_nt(
            nfc, mf_classic_verify_block[MfClassicTypeNum - i - 1], MfClassicKeyTypeA, NULL);
        if(error == MfClassicErrorNone) {
            *type = MfClassicTypeNum - i - 1;
            break;
        }
    }

    return error;
}
