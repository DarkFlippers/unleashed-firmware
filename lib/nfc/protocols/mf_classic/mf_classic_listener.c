#include "mf_classic_listener_i.h"

#include <nfc/protocols/nfc_listener_base.h>

#include <nfc/helpers/iso14443_crc.h>
#include <bit_lib/bit_lib.h>

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "MfClassicListener"

#define MF_CLASSIC_MAX_BUFF_SIZE (64)

typedef MfClassicListenerCommand (
    *MfClassicListenerCommandHandler)(MfClassicListener* instance, BitBuffer* buf);

typedef struct {
    uint8_t cmd_start_byte;
    size_t cmd_len_bits;
    size_t command_num;
    MfClassicListenerCommandHandler* handler;
} MfClassicListenerCmd;

static void mf_classic_listener_prepare_emulation(MfClassicListener* instance) {
    instance->total_block_num = mf_classic_get_total_block_num(instance->data->type);
}

static void mf_classic_listener_reset_state(MfClassicListener* instance) {
    crypto1_reset(instance->crypto);
    memset(&instance->auth_context, 0, sizeof(MfClassicAuthContext));
    instance->comm_state = MfClassicListenerCommStatePlain;
    instance->state = MfClassicListenerStateIdle;
    instance->cmd_in_progress = false;
    instance->current_cmd_handler_idx = 0;
    instance->write_block = 0;
    instance->transfer_value = 0;
    instance->transfer_valid = false;
    instance->value_cmd = MfClassicValueCommandInvalid;
}

static MfClassicListenerCommand
    mf_classic_listener_halt_handler(MfClassicListener* instance, BitBuffer* buff) {
    UNUSED(instance);

    MfClassicListenerCommand command = MfClassicListenerCommandNack;

    if(bit_buffer_get_byte(buff, 1) == MF_CLASSIC_CMD_HALT_LSB) {
        command = MfClassicListenerCommandSleep;
    }

    return command;
}

static MfClassicListenerCommand mf_classic_listener_auth_first_part_handler(
    MfClassicListener* instance,
    MfClassicKeyType key_type,
    uint8_t block_num) {
    MfClassicListenerCommand command = MfClassicListenerCommandNack;

    do {
        instance->state = MfClassicListenerStateIdle;

        if(block_num >= instance->total_block_num) break;

        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);

        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(instance->data, sector_num);
        MfClassicKey* key = (key_type == MfClassicKeyTypeA) ? &sec_tr->key_a : &sec_tr->key_b;
        uint64_t key_num = bit_lib_bytes_to_num_be(key->data, sizeof(MfClassicKey));
        uint32_t cuid = iso14443_3a_get_cuid(instance->data->iso14443_3a_data);

        instance->auth_context.key_type = key_type;
        instance->auth_context.block_num = block_num;

        furi_hal_random_fill_buf(instance->auth_context.nt.data, sizeof(MfClassicNt));
        uint32_t nt_num =
            bit_lib_bytes_to_num_be(instance->auth_context.nt.data, sizeof(MfClassicNt));

        crypto1_init(instance->crypto, key_num);
        if(instance->comm_state == MfClassicListenerCommStatePlain) {
            crypto1_word(instance->crypto, nt_num ^ cuid, 0);
            bit_buffer_copy_bytes(
                instance->tx_encrypted_buffer,
                instance->auth_context.nt.data,
                sizeof(MfClassicNt));
            iso14443_3a_listener_tx(instance->iso14443_3a_listener, instance->tx_encrypted_buffer);
            command = MfClassicListenerCommandProcessed;
        } else {
            uint8_t key_stream[4] = {};
            bit_lib_num_to_bytes_be(nt_num ^ cuid, sizeof(uint32_t), key_stream);
            bit_buffer_copy_bytes(
                instance->tx_plain_buffer, instance->auth_context.nt.data, sizeof(MfClassicNt));
            crypto1_encrypt(
                instance->crypto,
                key_stream,
                instance->tx_plain_buffer,
                instance->tx_encrypted_buffer);

            iso14443_3a_listener_tx_with_custom_parity(
                instance->iso14443_3a_listener, instance->tx_encrypted_buffer);

            command = MfClassicListenerCommandProcessed;
        }

        instance->cmd_in_progress = true;
        instance->current_cmd_handler_idx++;
    } while(false);

    return command;
}

