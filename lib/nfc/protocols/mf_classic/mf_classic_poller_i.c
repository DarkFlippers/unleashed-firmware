#include "mf_classic_poller_i.h"

#include <furi.h>
#include <furi_hal_random.h>

#include <nfc/helpers/iso14443_crc.h>

#define TAG "MfCLassicPoller"

MfClassicError mf_classic_process_error(Iso14443_3aError error) {
    MfClassicError ret = MfClassicErrorNone;

    switch(error) {
    case Iso14443_3aErrorNone:
        ret = MfClassicErrorNone;
        break;
    case Iso14443_3aErrorNotPresent:
        ret = MfClassicErrorNotPresent;
        break;
    case Iso14443_3aErrorColResFailed:
    case Iso14443_3aErrorCommunication:
    case Iso14443_3aErrorWrongCrc:
        ret = MfClassicErrorProtocol;
        break;
    case Iso14443_3aErrorTimeout:
        ret = MfClassicErrorTimeout;
        break;
    default:
        ret = MfClassicErrorProtocol;
        break;
    }

    return ret;
}

static MfClassicError mf_classic_poller_get_nt_common(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicNt* nt,
    bool is_nested) {
    MfClassicError ret = MfClassicErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t auth_type = (key_type == MfClassicKeyTypeB) ? MF_CLASSIC_CMD_AUTH_KEY_B :
                                                              MF_CLASSIC_CMD_AUTH_KEY_A;
        uint8_t auth_cmd[2] = {auth_type, block_num};
        bit_buffer_copy_bytes(instance->tx_plain_buffer, auth_cmd, sizeof(auth_cmd));

        if(is_nested) {
            iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);
            crypto1_encrypt(
                instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);
            error = iso14443_3a_poller_txrx_custom_parity(
                instance->iso14443_3a_poller,
                instance->tx_encrypted_buffer,
                instance->rx_plain_buffer, // NT gets decrypted by mf_classic_async_auth
                MF_CLASSIC_FWT_FC);
            if(error != Iso14443_3aErrorNone) {
                ret = mf_classic_process_error(error);
                break;
            }
        } else {
            error = iso14443_3a_poller_send_standard_frame(
                instance->iso14443_3a_poller,
                instance->tx_plain_buffer,
                instance->rx_plain_buffer,
                MF_CLASSIC_FWT_FC);
            if(error != Iso14443_3aErrorWrongCrc) {
                ret = mf_classic_process_error(error);
                break;
            }
        }
        if(bit_buffer_get_size_bytes(instance->rx_plain_buffer) != sizeof(MfClassicNt)) {
            ret = MfClassicErrorProtocol;
            break;
        }

        if(nt) {
            bit_buffer_write_bytes(instance->rx_plain_buffer, nt->data, sizeof(MfClassicNt));
        }
    } while(false);

    return ret;
}

MfClassicError mf_classic_poller_get_nt(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicNt* nt) {
    return mf_classic_poller_get_nt_common(instance, block_num, key_type, nt, false);
}

MfClassicError mf_classic_poller_get_nt_nested(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicNt* nt) {
    return mf_classic_poller_get_nt_common(instance, block_num, key_type, nt, true);
}

