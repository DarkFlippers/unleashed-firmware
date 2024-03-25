#include "st25tb_poller_i.h"

#include <nfc/helpers/iso14443_crc.h>

#define TAG "ST25TBPoller"

static St25tbError st25tb_poller_process_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return St25tbErrorNone;
    case NfcErrorTimeout:
        return St25tbErrorTimeout;
    default:
        return St25tbErrorNotPresent;
    }
}

St25tbError st25tb_poller_send_frame(
    St25tbPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_check(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    const size_t tx_bytes = bit_buffer_get_size_bytes(tx_buffer);
    furi_assert(
        tx_bytes <= bit_buffer_get_capacity_bytes(instance->tx_buffer) - ISO14443_CRC_SIZE);

    bit_buffer_copy(instance->tx_buffer, tx_buffer);
    iso14443_crc_append(Iso14443CrcTypeB, instance->tx_buffer);

    St25tbError ret = St25tbErrorNone;

    do {
        NfcError error =
            nfc_poller_trx(instance->nfc, instance->tx_buffer, instance->rx_buffer, fwt);
        if(error != NfcErrorNone) {
            FURI_LOG_T(TAG, "error during trx: %d", error);
            ret = st25tb_poller_process_error(error);
            break;
        }

        bit_buffer_copy(rx_buffer, instance->rx_buffer);
        if(!iso14443_crc_check(Iso14443CrcTypeB, instance->rx_buffer)) {
            ret = St25tbErrorWrongCrc;
            break;
        }

        iso14443_crc_trim(rx_buffer);
    } while(false);

    return ret;
}

St25tbError st25tb_poller_initiate(St25tbPoller* instance, uint8_t* chip_id_ptr) {
    // Send Initiate()
    furi_check(instance);
    furi_check(instance->nfc);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);
    bit_buffer_append_byte(instance->tx_buffer, 0x06);
    bit_buffer_append_byte(instance->tx_buffer, 0x00);

    St25tbError ret;
    do {
        ret = st25tb_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != 1) {
            FURI_LOG_E(TAG, "Unexpected Initiate response size");
            ret = St25tbErrorCommunication;
            break;
        }
        uint8_t chip_id = bit_buffer_get_byte(instance->rx_buffer, 0);
        FURI_LOG_D(TAG, "Got chip_id=0x%02X", chip_id);
        if(chip_id_ptr) {
            *chip_id_ptr = bit_buffer_get_byte(instance->rx_buffer, 0);
        }
    } while(false);

    return ret;
}

St25tbError st25tb_poller_select(St25tbPoller* instance, uint8_t* chip_id_ptr) {
    furi_check(instance);
    furi_check(instance->nfc);

    St25tbError ret;

    do {
        uint8_t chip_id;

        if(chip_id_ptr != NULL) {
            chip_id = *chip_id_ptr;
        } else {
            ret = st25tb_poller_initiate(instance, &chip_id);
            if(ret != St25tbErrorNone) {
                break;
            }
        }

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        // Send Select(Chip_ID), let's just assume that collisions won't ever happen :D
        bit_buffer_append_byte(instance->tx_buffer, 0x0E);
        bit_buffer_append_byte(instance->tx_buffer, chip_id);

        ret = st25tb_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != 1) {
            FURI_LOG_E(TAG, "Unexpected Select response size");
            ret = St25tbErrorCommunication;
            break;
        }

        if(bit_buffer_get_byte(instance->rx_buffer, 0) != chip_id) {
            FURI_LOG_E(TAG, "ChipID mismatch");
            ret = St25tbErrorColResFailed;
            break;
        }

        ret = st25tb_poller_get_uid(instance, instance->data->uid);
        if(ret != St25tbErrorNone) {
            break;
        }

        instance->data->type = st25tb_get_type_from_uid(instance->data->uid);
    } while(false);

    return ret;
}

St25tbError st25tb_poller_read(St25tbPoller* instance, St25tbData* data) {
    furi_assert(instance);
    furi_assert(instance->nfc);

    St25tbError ret;

    memcpy(data, instance->data, sizeof(St25tbData));

    do {
        bool read_blocks = true;
        for(uint8_t i = 0; i < st25tb_get_block_count(data->type); i++) {
            ret = st25tb_poller_read_block(instance, &data->blocks[i], i);
            if(ret != St25tbErrorNone) {
                read_blocks = false;
                break;
            }
        }
        if(!read_blocks) {
            break;
        }
        ret = st25tb_poller_read_block(instance, &data->system_otp_block, ST25TB_SYSTEM_OTP_BLOCK);
        if(ret != St25tbErrorNone) {
            break;
        }
    } while(false);

    return ret;
}