static MfClassicListenerCommand
    mf_classic_listener_auth_key_a_handler(MfClassicListener* instance, BitBuffer* buff) {
    MfClassicListenerCommand command = mf_classic_listener_auth_first_part_handler(
        instance, MfClassicKeyTypeA, bit_buffer_get_byte(buff, 1));

    return command;
}

static MfClassicListenerCommand
    mf_classic_listener_auth_key_b_handler(MfClassicListener* instance, BitBuffer* buff) {
    MfClassicListenerCommand command = mf_classic_listener_auth_first_part_handler(
        instance, MfClassicKeyTypeB, bit_buffer_get_byte(buff, 1));

    return command;
}

static MfClassicListenerCommand
    mf_classic_listener_auth_second_part_handler(MfClassicListener* instance, BitBuffer* buff) {
    MfClassicListenerCommand command = MfClassicListenerCommandSilent;

    do {
        instance->cmd_in_progress = false;

        if(bit_buffer_get_size_bytes(buff) != (sizeof(MfClassicNr) + sizeof(MfClassicAr))) {
            command = MfClassicListenerCommandSleep;
            break;
        }
        bit_buffer_write_bytes_mid(buff, instance->auth_context.nr.data, 0, sizeof(MfClassicNr));
        bit_buffer_write_bytes_mid(
            buff, instance->auth_context.ar.data, sizeof(MfClassicNr), sizeof(MfClassicAr));

        if(instance->callback) {
            instance->mfc_event.type = MfClassicListenerEventTypeAuthContextPartCollected,
            instance->mfc_event_data.auth_context = instance->auth_context;
            instance->callback(instance->generic_event, instance->context);
        }

        uint32_t nr_num =
            bit_lib_bytes_to_num_be(instance->auth_context.nr.data, sizeof(MfClassicNr));
        uint32_t ar_num =
            bit_lib_bytes_to_num_be(instance->auth_context.ar.data, sizeof(MfClassicAr));

        crypto1_word(instance->crypto, nr_num, 1);
        uint32_t nt_num =
            bit_lib_bytes_to_num_be(instance->auth_context.nt.data, sizeof(MfClassicNt));
        uint32_t secret_poller = ar_num ^ crypto1_word(instance->crypto, 0, 0);
        if(secret_poller != crypto1_prng_successor(nt_num, 64)) {
            FURI_LOG_T(
                TAG,
                "Wrong reader key: %08lX != %08lX",
                secret_poller,
                crypto1_prng_successor(nt_num, 64));
            command = MfClassicListenerCommandSleep;
            break;
        }

        uint32_t at_num = crypto1_prng_successor(nt_num, 96);
        bit_lib_num_to_bytes_be(at_num, sizeof(uint32_t), instance->auth_context.at.data);
        bit_buffer_copy_bytes(
            instance->tx_plain_buffer, instance->auth_context.at.data, sizeof(MfClassicAr));
        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);
        iso14443_3a_listener_tx_with_custom_parity(
            instance->iso14443_3a_listener, instance->tx_encrypted_buffer);

        instance->state = MfClassicListenerStateAuthComplete;
        instance->comm_state = MfClassicListenerCommStateEncrypted;
        command = MfClassicListenerCommandProcessed;

        if(instance->callback) {
            instance->mfc_event.type = MfClassicListenerEventTypeAuthContextFullCollected,
            instance->mfc_event_data.auth_context = instance->auth_context;
            instance->callback(instance->generic_event, instance->context);
        }
    } while(false);

    return command;
}