static MfClassicError mf_classic_poller_auth_common(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicAuthContext* data,
    bool is_nested) {
    MfClassicError ret = MfClassicErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        iso14443_3a_copy(
            instance->data->iso14443_3a_data,
            iso14443_3a_poller_get_data(instance->iso14443_3a_poller));

        MfClassicNt nt = {};
        if(is_nested) {
            ret = mf_classic_poller_get_nt_nested(instance, block_num, key_type, &nt);
        } else {
            ret = mf_classic_poller_get_nt(instance, block_num, key_type, &nt);
        }
        if(ret != MfClassicErrorNone) break;
        if(data) {
            data->nt = nt;
        }

        uint32_t cuid = iso14443_3a_get_cuid(instance->data->iso14443_3a_data);
        uint64_t key_num = nfc_util_bytes2num(key->data, sizeof(MfClassicKey));
        MfClassicNr nr = {};
        furi_hal_random_fill_buf(nr.data, sizeof(MfClassicNr));

        crypto1_encrypt_reader_nonce(
            instance->crypto,
            key_num,
            cuid,
            nt.data,
            nr.data,
            instance->tx_encrypted_buffer,
            is_nested);
        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);

        if(error != Iso14443_3aErrorNone) {
            ret = mf_classic_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_encrypted_buffer) != 4) {
            ret = MfClassicErrorAuth;
        }

        crypto1_word(instance->crypto, 0, 0);
        instance->auth_state = MfClassicAuthStatePassed;

        if(data) {
            data->nr = nr;
            const uint8_t* nr_ar = bit_buffer_get_data(instance->tx_encrypted_buffer);
            memcpy(data->ar.data, &nr_ar[4], sizeof(MfClassicAr));
            bit_buffer_write_bytes(
                instance->rx_encrypted_buffer, data->at.data, sizeof(MfClassicAt));
        }
    } while(false);

    if(ret != MfClassicErrorNone) {
        iso14443_3a_poller_halt(instance->iso14443_3a_poller);
    }

    return ret;
}

MfClassicError mf_classic_poller_auth(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicAuthContext* data) {
    return mf_classic_poller_auth_common(instance, block_num, key, key_type, data, false);
}

MfClassicError mf_classic_poller_auth_nested(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicAuthContext* data) {
    return mf_classic_poller_auth_common(instance, block_num, key, key_type, data, true);
}

MfClassicError mf_classic_poller_halt(MfClassicPoller* instance) {
    MfClassicError ret = MfClassicErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t halt_cmd[2] = {MF_CLASSIC_CMD_HALT_MSB, MF_CLASSIC_CMD_HALT_LSB};
        bit_buffer_copy_bytes(instance->tx_plain_buffer, halt_cmd, sizeof(halt_cmd));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);

        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);

        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);
        if(error != Iso14443_3aErrorTimeout) {
            ret = mf_classic_process_error(error);
            break;
        }
        instance->auth_state = MfClassicAuthStateIdle;
        instance->iso14443_3a_poller->state = Iso14443_3aPollerStateIdle;
    } while(false);

    return ret;
}

MfClassicError mf_classic_poller_read_block(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicBlock* data) {
    MfClassicError ret = MfClassicErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t read_block_cmd[2] = {MF_CLASSIC_CMD_READ_BLOCK, block_num};
        bit_buffer_copy_bytes(instance->tx_plain_buffer, read_block_cmd, sizeof(read_block_cmd));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);

        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);

        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_classic_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_encrypted_buffer) !=
           (sizeof(MfClassicBlock) + 2)) {
            ret = MfClassicErrorProtocol;
            break;
        }

        crypto1_decrypt(
            instance->crypto, instance->rx_encrypted_buffer, instance->rx_plain_buffer);

        if(!iso14443_crc_check(Iso14443CrcTypeA, instance->rx_plain_buffer)) {
            FURI_LOG_D(TAG, "CRC error");
            ret = MfClassicErrorProtocol;
            break;
        }

        iso14443_crc_trim(instance->rx_plain_buffer);
        bit_buffer_write_bytes(instance->rx_plain_buffer, data->data, sizeof(MfClassicBlock));
    } while(false);

    return ret;
}

