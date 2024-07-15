#include "st25tb_poller_sync.h"
#include "st25tb_poller_i.h"

#define ST25TB_POLLER_FLAG_COMMAND_COMPLETE (1UL << 0)

typedef enum {
    St25tbPollerCmdTypeDetectType,
    St25tbPollerCmdTypeReadBlock,
    St25tbPollerCmdTypeWriteBlock,

    St25tbPollerCmdTypeNum,
} St25tbPollerCmdType;

typedef struct {
    St25tbType* type;
} St25tbPollerCmdDetectTypeData;

typedef struct {
    St25tbData* data;
} St25tbPollerCmdReadData;

typedef struct {
    uint8_t block_num;
    uint32_t* block;
} St25tbPollerCmdReadBlockData;

typedef struct {
    uint8_t block_num;
    uint32_t block;
} St25tbPollerCmdWriteBlockData;

typedef union {
    St25tbPollerCmdDetectTypeData detect_type;
    St25tbPollerCmdReadData read;
    St25tbPollerCmdReadBlockData read_block;
    St25tbPollerCmdWriteBlockData write_block;
} St25tbPollerCmdData;

typedef struct {
    FuriThreadId thread_id;
    St25tbError error;
    St25tbPollerCmdType cmd_type;
    St25tbPollerCmdData cmd_data;
} St25tbPollerSyncContext;

typedef St25tbError (*St25tbPollerCmdHandler)(St25tbPoller* poller, St25tbPollerCmdData* data);

static St25tbError st25tb_poller_detect_handler(St25tbPoller* poller, St25tbPollerCmdData* data) {
    uint8_t uid[ST25TB_UID_SIZE];
    St25tbError error = st25tb_poller_get_uid(poller, uid);
    if(error == St25tbErrorNone) {
        *data->detect_type.type = st25tb_get_type_from_uid(uid);
    }
    return error;
}

static St25tbError
    st25tb_poller_read_block_handler(St25tbPoller* poller, St25tbPollerCmdData* data) {
    return st25tb_poller_read_block(poller, data->read_block.block, data->read_block.block_num);
}

static St25tbError
    st25tb_poller_write_block_handler(St25tbPoller* poller, St25tbPollerCmdData* data) {
    return st25tb_poller_write_block(poller, data->write_block.block, data->write_block.block_num);
}

static St25tbPollerCmdHandler st25tb_poller_cmd_handlers[St25tbPollerCmdTypeNum] = {
    [St25tbPollerCmdTypeDetectType] = st25tb_poller_detect_handler,
    [St25tbPollerCmdTypeReadBlock] = st25tb_poller_read_block_handler,
    [St25tbPollerCmdTypeWriteBlock] = st25tb_poller_write_block_handler,
};

static NfcCommand st25tb_poller_cmd_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolSt25tb);

    St25tbPollerSyncContext* poller_context = context;
    St25tbPoller* st25tb_poller = event.instance;
    St25tbPollerEvent* st25tb_event = event.event_data;

    if(st25tb_event->type == St25tbPollerEventTypeReady) {
        poller_context->error = st25tb_poller_cmd_handlers[poller_context->cmd_type](
            st25tb_poller, &poller_context->cmd_data);
    } else {
        poller_context->error = st25tb_event->data->error;
    }

    furi_thread_flags_set(poller_context->thread_id, ST25TB_POLLER_FLAG_COMMAND_COMPLETE);

    return NfcCommandStop;
}

static St25tbError st25tb_poller_cmd_execute(Nfc* nfc, St25tbPollerSyncContext* poller_ctx) {
    furi_assert(nfc);
    furi_assert(poller_ctx->cmd_type < St25tbPollerCmdTypeNum);
    poller_ctx->thread_id = furi_thread_get_current_id();

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolSt25tb);
    nfc_poller_start(poller, st25tb_poller_cmd_callback, poller_ctx);
    furi_thread_flags_wait(ST25TB_POLLER_FLAG_COMMAND_COMPLETE, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(ST25TB_POLLER_FLAG_COMMAND_COMPLETE);

    nfc_poller_stop(poller);
    nfc_poller_free(poller);

    return poller_ctx->error;
}

St25tbError st25tb_poller_sync_read_block(Nfc* nfc, uint8_t block_num, uint32_t* block) {
    furi_check(nfc);
    furi_check(block);
    St25tbPollerSyncContext poller_context = {
        .cmd_type = St25tbPollerCmdTypeReadBlock,
        .cmd_data =
            {
                .read_block =
                    {
                        .block = block,
                        .block_num = block_num,
                    },
            },
    };
    return st25tb_poller_cmd_execute(nfc, &poller_context);
}

St25tbError st25tb_poller_sync_write_block(Nfc* nfc, uint8_t block_num, uint32_t block) {
    furi_check(nfc);
    St25tbPollerSyncContext poller_context = {
        .cmd_type = St25tbPollerCmdTypeWriteBlock,
        .cmd_data =
            {
                .write_block =
                    {
                        .block = block,
                        .block_num = block_num,
                    },
            },
    };
    return st25tb_poller_cmd_execute(nfc, &poller_context);
}

St25tbError st25tb_poller_sync_detect_type(Nfc* nfc, St25tbType* type) {
    furi_check(nfc);
    furi_check(type);
    St25tbPollerSyncContext poller_context = {
        .cmd_type = St25tbPollerCmdTypeDetectType,
        .cmd_data =
            {
                .detect_type =
                    {
                        .type = type,
                    },
            },
    };
    return st25tb_poller_cmd_execute(nfc, &poller_context);
}

static NfcCommand nfc_scene_read_poller_callback_st25tb(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolSt25tb);

    St25tbPollerSyncContext* poller_context = context;
    St25tbPollerEvent* st25tb_event = event.event_data;

    NfcCommand command = NfcCommandContinue;
    if(st25tb_event->type == St25tbPollerEventTypeRequestMode) {
        st25tb_event->data->mode_request.mode = St25tbPollerModeRead;
    } else if(
        st25tb_event->type == St25tbPollerEventTypeSuccess ||
        st25tb_event->type == St25tbPollerEventTypeFailure) {
        if(st25tb_event->type == St25tbPollerEventTypeSuccess) {
            memcpy(
                poller_context->cmd_data.read.data,
                st25tb_poller_get_data(event.instance),
                sizeof(St25tbData));
        } else {
            poller_context->error = st25tb_event->data->error;
        }
        command = NfcCommandStop;
        furi_thread_flags_set(poller_context->thread_id, ST25TB_POLLER_FLAG_COMMAND_COMPLETE);
    }

    return command;
}

St25tbError st25tb_poller_sync_read(Nfc* nfc, St25tbData* data) {
    furi_check(nfc);
    furi_check(data);

    St25tbPollerSyncContext poller_context = {
        .thread_id = furi_thread_get_current_id(),
        .cmd_data =
            {
                .read =
                    {
                        .data = data,
                    },
            },
    };

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolSt25tb);
    nfc_poller_start(poller, nfc_scene_read_poller_callback_st25tb, &poller_context);
    furi_thread_flags_wait(ST25TB_POLLER_FLAG_COMMAND_COMPLETE, FuriFlagWaitAny, FuriWaitForever);
    furi_thread_flags_clear(ST25TB_POLLER_FLAG_COMMAND_COMPLETE);

    nfc_poller_stop(poller);
    nfc_poller_free(poller);

    return poller_context.error;
}