static MfClassicListenerCommand
    mf_classic_listener_read_block_handler(MfClassicListener* instance, BitBuffer* buff) {
    MfClassicListenerCommand command = MfClassicListenerCommandNack;
    MfClassicAuthContext* auth_ctx = &instance->auth_context;

    do {
        if(instance->state != MfClassicListenerStateAuthComplete) break;

        uint8_t block_num = bit_buffer_get_byte(buff, 1);
        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
        uint8_t auth_sector_num = mf_classic_get_sector_by_block(auth_ctx->block_num);
        if(sector_num != auth_sector_num) break;

        MfClassicBlock access_block = instance->data->block[block_num];

        if(mf_classic_is_sector_trailer(block_num)) {
            MfClassicSectorTrailer* access_sec_tr = (MfClassicSectorTrailer*)&access_block;
            if(!mf_classic_is_allowed_access(
                   instance->data, block_num, auth_ctx->key_type, MfClassicActionKeyARead)) {
                memset(access_sec_tr->key_a.data, 0, sizeof(MfClassicKey));
            }
            if(!mf_classic_is_allowed_access(
                   instance->data, block_num, auth_ctx->key_type, MfClassicActionKeyBRead)) {
                memset(access_sec_tr->key_b.data, 0, sizeof(MfClassicKey));
            }
            if(!mf_classic_is_allowed_access(
                   instance->data, block_num, auth_ctx->key_type, MfClassicActionACRead)) {
                memset(access_sec_tr->access_bits.data, 0, sizeof(MfClassicAccessBits));
            }
        } else if(!mf_classic_is_allowed_access(
                      instance->data, block_num, auth_ctx->key_type, MfClassicActionDataRead)) {
            break;
        }

        bit_buffer_copy_bytes(
            instance->tx_plain_buffer, access_block.data, sizeof(MfClassicBlock));
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_plain_buffer);
        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);
        iso14443_3a_listener_tx_with_custom_parity(
            instance->iso14443_3a_listener, instance->tx_encrypted_buffer);
        command = MfClassicListenerCommandProcessed;
    } while(false);

    return command;
}

static MfClassicListenerCommand mf_classic_listener_write_block_first_part_handler(
    MfClassicListener* instance,
    BitBuffer* buff) {
    MfClassicListenerCommand command = MfClassicListenerCommandNack;
    MfClassicAuthContext* auth_ctx = &instance->auth_context;

    do {
        if(instance->state != MfClassicListenerStateAuthComplete) break;

        uint8_t block_num = bit_buffer_get_byte(buff, 1);
        if(block_num >= instance->total_block_num) break;
        if(block_num == 0) break;

        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
        uint8_t auth_sector_num = mf_classic_get_sector_by_block(auth_ctx->block_num);
        if(sector_num != auth_sector_num) break;

        instance->write_block = block_num;
        instance->cmd_in_progress = true;
        instance->current_cmd_handler_idx++;
        command = MfClassicListenerCommandAck;
    } while(false);

    return command;
}

