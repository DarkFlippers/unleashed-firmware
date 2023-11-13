#include "st25tb_poller_i.h"

#include "bit_buffer.h"
#include "core/core_defines.h"
#include "protocols/st25tb/st25tb.h"
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

static St25tbError st25tb_poller_prepare_trx(St25tbPoller* instance) {
    furi_assert(instance);

    if(instance->state == St25tbPollerStateIdle) {
        return st25tb_poller_async_activate(instance, NULL);
    }

    return St25tbErrorNone;
}

static St25tbError st25tb_poller_frame_exchange(
    St25tbPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_assert(instance);

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
            FURI_LOG_D(TAG, "error during trx: %d", error);
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

St25tbType st25tb_get_type_from_uid(const uint8_t uid[ST25TB_UID_SIZE]) {
    switch(uid[2] >> 2) {
    case 0x0:
    case 0x3:
        return St25tbTypeX4k;
    case 0x4:
        return St25tbTypeX512;
    case 0x6:
        return St25tbType512Ac;
    case 0x7:
        return St25tbType04k;
    case 0xc:
        return St25tbType512At;
    case 0xf:
        return St25tbType02k;
    default:
        furi_crash("unsupported st25tb type");
    }
}

St25tbError st25tb_poller_async_initiate(St25tbPoller* instance, uint8_t* chip_id) {
    // Send Initiate()
    furi_assert(instance);
    furi_assert(instance->nfc);

    instance->state = St25tbPollerStateInitiateInProgress;
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);
    bit_buffer_append_byte(instance->tx_buffer, 0x06);
    bit_buffer_append_byte(instance->tx_buffer, 0x00);

    St25tbError ret;
    do {
        ret = st25tb_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != 1) {
            FURI_LOG_D(TAG, "Unexpected Initiate response size");
            ret = St25tbErrorCommunication;
            break;
        }
        if(chip_id) {
            *chip_id = bit_buffer_get_byte(instance->rx_buffer, 0);
        }
    } while(false);

    return ret;
}

St25tbError st25tb_poller_async_activate(St25tbPoller* instance, St25tbData* data) {
    furi_assert(instance);
    furi_assert(instance->nfc);

    st25tb_reset(data);

    St25tbError ret;

    do {
        ret = st25tb_poller_async_initiate(instance, &data->chip_id);
        if(ret != St25tbErrorNone) {
            break;
        }

        instance->state = St25tbPollerStateActivationInProgress;

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        // Send Select(Chip_ID), let's just assume that collisions won't ever happen :D
        bit_buffer_append_byte(instance->tx_buffer, 0x0E);
        bit_buffer_append_byte(instance->tx_buffer, data->chip_id);

        ret = st25tb_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            instance->state = St25tbPollerStateActivationFailed;
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != 1) {
            FURI_LOG_D(TAG, "Unexpected Select response size");
            instance->state = St25tbPollerStateActivationFailed;
            ret = St25tbErrorCommunication;
            break;
        }

        if(bit_buffer_get_byte(instance->rx_buffer, 0) != data->chip_id) {
            FURI_LOG_D(TAG, "ChipID mismatch");
            instance->state = St25tbPollerStateActivationFailed;
            ret = St25tbErrorColResFailed;
            break;
        }
        instance->state = St25tbPollerStateActivated;

        ret = st25tb_poller_async_get_uid(instance, data->uid);
        if(ret != St25tbErrorNone) {
            instance->state = St25tbPollerStateActivationFailed;
            break;
        }
        data->type = st25tb_get_type_from_uid(data->uid);

        bool read_blocks = true;
        for(uint8_t i = 0; i < st25tb_get_block_count(data->type); i++) {
            ret = st25tb_poller_async_read_block(instance, &data->blocks[i], i);
            if(ret != St25tbErrorNone) {
                read_blocks = false;
                break;
            }
        }
        if(!read_blocks) {
            break;
        }
        ret = st25tb_poller_async_read_block(
            instance, &data->system_otp_block, ST25TB_SYSTEM_OTP_BLOCK);
    } while(false);

    return ret;
}

St25tbError st25tb_poller_async_get_uid(St25tbPoller* instance, uint8_t uid[ST25TB_UID_SIZE]) {
    furi_assert(instance);
    furi_assert(instance->nfc);

    St25tbError ret;

    do {
        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        bit_buffer_append_byte(instance->tx_buffer, 0x0B);

        ret = st25tb_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != ST25TB_UID_SIZE) {
            FURI_LOG_D(TAG, "Unexpected Get_UID() response size");
            instance->state = St25tbPollerStateActivationFailed;
            ret = St25tbErrorCommunication;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, uid, ST25TB_UID_SIZE);
        FURI_SWAP(uid[0], uid[7]);
        FURI_SWAP(uid[1], uid[6]);
        FURI_SWAP(uid[2], uid[5]);
        FURI_SWAP(uid[3], uid[4]);
    } while(false);
    return ret;
}

St25tbError
    st25tb_poller_async_read_block(St25tbPoller* instance, uint32_t* block, uint8_t block_number) {
    furi_assert(instance);
    furi_assert(instance->nfc);
    furi_assert(block);
    furi_assert(
        (block_number <= st25tb_get_block_count(instance->data->type)) ||
        block_number == ST25TB_SYSTEM_OTP_BLOCK);
    FURI_LOG_D(TAG, "reading block %d", block_number);
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Send Read_block(Addr)
    bit_buffer_append_byte(instance->tx_buffer, 0x08);
    bit_buffer_append_byte(instance->tx_buffer, block_number);
    St25tbError ret;
    do {
        ret = st25tb_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorNone) {
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) != ST25TB_BLOCK_SIZE) {
            FURI_LOG_D(TAG, "Unexpected Read_block(Addr) response size");
            ret = St25tbErrorCommunication;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, block, ST25TB_BLOCK_SIZE);
        FURI_LOG_D(TAG, "read result: %08lX", *block);
    } while(false);

    return ret;
}

St25tbError st25tb_poller_halt(St25tbPoller* instance) {
    furi_assert(instance);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Send Completion()
    bit_buffer_append_byte(instance->tx_buffer, 0x0F);

    St25tbError ret;

    do {
        ret = st25tb_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, ST25TB_FDT_FC);
        if(ret != St25tbErrorTimeout) {
            break;
        }

        instance->state = St25tbPollerStateIdle;
    } while(false);

    return ret;
}

St25tbError st25tb_poller_send_frame(
    St25tbPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    St25tbError ret;

    do {
        ret = st25tb_poller_prepare_trx(instance);
        if(ret != St25tbErrorNone) break;

        ret = st25tb_poller_frame_exchange(instance, tx_buffer, rx_buffer, fwt);
    } while(false);

    return ret;
}