MfClassicError mf_classic_poller_write_block(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicBlock* data) {
    MfClassicError ret = MfClassicErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t write_block_cmd[2] = {MF_CLASSIC_CMD_WRITE_BLOCK, block_num};
        bit_buffer_copy_bytes(instance->tx_plain_buffer, write_block_cmd, sizeof(write_block_cmd));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);

        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);

        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_classic_process_error(error);
            break;
        }
        if(bit_buffer_get_size(instance->rx_encrypted_buffer) != 4) {
            ret = MfClassicErrorProtocol;
            break;
        }

        crypto1_decrypt(
            instance->crypto, instance->rx_encrypted_buffer, instance->rx_plain_buffer);

        if(bit_buffer_get_byte(instance->rx_plain_buffer, 0) != MF_CLASSIC_CMD_ACK) {
            FURI_LOG_D(TAG, "Not ACK received");
            ret = MfClassicErrorProtocol;
            break;
        }

        bit_buffer_copy_bytes(instance->tx_plain_buffer, data->data, sizeof(MfClassicBlock));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);

        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);

        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_classic_process_error(error);
            break;
        }
        if(bit_buffer_get_size(instance->rx_encrypted_buffer) != 4) {
            ret = MfClassicErrorProtocol;
            break;
        }

        crypto1_decrypt(
            instance->crypto, instance->rx_encrypted_buffer, instance->rx_plain_buffer);

        if(bit_buffer_get_byte(instance->rx_plain_buffer, 0) != MF_CLASSIC_CMD_ACK) {
            FURI_LOG_D(TAG, "Not ACK received");
            ret = MfClassicErrorProtocol;
            break;
        }
    } while(false);

    return ret;
}

MfClassicError mf_classic_poller_value_cmd(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicValueCommand cmd,
    int32_t data) {
    MfClassicError ret = MfClassicErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t cmd_value = 0;
        if(cmd == MfClassicValueCommandDecrement) {
            cmd_value = MF_CLASSIC_CMD_VALUE_DEC;
        } else if(cmd == MfClassicValueCommandIncrement) {
            cmd_value = MF_CLASSIC_CMD_VALUE_INC;
        } else {
            cmd_value = MF_CLASSIC_CMD_VALUE_RESTORE;
        }
        uint8_t value_cmd[2] = {cmd_value, block_num};
        bit_buffer_copy_bytes(instance->tx_plain_buffer, value_cmd, sizeof(value_cmd));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);

        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);

        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_classic_process_error(error);
            break;
        }
        if(bit_buffer_get_size(instance->rx_encrypted_buffer) != 4) {
            ret = MfClassicErrorProtocol;
            break;
        }

        crypto1_decrypt(
            instance->crypto, instance->rx_encrypted_buffer, instance->rx_plain_buffer);

        if(bit_buffer_get_byte(instance->rx_plain_buffer, 0) != MF_CLASSIC_CMD_ACK) {
            FURI_LOG_D(TAG, "Not ACK received");
            ret = MfClassicErrorProtocol;
            break;
        }

        bit_buffer_copy_bytes(instance->tx_plain_buffer, (uint8_t*)&data, sizeof(data));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);

        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);

        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);

        // Command processed if tag doesn't respond
        if(error != Iso14443_3aErrorTimeout) {
            ret = MfClassicErrorProtocol;
            break;
        }
        ret = MfClassicErrorNone;
    } while(false);

    return ret;
}

MfClassicError mf_classic_poller_value_transfer(MfClassicPoller* instance, uint8_t block_num) {
    MfClassicError ret = MfClassicErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t transfer_cmd[2] = {MF_CLASSIC_CMD_VALUE_TRANSFER, block_num};
        bit_buffer_copy_bytes(instance->tx_plain_buffer, transfer_cmd, sizeof(transfer_cmd));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);

        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);

        error = iso14443_3a_poller_txrx_custom_parity(
            instance->iso14443_3a_poller,
            instance->tx_encrypted_buffer,
            instance->rx_encrypted_buffer,
            MF_CLASSIC_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_classic_process_error(error);
            break;
        }
        if(bit_buffer_get_size(instance->rx_encrypted_buffer) != 4) {
            ret = MfClassicErrorProtocol;
            break;
        }

        crypto1_decrypt(
            instance->crypto, instance->rx_encrypted_buffer, instance->rx_plain_buffer);

        if(bit_buffer_get_byte(instance->rx_plain_buffer, 0) != MF_CLASSIC_CMD_ACK) {
            FURI_LOG_D(TAG, "Not ACK received");
            ret = MfClassicErrorProtocol;
            break;
        }

    } while(false);

    return ret;
}