static MfClassicListenerCommand mf_classic_listener_write_block_second_part_handler(
    MfClassicListener* instance,
    BitBuffer* buff) {
    MfClassicListenerCommand command = MfClassicListenerCommandNack;
    MfClassicAuthContext* auth_ctx = &instance->auth_context;

    do {
        instance->cmd_in_progress = false;

        size_t buff_size = bit_buffer_get_size_bytes(buff);
        if(buff_size != sizeof(MfClassicBlock)) break;

        uint8_t block_num = instance->write_block;
        MfClassicKeyType key_type = auth_ctx->key_type;
        MfClassicBlock block = instance->data->block[block_num];

        if(mf_classic_is_sector_trailer(block_num)) {
            MfClassicSectorTrailer* sec_tr = (MfClassicSectorTrailer*)&block;

            // Check if any writing is allowed
            if(!mf_classic_is_allowed_access(
                   instance->data, block_num, key_type, MfClassicActionKeyAWrite) &&
               !mf_classic_is_allowed_access(
                   instance->data, block_num, key_type, MfClassicActionKeyBWrite) &&
               !mf_classic_is_allowed_access(
                   instance->data, block_num, key_type, MfClassicActionACWrite)) {
                break;
            }

            if(mf_classic_is_allowed_access(
                   instance->data, block_num, key_type, MfClassicActionKeyAWrite)) {
                bit_buffer_write_bytes_mid(buff, sec_tr->key_a.data, 0, sizeof(MfClassicKey));
            }
            if(mf_classic_is_allowed_access(
                   instance->data, block_num, key_type, MfClassicActionKeyBWrite)) {
                bit_buffer_write_bytes_mid(buff, sec_tr->key_b.data, 10, sizeof(MfClassicKey));
            }
            if(mf_classic_is_allowed_access(
                   instance->data, block_num, key_type, MfClassicActionACWrite)) {
                bit_buffer_write_bytes_mid(
                    buff, sec_tr->access_bits.data, 6, sizeof(MfClassicAccessBits));
            }
        } else {
            if(mf_classic_is_allowed_access(
                   instance->data, block_num, key_type, MfClassicActionDataWrite)) {
                bit_buffer_write_bytes_mid(buff, block.data, 0, sizeof(MfClassicBlock));
            } else {
                break;
            }
        }

        instance->data->block[block_num] = block;
        command = MfClassicListenerCommandAck;
    } while(false);

    return command;
}

static MfClassicListenerCommand
    mf_classic_listener_value_cmd_handler(MfClassicListener* instance, BitBuffer* buff) {
    MfClassicListenerCommand command = MfClassicListenerCommandNack;
    MfClassicAuthContext* auth_ctx = &instance->auth_context;

    do {
        if(instance->state != MfClassicListenerStateAuthComplete) break;

        uint8_t block_num = bit_buffer_get_byte(buff, 1);
        if(block_num >= instance->total_block_num) break;

        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
        uint8_t auth_sector_num = mf_classic_get_sector_by_block(auth_ctx->block_num);
        if(sector_num != auth_sector_num) break;

        uint8_t cmd = bit_buffer_get_byte(buff, 0);
        MfClassicAction action = MfClassicActionDataDec;
        if(cmd == MF_CLASSIC_CMD_VALUE_DEC) {
            instance->value_cmd = MfClassicValueCommandDecrement;
        } else if(cmd == MF_CLASSIC_CMD_VALUE_INC) {
            instance->value_cmd = MfClassicValueCommandIncrement;
            action = MfClassicActionDataInc;
        } else if(cmd == MF_CLASSIC_CMD_VALUE_RESTORE) {
            instance->value_cmd = MfClassicValueCommandRestore;
        } else {
            break;
        }

        if(!mf_classic_is_allowed_access(instance->data, block_num, auth_ctx->key_type, action)) {
            break;
        }

        if(!mf_classic_block_to_value(
               &instance->data->block[block_num], &instance->transfer_value, NULL)) {
            break;
        }

        instance->transfer_valid = true;
        instance->cmd_in_progress = true;
        instance->current_cmd_handler_idx++;
        command = MfClassicListenerCommandAck;
    } while(false);

    return command;
}

static MfClassicListenerCommand
    mf_classic_listener_value_dec_handler(MfClassicListener* instance, BitBuffer* buff) {
    return mf_classic_listener_value_cmd_handler(instance, buff);
}

static MfClassicListenerCommand
    mf_classic_listener_value_inc_handler(MfClassicListener* instance, BitBuffer* buff) {
    return mf_classic_listener_value_cmd_handler(instance, buff);
}

static MfClassicListenerCommand
    mf_classic_listener_value_restore_handler(MfClassicListener* instance, BitBuffer* buff) {
    return mf_classic_listener_value_cmd_handler(instance, buff);
}

