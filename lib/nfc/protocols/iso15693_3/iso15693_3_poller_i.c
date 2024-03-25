#include "iso15693_3_poller_i.h"

#include <nfc/helpers/iso13239_crc.h>

#define TAG "Iso15693_3Poller"

#define BITS_IN_BYTE (8)

#define ISO15693_3_POLLER_NUM_BLOCKS_PER_QUERY (32U)

static Iso15693_3Error iso15693_3_poller_process_nfc_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return Iso15693_3ErrorNone;
    case NfcErrorTimeout:
        return Iso15693_3ErrorTimeout;
    default:
        return Iso15693_3ErrorNotPresent;
    }
}

static Iso15693_3Error iso15693_3_poller_filter_error(Iso15693_3Error error) {
    switch(error) {
    /* If a particular optional command is not supported, the card might
     * respond with a "Not supported" error or not respond at all.
     * Therefore, treat these errors as non-critical ones. */
    case Iso15693_3ErrorNotSupported:
    case Iso15693_3ErrorTimeout:
        return Iso15693_3ErrorNone;
    default:
        return error;
    }
}

Iso15693_3Error iso15693_3_poller_send_frame(
    Iso15693_3Poller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_assert(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    Iso15693_3Error ret = Iso15693_3ErrorNone;

    do {
        if(bit_buffer_get_size_bytes(tx_buffer) >
           bit_buffer_get_capacity_bytes(instance->tx_buffer) - ISO13239_CRC_SIZE) {
            ret = Iso15693_3ErrorBufferOverflow;
            break;
        }

        bit_buffer_copy(instance->tx_buffer, tx_buffer);
        iso13239_crc_append(Iso13239CrcTypeDefault, instance->tx_buffer);

        NfcError error =
            nfc_poller_trx(instance->nfc, instance->tx_buffer, instance->rx_buffer, fwt);
        if(error != NfcErrorNone) {
            ret = iso15693_3_poller_process_nfc_error(error);
            break;
        }

        if(!iso13239_crc_check(Iso13239CrcTypeDefault, instance->rx_buffer)) {
            ret = Iso15693_3ErrorWrongCrc;
            break;
        }

        iso13239_crc_trim(instance->rx_buffer);
        bit_buffer_copy(rx_buffer, instance->rx_buffer);
    } while(false);

    return ret;
}

Iso15693_3Error iso15693_3_poller_activate(Iso15693_3Poller* instance, Iso15693_3Data* data) {
    furi_assert(instance);
    furi_assert(instance->nfc);

    iso15693_3_reset(data);

    Iso15693_3Error ret;

    do {
        instance->state = Iso15693_3PollerStateColResInProgress;

        // Inventory: Mandatory command
        ret = iso15693_3_poller_inventory(instance, data->uid);
        if(ret != Iso15693_3ErrorNone) {
            instance->state = Iso15693_3PollerStateColResFailed;
            break;
        }

        instance->state = Iso15693_3PollerStateActivated;

        // Get system info: Optional command
        Iso15693_3SystemInfo* system_info = &data->system_info;
        ret = iso15693_3_poller_get_system_info(instance, system_info);
        if(ret != Iso15693_3ErrorNone) {
            ret = iso15693_3_poller_filter_error(ret);
            break;
        }

        if(system_info->block_count > 0) {
            // Read blocks: Optional command
            simple_array_init(
                data->block_data, system_info->block_count * system_info->block_size);
            ret = iso15693_3_poller_read_blocks(
                instance,
                simple_array_get_data(data->block_data),
                system_info->block_count,
                system_info->block_size);
            if(ret != Iso15693_3ErrorNone) {
                ret = iso15693_3_poller_filter_error(ret);
                break;
            }

            // Get block security status: Optional command
            simple_array_init(data->block_security, system_info->block_count);

            ret = iso15693_3_poller_get_blocks_security(
                instance, simple_array_get_data(data->block_security), system_info->block_count);
            if(ret != Iso15693_3ErrorNone) {
                ret = iso15693_3_poller_filter_error(ret);
                break;
            }
        }
    } while(false);

    return ret;
}

Iso15693_3Error iso15693_3_poller_inventory(Iso15693_3Poller* instance, uint8_t* uid) {
    furi_assert(instance);
    furi_assert(instance->nfc);
    furi_assert(uid);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Send INVENTORY
    bit_buffer_append_byte(
        instance->tx_buffer,
        ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI |
            ISO15693_3_REQ_FLAG_INVENTORY_T5 | ISO15693_3_REQ_FLAG_T5_N_SLOTS_1);
    bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_CMD_INVENTORY);
    bit_buffer_append_byte(instance->tx_buffer, 0x00);

    Iso15693_3Error ret;

    do {
        ret = iso15693_3_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC);
        if(ret != Iso15693_3ErrorNone) break;

        ret = iso15693_3_inventory_response_parse(uid, instance->rx_buffer);
    } while(false);

    return ret;
}

