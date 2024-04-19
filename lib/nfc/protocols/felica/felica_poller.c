#include "felica_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>
#include <furi_hal.h>

#define TAG "FelicaPoller"

typedef NfcCommand (*FelicaPollerReadHandler)(FelicaPoller* instance);

const FelicaData* felica_poller_get_data(FelicaPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static FelicaPoller* felica_poller_alloc(Nfc* nfc) {
    furi_assert(nfc);

    FelicaPoller* instance = malloc(sizeof(FelicaPoller));
    instance->nfc = nfc;
    instance->tx_buffer = bit_buffer_alloc(FELICA_POLLER_MAX_BUFFER_SIZE);
    instance->rx_buffer = bit_buffer_alloc(FELICA_POLLER_MAX_BUFFER_SIZE);

    nfc_config(instance->nfc, NfcModePoller, NfcTechFelica);
    nfc_set_guard_time_us(instance->nfc, FELICA_GUARD_TIME_US);
    nfc_set_fdt_poll_fc(instance->nfc, FELICA_FDT_POLL_FC);
    nfc_set_fdt_poll_poll_us(instance->nfc, FELICA_POLL_POLL_MIN_US);

    mbedtls_des3_init(&instance->auth.des_context);
    instance->data = felica_alloc();

    instance->felica_event.data = &instance->felica_event_data;
    instance->general_event.protocol = NfcProtocolFelica;
    instance->general_event.event_data = &instance->felica_event;
    instance->general_event.instance = instance;

    return instance;
}

static void felica_poller_free(FelicaPoller* instance) {
    furi_assert(instance);

    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(instance->data);

    mbedtls_des3_free(&instance->auth.des_context);
    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    felica_free(instance->data);
    free(instance);
}

static void
    felica_poller_set_callback(FelicaPoller* instance, NfcGenericCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

NfcCommand felica_poller_state_handler_idle(FelicaPoller* instance) {
    FURI_LOG_D(TAG, "Idle");
    felica_reset(instance->data);
    instance->state = FelicaPollerStateActivated;

    return NfcCommandContinue;
}

NfcCommand felica_poller_state_handler_activate(FelicaPoller* instance) {
    FURI_LOG_D(TAG, "Activate");

    NfcCommand command = NfcCommandContinue;

    FelicaError error = felica_poller_activate(instance, instance->data);
    if(error == FelicaErrorNone) {
        furi_hal_random_fill_buf(instance->data->data.fs.rc.data, FELICA_DATA_BLOCK_SIZE);

        instance->felica_event.type = FelicaPollerEventTypeRequestAuthContext;
        instance->felica_event_data.auth_context = &instance->auth.context;

        instance->callback(instance->general_event, instance->context);

        bool skip_auth = instance->auth.context.skip_auth;
        instance->state = skip_auth ? FelicaPollerStateReadBlocks :
                                      FelicaPollerStateAuthenticateInternal;
    } else if(error != FelicaErrorTimeout) {
        instance->felica_event.type = FelicaPollerEventTypeError;
        instance->felica_event_data.error = error;
        instance->state = FelicaPollerStateReadFailed;
    }
    return command;
}

NfcCommand felica_poller_state_handler_auth_internal(FelicaPoller* instance) {
    FURI_LOG_D(TAG, "Auth Internal");

    felica_calculate_session_key(
        &instance->auth.des_context,
        instance->auth.context.card_key.data,
        instance->data->data.fs.rc.data,
        instance->auth.session_key.data);

    instance->state = FelicaPollerStateReadBlocks;

    uint8_t blocks[3] = {FELICA_BLOCK_INDEX_RC, 0, 0};
    FelicaPollerWriteCommandResponse* tx_resp;
    do {
        FelicaError error = felica_poller_write_blocks(
            instance, 1, blocks, instance->data->data.fs.rc.data, &tx_resp);
        if((error != FelicaErrorNone) || (tx_resp->SF1 != 0) || (tx_resp->SF2 != 0)) break;

        blocks[0] = FELICA_BLOCK_INDEX_ID;
        blocks[1] = FELICA_BLOCK_INDEX_WCNT;
        blocks[2] = FELICA_BLOCK_INDEX_MAC_A;
        FelicaPollerReadCommandResponse* rx_resp;
        error = felica_poller_read_blocks(instance, sizeof(blocks), blocks, &rx_resp);
        if(error != FelicaErrorNone || rx_resp->SF1 != 0 || rx_resp->SF2 != 0) break;

        if(felica_check_mac(
               &instance->auth.des_context,
               instance->auth.session_key.data,
               instance->data->data.fs.rc.data,
               blocks,
               rx_resp->block_count,
               rx_resp->data)) {
            instance->auth.context.auth_status.internal = true;
            instance->data->data.fs.wcnt.SF1 = 0;
            instance->data->data.fs.wcnt.SF2 = 0;
            memcpy(
                instance->data->data.fs.wcnt.data,
                rx_resp->data + FELICA_DATA_BLOCK_SIZE,
                FELICA_DATA_BLOCK_SIZE);
            instance->state = FelicaPollerStateAuthenticateExternal;
        }
    } while(false);

    return NfcCommandContinue;
}

NfcCommand felica_poller_state_handler_auth_external(FelicaPoller* instance) {
    FURI_LOG_D(TAG, "Auth External");
    instance->state = FelicaPollerStateReadBlocks;
    uint8_t blocks[2];

    instance->data->data.fs.state.data[0] = 1;
    FelicaAuthentication* auth = &instance->auth;
    felica_calculate_mac_write(
        &auth->des_context,
        auth->session_key.data,
        instance->data->data.fs.rc.data,
        instance->data->data.fs.wcnt.data,
        instance->data->data.fs.state.data,
        instance->data->data.fs.mac_a.data);

    memcpy(instance->data->data.fs.mac_a.data + 8, instance->data->data.fs.wcnt.data, 3); //-V1086

    uint8_t tx_data[FELICA_DATA_BLOCK_SIZE * 2];
    memcpy(tx_data, instance->data->data.fs.state.data, FELICA_DATA_BLOCK_SIZE);
    memcpy(
        tx_data + FELICA_DATA_BLOCK_SIZE,
        instance->data->data.fs.mac_a.data,
        FELICA_DATA_BLOCK_SIZE);

    do {
        blocks[0] = FELICA_BLOCK_INDEX_STATE;
        blocks[1] = FELICA_BLOCK_INDEX_MAC_A;
        FelicaPollerWriteCommandResponse* tx_resp;
        FelicaError error = felica_poller_write_blocks(instance, 2, blocks, tx_data, &tx_resp);
        if(error != FelicaErrorNone || tx_resp->SF1 != 0 || tx_resp->SF2 != 0) break;

        FelicaPollerReadCommandResponse* rx_resp;
        error = felica_poller_read_blocks(instance, 1, blocks, &rx_resp);
        if(error != FelicaErrorNone || rx_resp->SF1 != 0 || rx_resp->SF2 != 0) break;

        instance->data->data.fs.state.SF1 = 0;
        instance->data->data.fs.state.SF2 = 0;
        memcpy(instance->data->data.fs.state.data, rx_resp->data, FELICA_DATA_BLOCK_SIZE);
        instance->auth.context.auth_status.external = instance->data->data.fs.state.data[0];
    } while(false);
    instance->state = FelicaPollerStateReadBlocks;
    return NfcCommandContinue;
}

NfcCommand felica_poller_state_handler_read_blocks(FelicaPoller* instance) {
    FURI_LOG_D(TAG, "Read Blocks");

    uint8_t block_count = 1;
    uint8_t block_list[4] = {0, 0, 0, 0};
    block_list[0] = instance->block_index;

    instance->block_index++;
    if(instance->block_index == FELICA_BLOCK_INDEX_REG + 1) {
        instance->block_index = FELICA_BLOCK_INDEX_RC;
    } else if(instance->block_index == FELICA_BLOCK_INDEX_MC + 1) {
        instance->block_index = FELICA_BLOCK_INDEX_WCNT;
    } else if(instance->block_index == FELICA_BLOCK_INDEX_STATE + 1) {
        instance->block_index = FELICA_BLOCK_INDEX_CRC_CHECK;
    }

    FelicaPollerReadCommandResponse* response;
    FelicaError error = felica_poller_read_blocks(instance, block_count, block_list, &response);
    if(error == FelicaErrorNone) {
        block_count = (response->SF1 == 0) ? response->block_count : block_count;
        uint8_t* data_ptr =
            instance->data->data.dump + instance->data->blocks_total * sizeof(FelicaBlock);

        *data_ptr++ = response->SF1;
        *data_ptr++ = response->SF2;

        if(response->SF1 == 0) {
            uint8_t* response_data_ptr = response->data;
            instance->data->blocks_read++;
            memcpy(data_ptr, response_data_ptr, FELICA_DATA_BLOCK_SIZE);
        } else {
            memset(data_ptr, 0, FELICA_DATA_BLOCK_SIZE);
        }
        instance->data->blocks_total++;

        if(instance->data->blocks_total == FELICA_BLOCKS_TOTAL_COUNT) {
            instance->state = FelicaPollerStateReadSuccess;
        }
    } else {
        instance->felica_event.type = FelicaPollerEventTypeError;
        instance->felica_event_data.error = error;
        instance->state = FelicaPollerStateReadFailed;
    }

    return NfcCommandContinue;
}

NfcCommand felica_poller_state_handler_read_success(FelicaPoller* instance) {
    FURI_LOG_D(TAG, "Read Success");

    if(!instance->auth.context.auth_status.internal ||
       !instance->auth.context.auth_status.external) {
        instance->data->blocks_read--;
        instance->felica_event.type = FelicaPollerEventTypeIncomplete;
    } else {
        memcpy(
            instance->data->data.fs.ck.data,
            instance->auth.context.card_key.data,
            FELICA_DATA_BLOCK_SIZE);
        instance->felica_event.type = FelicaPollerEventTypeReady;
    }

    instance->felica_event_data.error = FelicaErrorNone;
    return instance->callback(instance->general_event, instance->context);
}

NfcCommand felica_poller_state_handler_read_failed(FelicaPoller* instance) {
    FURI_LOG_D(TAG, "Read Fail");
    instance->callback(instance->general_event, instance->context);

    return NfcCommandStop;
}

static const FelicaPollerReadHandler felica_poller_handler[FelicaPollerStateNum] = {
    [FelicaPollerStateIdle] = felica_poller_state_handler_idle,
    [FelicaPollerStateActivated] = felica_poller_state_handler_activate,
    [FelicaPollerStateAuthenticateInternal] = felica_poller_state_handler_auth_internal,
    [FelicaPollerStateAuthenticateExternal] = felica_poller_state_handler_auth_external,
    [FelicaPollerStateReadBlocks] = felica_poller_state_handler_read_blocks,
    [FelicaPollerStateReadSuccess] = felica_poller_state_handler_read_success,
    [FelicaPollerStateReadFailed] = felica_poller_state_handler_read_failed,
};

static NfcCommand felica_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    FelicaPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(nfc_event->type == NfcEventTypePollerReady) {
        command = felica_poller_handler[instance->state](instance);
    }

    return command;
}

static bool felica_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolInvalid);

    bool protocol_detected = false;
    FelicaPoller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    furi_assert(instance->state == FelicaPollerStateIdle);

    if(nfc_event->type == NfcEventTypePollerReady) {
        FelicaError error = felica_poller_activate(instance, instance->data);
        protocol_detected = (error == FelicaErrorNone);
    }

    return protocol_detected;
}

const NfcPollerBase nfc_poller_felica = {
    .alloc = (NfcPollerAlloc)felica_poller_alloc,
    .free = (NfcPollerFree)felica_poller_free,
    .set_callback = (NfcPollerSetCallback)felica_poller_set_callback,
    .run = (NfcPollerRun)felica_poller_run,
    .detect = (NfcPollerDetect)felica_poller_detect,
    .get_data = (NfcPollerGetData)felica_poller_get_data,
};