static MfClassicListenerCommand
    mf_classic_listener_value_data_receive_handler(MfClassicListener* instance, BitBuffer* buff) {
    MfClassicListenerCommand command = MfClassicListenerCommandNack;

    do {
        if(bit_buffer_get_size_bytes(buff) != 4) break;

        int32_t data;
        bit_buffer_write_bytes_mid(buff, &data, 0, sizeof(data));

        if(data < 0) {
            data = -data;
        }

        if(instance->value_cmd == MfClassicValueCommandDecrement) {
            data = -data;
        } else if(instance->value_cmd == MfClassicValueCommandRestore) {
            data = 0;
        }

        instance->transfer_value += data;
        instance->transfer_valid = true;

        instance->cmd_in_progress = true;
        instance->current_cmd_handler_idx++;
        command = MfClassicListenerCommandSilent;
    } while(false);

    return command;
}

static MfClassicListenerCommand
    mf_classic_listener_value_transfer_handler(MfClassicListener* instance, BitBuffer* buff) {
    MfClassicListenerCommand command = MfClassicListenerCommandNack;
    MfClassicAuthContext* auth_ctx = &instance->auth_context;

    do {
        instance->cmd_in_progress = false;

        if(bit_buffer_get_size_bytes(buff) != 2) break;
        if(bit_buffer_get_byte(buff, 0) != MF_CLASSIC_CMD_VALUE_TRANSFER) break;

        uint8_t block_num = bit_buffer_get_byte(buff, 1);
        if(!mf_classic_is_allowed_access(
               instance->data, block_num, auth_ctx->key_type, MfClassicActionDataDec)) {
            break;
        }

        mf_classic_value_to_block(
            instance->transfer_value, block_num, &instance->data->block[block_num]);
        instance->transfer_value = 0;
        instance->transfer_valid = false;

        command = MfClassicListenerCommandAck;
    } while(false);

    return command;
}

static MfClassicListenerCommandHandler mf_classic_listener_halt_handlers[] = {
    mf_classic_listener_halt_handler,
};

static MfClassicListenerCommandHandler mf_classic_listener_auth_key_a_handlers[] = {
    mf_classic_listener_auth_key_a_handler,
    mf_classic_listener_auth_second_part_handler,
};

static MfClassicListenerCommandHandler mf_classic_listener_auth_key_b_handlers[] = {
    mf_classic_listener_auth_key_b_handler,
    mf_classic_listener_auth_second_part_handler,
};

static MfClassicListenerCommandHandler mf_classic_listener_read_block_handlers[] = {
    mf_classic_listener_read_block_handler,
};

static MfClassicListenerCommandHandler mf_classic_listener_write_block_handlers[] = {
    mf_classic_listener_write_block_first_part_handler,
    mf_classic_listener_write_block_second_part_handler,
};

static MfClassicListenerCommandHandler mf_classic_listener_value_dec_handlers[] = {
    mf_classic_listener_value_dec_handler,
    mf_classic_listener_value_data_receive_handler,
    mf_classic_listener_value_transfer_handler,
};

static MfClassicListenerCommandHandler mf_classic_listener_value_inc_handlers[] = {
    mf_classic_listener_value_inc_handler,
    mf_classic_listener_value_data_receive_handler,
    mf_classic_listener_value_transfer_handler,
};

static MfClassicListenerCommandHandler mf_classic_listener_value_restore_handlers[] = {
    mf_classic_listener_value_restore_handler,
    mf_classic_listener_value_data_receive_handler,
    mf_classic_listener_value_transfer_handler,
};