Iso15693_3Error
    iso15693_3_poller_get_system_info(Iso15693_3Poller* instance, Iso15693_3SystemInfo* data) {
    furi_assert(instance);
    furi_assert(data);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Send GET SYSTEM INFO
    bit_buffer_append_byte(
        instance->tx_buffer, ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI);

    bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_CMD_GET_SYS_INFO);

    Iso15693_3Error ret;

    do {
        ret = iso15693_3_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC);
        if(ret != Iso15693_3ErrorNone) break;

        ret = iso15693_3_system_info_response_parse(data, instance->rx_buffer);
    } while(false);

    return ret;
}

Iso15693_3Error iso15693_3_poller_read_block(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint8_t block_number,
    uint8_t block_size) {
    furi_assert(instance);
    furi_assert(data);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    bit_buffer_append_byte(
        instance->tx_buffer, ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI);
    bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_CMD_READ_BLOCK);
    bit_buffer_append_byte(instance->tx_buffer, block_number);

    Iso15693_3Error ret;

    do {
        ret = iso15693_3_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC);
        if(ret != Iso15693_3ErrorNone) break;

        ret = iso15693_3_read_block_response_parse(data, block_size, instance->rx_buffer);
    } while(false);

    return ret;
}

Iso15693_3Error iso15693_3_poller_read_blocks(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint16_t block_count,
    uint8_t block_size) {
    furi_assert(instance);
    furi_assert(data);
    furi_assert(block_count);
    furi_assert(block_size);

    Iso15693_3Error ret = Iso15693_3ErrorNone;

    for(uint32_t i = 0; i < block_count; ++i) {
        ret = iso15693_3_poller_read_block(instance, &data[block_size * i], i, block_size);
        if(ret != Iso15693_3ErrorNone) break;
    }

    return ret;
}

Iso15693_3Error iso15693_3_poller_get_blocks_security(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint16_t block_count) {
    furi_assert(instance);
    furi_assert(data);

    // Limit the number of blocks to 32 in a single query
    const uint32_t num_queries = block_count / ISO15693_3_POLLER_NUM_BLOCKS_PER_QUERY +
                                 (block_count % ISO15693_3_POLLER_NUM_BLOCKS_PER_QUERY ? 1 : 0);

    Iso15693_3Error ret = Iso15693_3ErrorNone;

    for(uint32_t i = 0; i < num_queries; ++i) {
        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        bit_buffer_append_byte(
            instance->tx_buffer,
            ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI);

        bit_buffer_append_byte(instance->tx_buffer, ISO15693_3_CMD_GET_BLOCKS_SECURITY);

        const uint8_t start_block_num = i * ISO15693_3_POLLER_NUM_BLOCKS_PER_QUERY;
        bit_buffer_append_byte(instance->tx_buffer, start_block_num);

        const uint8_t block_count_per_query =
            MIN(block_count - start_block_num, (uint16_t)ISO15693_3_POLLER_NUM_BLOCKS_PER_QUERY);
        // Block count byte must be 1 less than the desired count
        bit_buffer_append_byte(instance->tx_buffer, block_count_per_query - 1);

        ret = iso15693_3_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ISO15693_3_FDT_POLL_FC);
        if(ret != Iso15693_3ErrorNone) break;

        ret = iso15693_3_get_block_security_response_parse(
            &data[start_block_num], block_count_per_query, instance->rx_buffer);
        if(ret != Iso15693_3ErrorNone) break;
    }

    return ret;
}
