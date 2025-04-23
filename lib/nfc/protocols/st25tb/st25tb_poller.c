#include "st25tb_poller.h"
#include "st25tb_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#define TAG "ST25TBPoller"

typedef NfcCommand (*St25tbPollerStateHandler)(St25tbPoller* instance);

const St25tbData* st25tb_poller_get_data(St25tbPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static St25tbPoller* st25tb_poller_alloc(Nfc* nfc) {
    furi_assert(nfc);

    St25tbPoller* instance = malloc(sizeof(St25tbPoller));
    instance->nfc = nfc;
    instance->state = St25tbPollerStateSelect;
    instance->tx_buffer = bit_buffer_alloc(ST25TB_POLLER_MAX_BUFFER_SIZE);
    instance->rx_buffer = bit_buffer_alloc(ST25TB_POLLER_MAX_BUFFER_SIZE);

    // RF configuration is the same as 14b
    nfc_config(instance->nfc, NfcModePoller, NfcTechIso14443b);
    nfc_set_guard_time_us(instance->nfc, ST25TB_GUARD_TIME_US);
    nfc_set_fdt_poll_fc(instance->nfc, ST25TB_FDT_FC);
    nfc_set_fdt_poll_poll_us(instance->nfc, ST25TB_POLL_POLL_MIN_US);
    instance->data = st25tb_alloc();

    instance->st25tb_event.data = &instance->st25tb_event_data;
    instance->general_event.protocol = NfcProtocolSt25tb;
    instance->general_event.event_data = &instance->st25tb_event;
    instance->general_event.instance = instance;

    return instance;
}

static void st25tb_poller_free(St25tbPoller* instance) {
    furi_assert(instance);

    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(instance->data);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    st25tb_free(instance->data);
    free(instance);
}

static void
    st25tb_poller_set_callback(St25tbPoller* instance, NfcGenericCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand st25tb_poller_select_handler(St25tbPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    do {
        St25tbError error = st25tb_poller_select(instance, NULL);
        if(error != St25tbErrorNone) {
            instance->state = St25tbPollerStateFailure;
            instance->st25tb_event_data.error = error;
            break;
        }

        instance->st25tb_event.type = St25tbPollerEventTypeReady;
        instance->st25tb_event.data->ready.type = instance->data->type;
        command = instance->callback(instance->general_event, instance->context);
        instance->state = St25tbPollerStateRequestMode;
    } while(false);

    return command;
}

static NfcCommand st25tb_poller_request_mode_handler(St25tbPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    instance->st25tb_event.type = St25tbPollerEventTypeRequestMode;
    command = instance->callback(instance->general_event, instance->context);

    St25tbPollerEventDataModeRequest* mode_request_data =
        &instance->st25tb_event_data.mode_request;

    furi_check(mode_request_data->mode < St25tbPollerModeNum);

    if(mode_request_data->mode == St25tbPollerModeRead) {
        instance->state = St25tbPollerStateRead;
        instance->poller_ctx.read.current_block = 0;
    } else {
        instance->state = St25tbPollerStateWrite;
        instance->poller_ctx.write.block_number =
            mode_request_data->params.write_params.block_number;
        instance->poller_ctx.write.block_data = mode_request_data->params.write_params.block_data;
    }

    return command;
}

static NfcCommand st25tb_poller_read_handler(St25tbPoller* instance) {
    St25tbError error = St25tbErrorNone;

    do {
        uint8_t total_blocks = st25tb_get_block_count(instance->data->type);
        uint8_t* current_block = &instance->poller_ctx.read.current_block;
        if(*current_block == total_blocks) {
            error = st25tb_poller_read_block(
                instance, &instance->data->system_otp_block, ST25TB_SYSTEM_OTP_BLOCK);
            if(error != St25tbErrorNone) {
                FURI_LOG_E(TAG, "Failed to read OTP block");
                instance->state = St25tbPollerStateFailure;
                instance->st25tb_event_data.error = error;
                break;
            } else {
                instance->state = St25tbPollerStateSuccess;
                break;
            }
        } else {
            error = st25tb_poller_read_block(
                instance, &instance->data->blocks[*current_block], *current_block);
            if(error != St25tbErrorNone) {
                FURI_LOG_E(TAG, "Failed to read block %d", *current_block);
                instance->state = St25tbPollerStateFailure;
                instance->st25tb_event_data.error = error;
                break;
            }

            *current_block += 1;
        }
    } while(false);

    return NfcCommandContinue;
}

static NfcCommand st25tb_poller_write_handler(St25tbPoller* instance) {
    St25tbPollerWriteContext* write_ctx = &instance->poller_ctx.write;
    St25tbError error =
        st25tb_poller_write_block(instance, write_ctx->block_data, write_ctx->block_number);

    if(error == St25tbErrorNone) {
        instance->state = St25tbPollerStateSuccess;
    } else {
        instance->state = St25tbPollerStateFailure;
        instance->st25tb_event_data.error = error;
    }

    return NfcCommandContinue;
}

NfcCommand st25tb_poller_success_handler(St25tbPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    instance->st25tb_event.type = St25tbPollerEventTypeSuccess;
    command = instance->callback(instance->general_event, instance->context);
    furi_delay_ms(100);
    instance->state = St25tbPollerStateRequestMode;

    return command;
}

NfcCommand st25tb_poller_failure_handler(St25tbPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    instance->st25tb_event.type = St25tbPollerEventTypeFailure;
    command = instance->callback(instance->general_event, instance->context);
    furi_delay_ms(100);
    instance->state = St25tbPollerStateSelect;

    return command;
}

static St25tbPollerStateHandler st25tb_poller_state_handlers[St25tbPollerStateNum] = {
    [St25tbPollerStateSelect] = st25tb_poller_select_handler,
    [St25tbPollerStateRequestMode] = st25tb_poller_request_mode_handler,
    [St25tbPollerStateRead] = st25tb_poller_read_handler,
    [St25tbPollerStateWrite] = st25tb_poller_write_handler,
    [St25tbPollerStateSuccess] = st25tb_poller_success_handler,
    [St25tbPollerStateFailure] = st25tb_poller_failure_handler,
};

static NfcCommand st25tb_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    St25tbPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    furi_assert(instance->state < St25tbPollerStateNum);

    if(nfc_event->type == NfcEventTypePollerReady) {
        command = st25tb_poller_state_handlers[instance->state](instance);
    }

    return command;
}

static bool st25tb_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolInvalid);

    bool protocol_detected = false;
    St25tbPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    furi_assert(instance->state == St25tbPollerStateSelect);

    if(nfc_event->type == NfcEventTypePollerReady) {
        St25tbError error = st25tb_poller_initiate(instance, NULL);
        protocol_detected = (error == St25tbErrorNone);
    }

    return protocol_detected;
}

const NfcPollerBase nfc_poller_st25tb = {
    .alloc = (NfcPollerAlloc)st25tb_poller_alloc,
    .free = (NfcPollerFree)st25tb_poller_free,
    .set_callback = (NfcPollerSetCallback)st25tb_poller_set_callback,
    .run = (NfcPollerRun)st25tb_poller_run,
    .detect = (NfcPollerDetect)st25tb_poller_detect,
    .get_data = (NfcPollerGetData)st25tb_poller_get_data,
};