static const MfClassicListenerCmd mf_classic_listener_cmd_handlers[] = {
    {
        .cmd_start_byte = MF_CLASSIC_CMD_HALT_MSB,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_halt_handlers),
        .handler = mf_classic_listener_halt_handlers,
    },
    {
        // This crutch is necessary since some devices (like Pixel) send 15-bit "HALT" command ...
        .cmd_start_byte = MF_CLASSIC_CMD_HALT_MSB,
        .cmd_len_bits = 15,
        .command_num = COUNT_OF(mf_classic_listener_halt_handlers),
        .handler = mf_classic_listener_halt_handlers,
    },
    {
        .cmd_start_byte = MF_CLASSIC_CMD_AUTH_KEY_A,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_auth_key_a_handlers),
        .handler = mf_classic_listener_auth_key_a_handlers,
    },
    {
        .cmd_start_byte = MF_CLASSIC_CMD_AUTH_KEY_B,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_auth_key_b_handlers),
        .handler = mf_classic_listener_auth_key_b_handlers,
    },
    {
        .cmd_start_byte = MF_CLASSIC_CMD_READ_BLOCK,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_read_block_handlers),
        .handler = mf_classic_listener_read_block_handlers,
    },
    {
        .cmd_start_byte = MF_CLASSIC_CMD_WRITE_BLOCK,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_write_block_handlers),
        .handler = mf_classic_listener_write_block_handlers,
    },
    {
        .cmd_start_byte = MF_CLASSIC_CMD_VALUE_DEC,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_value_dec_handlers),
        .handler = mf_classic_listener_value_dec_handlers,
    },
    {
        .cmd_start_byte = MF_CLASSIC_CMD_VALUE_INC,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_value_inc_handlers),
        .handler = mf_classic_listener_value_inc_handlers,
    },
    {
        .cmd_start_byte = MF_CLASSIC_CMD_VALUE_RESTORE,
        .cmd_len_bits = 2 * 8,
        .command_num = COUNT_OF(mf_classic_listener_value_restore_handlers),
        .handler = mf_classic_listener_value_restore_handlers,
    },
};

static void mf_classic_listener_send_short_frame(MfClassicListener* instance, uint8_t data) {
    BitBuffer* tx_buffer = instance->tx_plain_buffer;

    bit_buffer_set_size(instance->tx_plain_buffer, 4);
    bit_buffer_set_byte(instance->tx_plain_buffer, 0, data);
    if(instance->comm_state == MfClassicListenerCommStateEncrypted) {
        crypto1_encrypt(
            instance->crypto, NULL, instance->tx_plain_buffer, instance->tx_encrypted_buffer);
        tx_buffer = instance->tx_encrypted_buffer;
    }

    iso14443_3a_listener_tx_with_custom_parity(instance->iso14443_3a_listener, tx_buffer);
}

