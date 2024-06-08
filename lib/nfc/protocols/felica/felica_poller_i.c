#include "felica_poller_i.h"

#include <nfc/helpers/felica_crc.h>

#define TAG "FelicaPoller"

static FelicaError felica_poller_process_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return FelicaErrorNone;
    case NfcErrorTimeout:
        return FelicaErrorTimeout;
    default:
        return FelicaErrorNotPresent;
    }
}

FelicaError felica_poller_frame_exchange(
    const FelicaPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_assert(instance);

    const size_t tx_bytes = bit_buffer_get_size_bytes(tx_buffer);
    furi_assert(tx_bytes <= bit_buffer_get_capacity_bytes(instance->tx_buffer) - FELICA_CRC_SIZE);

    felica_crc_append(instance->tx_buffer);

    FelicaError ret = FelicaErrorNone;

    do {
        NfcError error =
            nfc_poller_trx(instance->nfc, instance->tx_buffer, instance->rx_buffer, fwt);
        if(error != NfcErrorNone) {
            ret = felica_poller_process_error(error);
            break;
        }

        bit_buffer_copy(rx_buffer, instance->rx_buffer);
        if(!felica_crc_check(instance->rx_buffer)) {
            ret = FelicaErrorWrongCrc;
            break;
        }

        felica_crc_trim(rx_buffer);
    } while(false);

    return ret;
}

FelicaError felica_poller_polling(
    FelicaPoller* instance,
    const FelicaPollerPollingCommand* cmd,
    FelicaPollerPollingResponse* resp) {
    furi_assert(instance);
    furi_assert(cmd);
    furi_assert(resp);

    FelicaError error = FelicaErrorNone;

    do {
        bit_buffer_set_size_bytes(instance->tx_buffer, 2);
        // Set frame len
        bit_buffer_set_byte(
            instance->tx_buffer, 0, sizeof(FelicaPollerPollingCommand) + FELICA_CRC_SIZE);
        // Set command code
        bit_buffer_set_byte(instance->tx_buffer, 1, FELICA_POLLER_CMD_POLLING_REQ_CODE);
        // Set other data
        bit_buffer_append_bytes(
            instance->tx_buffer, (uint8_t*)cmd, sizeof(FelicaPollerPollingCommand));

        error = felica_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, FELICA_POLLER_POLLING_FWT);

        if(error != FelicaErrorNone) break;
        if(bit_buffer_get_byte(instance->rx_buffer, 1) != FELICA_POLLER_CMD_POLLING_RESP_CODE) {
            error = FelicaErrorProtocol;
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) <
           sizeof(FelicaIDm) + sizeof(FelicaPMm) + 1) {
            error = FelicaErrorProtocol;
            break;
        }

        bit_buffer_write_bytes_mid(instance->rx_buffer, resp->idm.data, 2, sizeof(FelicaIDm));
        bit_buffer_write_bytes_mid(
            instance->rx_buffer, resp->pmm.data, sizeof(FelicaIDm) + 2, sizeof(FelicaPMm));

    } while(false);

    return error;
}

static void felica_poller_prepare_tx_buffer(
    const FelicaPoller* instance,
    const uint8_t command,
    const uint16_t service_code,
    const uint8_t block_count,
    const uint8_t* const blocks,
    const uint8_t data_block_count,
    const uint8_t* data) {
    FelicaCommandHeader cmd = {
        .code = command,
        .idm = instance->data->idm,
        .service_num = 1,
        .service_code = service_code,
        .block_count = block_count,
    };

    FelicaBlockListElement block_list[4] = {{0}, {0}, {0}, {0}};
    for(uint8_t i = 0; i < block_count; i++) {
        block_list[i].length = 1;
        block_list[i].block_number = blocks[i];
    }

    uint8_t block_list_count = block_count;
    uint8_t block_list_size = block_list_count * sizeof(FelicaBlockListElement);
    uint8_t total_size = sizeof(FelicaCommandHeader) + 1 + block_list_size +
                         data_block_count * FELICA_DATA_BLOCK_SIZE;
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_append_byte(instance->tx_buffer, total_size);
    bit_buffer_append_bytes(instance->tx_buffer, (uint8_t*)&cmd, sizeof(FelicaCommandHeader));
    bit_buffer_append_bytes(instance->tx_buffer, (uint8_t*)&block_list, block_list_size);

    if(data_block_count != 0) {
        bit_buffer_append_bytes(
            instance->tx_buffer, data, data_block_count * FELICA_DATA_BLOCK_SIZE);
    }
}

FelicaError felica_poller_read_blocks(
    FelicaPoller* instance,
    const uint8_t block_count,
    const uint8_t* const block_numbers,
    FelicaPollerReadCommandResponse** const response_ptr) {
    furi_assert(instance);
    furi_assert(block_count <= 4);
    furi_assert(block_numbers);
    furi_assert(response_ptr);

    felica_poller_prepare_tx_buffer(
        instance,
        FELICA_CMD_READ_WITHOUT_ENCRYPTION,
        FELICA_SERVICE_RO_ACCESS,
        block_count,
        block_numbers,
        0,
        NULL);
    bit_buffer_reset(instance->rx_buffer);

    FelicaError error = felica_poller_frame_exchange(
        instance, instance->tx_buffer, instance->rx_buffer, FELICA_POLLER_POLLING_FWT);
    if(error == FelicaErrorNone) {
        *response_ptr = (FelicaPollerReadCommandResponse*)bit_buffer_get_data(instance->rx_buffer);
    }
    return error;
}

FelicaError felica_poller_write_blocks(
    const FelicaPoller* instance,
    const uint8_t block_count,
    const uint8_t* const block_numbers,
    const uint8_t* data,
    FelicaPollerWriteCommandResponse** const response_ptr) {
    furi_assert(instance);
    furi_assert(block_count <= 2);
    furi_assert(block_numbers);
    furi_assert(data);
    furi_assert(response_ptr);

    felica_poller_prepare_tx_buffer(
        instance,
        FELICA_CMD_WRITE_WITHOUT_ENCRYPTION,
        FELICA_SERVICE_RW_ACCESS,
        block_count,
        block_numbers,
        block_count,
        data);
    bit_buffer_reset(instance->rx_buffer);

    FelicaError error = felica_poller_frame_exchange(
        instance, instance->tx_buffer, instance->rx_buffer, FELICA_POLLER_POLLING_FWT);
    if(error == FelicaErrorNone) {
        *response_ptr =
            (FelicaPollerWriteCommandResponse*)bit_buffer_get_data(instance->rx_buffer);
    }
    return error;
}

FelicaError felica_poller_activate(FelicaPoller* instance, FelicaData* data) {
    furi_assert(instance);

    FelicaError ret;

    do {
        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        // Send Polling command
        const FelicaPollerPollingCommand polling_cmd = {
            .system_code = FELICA_SYSTEM_CODE_CODE,
            .request_code = 0,
            .time_slot = FELICA_TIME_SLOT_1,
        };
        FelicaPollerPollingResponse polling_resp = {};

        ret = felica_poller_polling(instance, &polling_cmd, &polling_resp);

        if(ret != FelicaErrorNone) {
            FURI_LOG_T(TAG, "Activation failed error: %d", ret);
            break;
        }

        data->idm = polling_resp.idm;
        data->pmm = polling_resp.pmm;
        instance->state = FelicaPollerStateActivated;
    } while(false);

    return ret;
}