St25tbError st25tb_poller_get_uid(St25tbPoller* instance, uint8_t* uid) {
    furi_check(instance);
    furi_check(instance->nfc);
    furi_check(uid);

    St25tbError ret;

    do {
        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        bit_buffer_append_byte(instance->tx_buffer, 0x0B);

        ret = st25tb_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != ST25TB_UID_SIZE) {
            FURI_LOG_E(TAG, "Unexpected Get_UID() response size");
            ret = St25tbErrorCommunication;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, uid, ST25TB_UID_SIZE);
        FURI_SWAP(uid[0], uid[7]);
        FURI_SWAP(uid[1], uid[6]);
        FURI_SWAP(uid[2], uid[5]);
        FURI_SWAP(uid[3], uid[4]);
        FURI_LOG_I(
            TAG,
            "Got tag with uid: %02X %02X %02X %02X %02X %02X %02X %02X",
            uid[0],
            uid[1],
            uid[2],
            uid[3],
            uid[4],
            uid[5],
            uid[6],
            uid[7]);
    } while(false);
    return ret;
}

St25tbError
    st25tb_poller_read_block(St25tbPoller* instance, uint32_t* block, uint8_t block_number) {
    furi_check(instance);
    furi_check(instance->nfc);
    furi_check(block);
    furi_check(
        (block_number <= st25tb_get_block_count(instance->data->type)) ||
        block_number == ST25TB_SYSTEM_OTP_BLOCK);
    FURI_LOG_T(TAG, "reading block %d", block_number);
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Send Read_block(Addr)
    bit_buffer_append_byte(instance->tx_buffer, 0x08);
    bit_buffer_append_byte(instance->tx_buffer, block_number);
    St25tbError ret;
    do {
        ret = st25tb_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != ST25TB_BLOCK_SIZE) {
            FURI_LOG_E(TAG, "Unexpected Read_block(Addr) response size");
            ret = St25tbErrorCommunication;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, block, ST25TB_BLOCK_SIZE);
        FURI_LOG_D(TAG, "Read_block(%d) result: %08lX", block_number, *block);
    } while(false);

    return ret;
}

St25tbError
    st25tb_poller_write_block(St25tbPoller* instance, uint32_t block, uint8_t block_number) {
    furi_check(instance);
    furi_check(instance->nfc);
    furi_check(
        (block_number <= st25tb_get_block_count(instance->data->type)) ||
        block_number == ST25TB_SYSTEM_OTP_BLOCK);
    FURI_LOG_T(TAG, "writing block %d", block_number);
    bit_buffer_reset(instance->tx_buffer);

    // Send Write_block(Addr, Data)
    bit_buffer_append_byte(instance->tx_buffer, 0x09);
    bit_buffer_append_byte(instance->tx_buffer, block_number);
    bit_buffer_append_bytes(instance->tx_buffer, (uint8_t*)&block, ST25TB_BLOCK_SIZE);
    St25tbError ret;
    do {
        ret = st25tb_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorTimeout) { // tag doesn't ack writes so timeout are expected.
            break;
        }

        furi_delay_ms(7); // 7ms is the max programming time as per datasheet

        uint32_t block_check;
        ret = st25tb_poller_read_block(instance, &block_check, block_number);
        if(ret != St25tbErrorNone) {
            FURI_LOG_E(TAG, "write verification failed: read error");
            break;
        }
        if(block_check != block) {
            FURI_LOG_E(
                TAG,
                "write verification failed: wrote %08lX but read back %08lX",
                block,
                block_check);
            ret = St25tbErrorWriteFailed;
            break;
        }
        FURI_LOG_D(TAG, "wrote %08lX to block %d", block, block_number);
    } while(false);

    return ret;
}

St25tbError st25tb_poller_halt(St25tbPoller* instance) {
    furi_check(instance);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Send Completion()
    bit_buffer_append_byte(instance->tx_buffer, 0x0F);

    St25tbError ret;

    do {
        ret = st25tb_poller_send_frame(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorTimeout) {
            break;
        }

        instance->state = St25tbPollerStateSelect;
    } while(false);

    return ret;
}