NfcCommand mf_classic_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);

    NfcCommand command = NfcCommandContinue;
    MfClassicListener* instance = context;
    Iso14443_3aListenerEvent* iso3_event = event.event_data;
    BitBuffer* rx_buffer_plain;

    if(iso3_event->type == Iso14443_3aListenerEventTypeFieldOff) {
        mf_classic_listener_reset_state(instance);
        command = NfcCommandSleep;
    } else if(
        (iso3_event->type == Iso14443_3aListenerEventTypeReceivedData) ||
        (iso3_event->type == Iso14443_3aListenerEventTypeReceivedStandardFrame)) {
        if(instance->comm_state == MfClassicListenerCommStateEncrypted) {
            if(instance->state == MfClassicListenerStateAuthComplete) {
                crypto1_decrypt(
                    instance->crypto, iso3_event->data->buffer, instance->rx_plain_buffer);
                rx_buffer_plain = instance->rx_plain_buffer;
                if(iso14443_crc_check(Iso14443CrcTypeA, rx_buffer_plain)) {
                    iso14443_crc_trim(rx_buffer_plain);
                }
            } else {
                rx_buffer_plain = iso3_event->data->buffer;
            }
        } else {
            rx_buffer_plain = iso3_event->data->buffer;
        }

        MfClassicListenerCommand mfc_command = MfClassicListenerCommandNack;
        if(instance->cmd_in_progress) {
            mfc_command =
                mf_classic_listener_cmd_handlers[instance->current_cmd_idx]
                    .handler[instance->current_cmd_handler_idx](instance, rx_buffer_plain);
        } else {
            for(size_t i = 0; i < COUNT_OF(mf_classic_listener_cmd_handlers); i++) {
                if(bit_buffer_get_size(rx_buffer_plain) !=
                   mf_classic_listener_cmd_handlers[i].cmd_len_bits) {
                    continue;
                }
                if(bit_buffer_get_byte(rx_buffer_plain, 0) !=
                   mf_classic_listener_cmd_handlers[i].cmd_start_byte) {
                    continue;
                }
                instance->current_cmd_idx = i;
                instance->current_cmd_handler_idx = 0;
                mfc_command =
                    mf_classic_listener_cmd_handlers[i].handler[0](instance, rx_buffer_plain);
                break;
            }
        }

        if(mfc_command == MfClassicListenerCommandAck) {
            mf_classic_listener_send_short_frame(instance, MF_CLASSIC_CMD_ACK);
        } else if(mfc_command == MfClassicListenerCommandNack) {
            // Calculate nack based on the transfer buffer validity
            uint8_t nack = MF_CLASSIC_CMD_NACK;
            if(!instance->transfer_valid) {
                nack += MF_CLASSIC_CMD_NACK_TRANSFER_INVALID;
            }

            mf_classic_listener_send_short_frame(instance, nack);
            mf_classic_listener_reset_state(instance);
            command = NfcCommandSleep;
        } else if(mfc_command == MfClassicListenerCommandSilent) {
            command = NfcCommandReset;
        } else if(mfc_command == MfClassicListenerCommandSleep) {
            mf_classic_listener_reset_state(instance);
            command = NfcCommandSleep;
        }
    } else if(iso3_event->type == Iso14443_3aListenerEventTypeHalted) {
        mf_classic_listener_reset_state(instance);
    }

    return command;
}

MfClassicListener*
    mf_classic_listener_alloc(Iso14443_3aListener* iso14443_3a_listener, MfClassicData* data) {
    MfClassicListener* instance = malloc(sizeof(MfClassicListener));
    instance->iso14443_3a_listener = iso14443_3a_listener;
    instance->data = data;
    mf_classic_listener_prepare_emulation(instance);

    instance->crypto = crypto1_alloc();
    instance->tx_plain_buffer = bit_buffer_alloc(MF_CLASSIC_MAX_BUFF_SIZE);
    instance->tx_encrypted_buffer = bit_buffer_alloc(MF_CLASSIC_MAX_BUFF_SIZE);
    instance->rx_plain_buffer = bit_buffer_alloc(MF_CLASSIC_MAX_BUFF_SIZE);

    instance->mfc_event.data = &instance->mfc_event_data;
    instance->generic_event.protocol = NfcProtocolMfClassic;
    instance->generic_event.event_data = &instance->mfc_event;
    instance->generic_event.instance = instance;

    return instance;
}

void mf_classic_listener_free(MfClassicListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    furi_assert(instance->crypto);
    furi_assert(instance->rx_plain_buffer);
    furi_assert(instance->tx_encrypted_buffer);
    furi_assert(instance->tx_plain_buffer);

    crypto1_free(instance->crypto);
    bit_buffer_free(instance->rx_plain_buffer);
    bit_buffer_free(instance->tx_encrypted_buffer);
    bit_buffer_free(instance->tx_plain_buffer);

    free(instance);
}

void mf_classic_listener_set_callback(
    MfClassicListener* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

const MfClassicData* mf_classic_listener_get_data(const MfClassicListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

const NfcListenerBase mf_classic_listener = {
    .alloc = (NfcListenerAlloc)mf_classic_listener_alloc,
    .free = (NfcListenerFree)mf_classic_listener_free,
    .set_callback = (NfcListenerSetCallback)mf_classic_listener_set_callback,
    .get_data = (NfcListenerGetData)mf_classic_listener_get_data,
    .run = (NfcListenerRun)mf_classic_listener_run,
};
