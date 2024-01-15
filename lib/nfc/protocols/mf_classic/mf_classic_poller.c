#include "mf_classic_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "MfClassicPoller"

#define MF_CLASSIC_MAX_BUFF_SIZE (64)

typedef NfcCommand (*MfClassicPollerReadHandler)(MfClassicPoller* instance);

MfClassicPoller* mf_classic_poller_alloc(Iso14443_3aPoller* iso14443_3a_poller) {
    furi_assert(iso14443_3a_poller);

    MfClassicPoller* instance = malloc(sizeof(MfClassicPoller));
    instance->iso14443_3a_poller = iso14443_3a_poller;
    instance->data = mf_classic_alloc();
    instance->crypto = crypto1_alloc();
    instance->tx_plain_buffer = bit_buffer_alloc(MF_CLASSIC_MAX_BUFF_SIZE);
    instance->tx_encrypted_buffer = bit_buffer_alloc(MF_CLASSIC_MAX_BUFF_SIZE);
    instance->rx_plain_buffer = bit_buffer_alloc(MF_CLASSIC_MAX_BUFF_SIZE);
    instance->rx_encrypted_buffer = bit_buffer_alloc(MF_CLASSIC_MAX_BUFF_SIZE);
    instance->current_type_check = MfClassicType4k;
    instance->card_state = MfClassicCardStateLost;

    instance->mfc_event.data = &instance->mfc_event_data;

    instance->general_event.protocol = NfcProtocolMfClassic;
    instance->general_event.event_data = &instance->mfc_event;
    instance->general_event.instance = instance;

    return instance;
}

void mf_classic_poller_free(MfClassicPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    furi_assert(instance->crypto);
    furi_assert(instance->tx_plain_buffer);
    furi_assert(instance->rx_plain_buffer);
    furi_assert(instance->tx_encrypted_buffer);
    furi_assert(instance->rx_encrypted_buffer);

    mf_classic_free(instance->data);
    crypto1_free(instance->crypto);
    bit_buffer_free(instance->tx_plain_buffer);
    bit_buffer_free(instance->rx_plain_buffer);
    bit_buffer_free(instance->tx_encrypted_buffer);
    bit_buffer_free(instance->rx_encrypted_buffer);

    free(instance);
}

static NfcCommand mf_classic_poller_handle_data_update(MfClassicPoller* instance) {
    MfClassicPollerEventDataUpdate* data_update = &instance->mfc_event_data.data_update;

    mf_classic_get_read_sectors_and_keys(
        instance->data, &data_update->sectors_read, &data_update->keys_found);
    data_update->current_sector = instance->mode_ctx.dict_attack_ctx.current_sector;
    instance->mfc_event.type = MfClassicPollerEventTypeDataUpdate;
    return instance->callback(instance->general_event, instance->context);
}

static void mf_classic_poller_check_key_b_is_readable(
    MfClassicPoller* instance,
    uint8_t block_num,
    MfClassicBlock* data) {
    do {
        if(!mf_classic_is_sector_trailer(block_num)) break;
        if(!mf_classic_is_allowed_access(
               instance->data, block_num, MfClassicKeyTypeA, MfClassicActionKeyBRead))
            break;

        MfClassicSectorTrailer* sec_tr = (MfClassicSectorTrailer*)data;
        uint64_t key_b = nfc_util_bytes2num(sec_tr->key_b.data, sizeof(MfClassicKey));
        uint8_t sector_num = mf_classic_get_sector_by_block(block_num);
        mf_classic_set_key_found(instance->data, sector_num, MfClassicKeyTypeB, key_b);
    } while(false);
}

NfcCommand mf_classic_poller_handler_detect_type(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandReset;

    if(instance->current_type_check == MfClassicType4k) {
        iso14443_3a_copy(
            instance->data->iso14443_3a_data,
            iso14443_3a_poller_get_data(instance->iso14443_3a_poller));
        MfClassicError error = mf_classic_poller_get_nt(instance, 254, MfClassicKeyTypeA, NULL);
        if(error == MfClassicErrorNone) {
            instance->data->type = MfClassicType4k;
            instance->state = MfClassicPollerStateStart;
            instance->current_type_check = MfClassicType4k;
            FURI_LOG_D(TAG, "4K detected");
        } else {
            instance->current_type_check = MfClassicType1k;
        }
    } else if(instance->current_type_check == MfClassicType1k) {
        MfClassicError error = mf_classic_poller_get_nt(instance, 62, MfClassicKeyTypeA, NULL);
        if(error == MfClassicErrorNone) {
            instance->data->type = MfClassicType1k;
            FURI_LOG_D(TAG, "1K detected");
        } else {
            instance->data->type = MfClassicTypeMini;
            FURI_LOG_D(TAG, "Mini detected");
        }
        instance->current_type_check = MfClassicType4k;
        instance->state = MfClassicPollerStateStart;
    }

    return command;
}

NfcCommand mf_classic_poller_handler_start(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    instance->sectors_total = mf_classic_get_total_sectors_num(instance->data->type);
    memset(&instance->mode_ctx, 0, sizeof(MfClassicPollerModeContext));

    instance->mfc_event.type = MfClassicPollerEventTypeRequestMode;
    command = instance->callback(instance->general_event, instance->context);

    if(instance->mfc_event_data.poller_mode.mode == MfClassicPollerModeDictAttack) {
        mf_classic_copy(instance->data, instance->mfc_event_data.poller_mode.data);
        instance->state = MfClassicPollerStateRequestKey;
    } else if(instance->mfc_event_data.poller_mode.mode == MfClassicPollerModeRead) {
        instance->state = MfClassicPollerStateRequestReadSector;
    } else if(instance->mfc_event_data.poller_mode.mode == MfClassicPollerModeWrite) {
        instance->state = MfClassicPollerStateRequestSectorTrailer;
    } else {
        furi_crash("Invalid mode selected");
    }

    return command;
}

NfcCommand mf_classic_poller_handler_request_sector_trailer(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerWriteContext* write_ctx = &instance->mode_ctx.write_ctx;

    if(write_ctx->current_sector == instance->sectors_total) {
        instance->state = MfClassicPollerStateSuccess;
    } else {
        instance->mfc_event.type = MfClassicPollerEventTypeRequestSectorTrailer;
        instance->mfc_event_data.sec_tr_data.sector_num = write_ctx->current_sector;
        command = instance->callback(instance->general_event, instance->context);
        if(instance->mfc_event_data.sec_tr_data.sector_trailer_provided) {
            instance->state = MfClassicPollerStateCheckWriteConditions;
            memcpy(
                &write_ctx->sec_tr,
                &instance->mfc_event_data.sec_tr_data.sector_trailer,
                sizeof(MfClassicSectorTrailer));
            write_ctx->current_block =
                MAX(1, mf_classic_get_first_block_num_of_sector(write_ctx->current_sector));

        } else {
            write_ctx->current_sector++;
        }
    }

    return command;
}

NfcCommand mf_classic_handler_check_write_conditions(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    MfClassicPollerWriteContext* write_ctx = &instance->mode_ctx.write_ctx;
    MfClassicSectorTrailer* sec_tr = &write_ctx->sec_tr;

    do {
        // Check last block in sector to write
        uint8_t sec_tr_block_num =
            mf_classic_get_sector_trailer_num_by_sector(write_ctx->current_sector);
        if(write_ctx->current_block == sec_tr_block_num) {
            write_ctx->current_sector++;
            instance->state = MfClassicPollerStateRequestSectorTrailer;
            break;
        }

        // Check write and read access
        if(mf_classic_is_allowed_access_data_block(
               sec_tr, write_ctx->current_block, MfClassicKeyTypeA, MfClassicActionDataWrite)) {
            write_ctx->key_type_write = MfClassicKeyTypeA;
        } else if(mf_classic_is_allowed_access_data_block(
                      sec_tr,
                      write_ctx->current_block,
                      MfClassicKeyTypeB,
                      MfClassicActionDataWrite)) {
            write_ctx->key_type_write = MfClassicKeyTypeB;
        } else if(mf_classic_is_value_block(sec_tr, write_ctx->current_block)) {
            write_ctx->is_value_block = true;
        } else {
            FURI_LOG_D(TAG, "Not allowed to write block %d", write_ctx->current_block);
            write_ctx->current_block++;
            break;
        }

        if(mf_classic_is_allowed_access_data_block(
               sec_tr,
               write_ctx->current_block,
               write_ctx->key_type_write,
               MfClassicActionDataRead)) {
            write_ctx->key_type_read = write_ctx->key_type_write;
        } else {
            write_ctx->key_type_read = write_ctx->key_type_write == MfClassicKeyTypeA ?
                                           MfClassicKeyTypeB :
                                           MfClassicKeyTypeA;
            if(!mf_classic_is_allowed_access_data_block(
                   sec_tr,
                   write_ctx->current_block,
                   write_ctx->key_type_read,
                   MfClassicActionDataRead)) {
                FURI_LOG_D(TAG, "Not allowed to read block %d", write_ctx->current_block);
                write_ctx->current_block++;
                break;
            }
        }

        write_ctx->need_halt_before_write =
            (write_ctx->key_type_read != write_ctx->key_type_write);
        instance->state = MfClassicPollerStateReadBlock;
    } while(false);

    return command;
}

NfcCommand mf_classic_poller_handler_read_block(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerWriteContext* write_ctx = &instance->mode_ctx.write_ctx;

    MfClassicKey* auth_key = write_ctx->key_type_read == MfClassicKeyTypeA ?
                                 &write_ctx->sec_tr.key_a :
                                 &write_ctx->sec_tr.key_b;
    MfClassicError error = MfClassicErrorNone;

    do {
        // Authenticate to sector
        error = mf_classic_poller_auth(
            instance, write_ctx->current_block, auth_key, write_ctx->key_type_read, NULL);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to auth to block %d", write_ctx->current_block);
            instance->state = MfClassicPollerStateFail;
            break;
        }

        // Read block from tag
        error = mf_classic_poller_read_block(
            instance, write_ctx->current_block, &write_ctx->tag_block);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %d", write_ctx->current_block);
            instance->state = MfClassicPollerStateFail;
            break;
        }

        if(write_ctx->is_value_block) {
            mf_classic_poller_halt(instance);
            instance->state = MfClassicPollerStateWriteValueBlock;
        } else {
            if(write_ctx->need_halt_before_write) {
                mf_classic_poller_halt(instance);
            }
            instance->state = MfClassicPollerStateWriteBlock;
        }
    } while(false);

    return command;
}

NfcCommand mf_classic_poller_handler_write_block(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    MfClassicPollerWriteContext* write_ctx = &instance->mode_ctx.write_ctx;
    MfClassicKey* auth_key = write_ctx->key_type_write == MfClassicKeyTypeA ?
                                 &write_ctx->sec_tr.key_a :
                                 &write_ctx->sec_tr.key_b;
    MfClassicError error = MfClassicErrorNone;

    do {
        // Request block to write
        instance->mfc_event.type = MfClassicPollerEventTypeRequestWriteBlock;
        instance->mfc_event_data.write_block_data.block_num = write_ctx->current_block;
        command = instance->callback(instance->general_event, instance->context);
        if(!instance->mfc_event_data.write_block_data.write_block_provided) break;

        // Compare tag and saved block
        if(memcmp(
               write_ctx->tag_block.data,
               instance->mfc_event_data.write_block_data.write_block.data,
               sizeof(MfClassicBlock)) == 0) {
            FURI_LOG_D(TAG, "Block %d is equal. Skip writing", write_ctx->current_block);
            break;
        }

        // Reauth if necessary
        if(write_ctx->need_halt_before_write) {
            error = mf_classic_poller_auth(
                instance, write_ctx->current_block, auth_key, write_ctx->key_type_write, NULL);
            if(error != MfClassicErrorNone) {
                FURI_LOG_D(
                    TAG, "Failed to auth to block %d for writing", write_ctx->current_block);
                instance->state = MfClassicPollerStateFail;
                break;
            }
        }

        // Write block
        error = mf_classic_poller_write_block(
            instance,
            write_ctx->current_block,
            &instance->mfc_event_data.write_block_data.write_block);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to write block %d", write_ctx->current_block);
            instance->state = MfClassicPollerStateFail;
            break;
        }

    } while(false);

    mf_classic_poller_halt(instance);
    write_ctx->current_block++;
    instance->state = MfClassicPollerStateCheckWriteConditions;

    return command;
}

NfcCommand mf_classic_poller_handler_write_value_block(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerWriteContext* write_ctx = &instance->mode_ctx.write_ctx;

    do {
        // Request block to write
        instance->mfc_event.type = MfClassicPollerEventTypeRequestWriteBlock;
        instance->mfc_event_data.write_block_data.block_num = write_ctx->current_block;
        command = instance->callback(instance->general_event, instance->context);
        if(!instance->mfc_event_data.write_block_data.write_block_provided) break;

        // Compare tag and saved block
        if(memcmp(
               write_ctx->tag_block.data,
               instance->mfc_event_data.write_block_data.write_block.data,
               sizeof(MfClassicBlock)) == 0) {
            FURI_LOG_D(TAG, "Block %d is equal. Skip writing", write_ctx->current_block);
            break;
        }

        bool key_a_inc_allowed = mf_classic_is_allowed_access_data_block(
            &write_ctx->sec_tr,
            write_ctx->current_block,
            MfClassicKeyTypeA,
            MfClassicActionDataInc);
        bool key_b_inc_allowed = mf_classic_is_allowed_access_data_block(
            &write_ctx->sec_tr,
            write_ctx->current_block,
            MfClassicKeyTypeB,
            MfClassicActionDataInc);
        bool key_a_dec_allowed = mf_classic_is_allowed_access_data_block(
            &write_ctx->sec_tr,
            write_ctx->current_block,
            MfClassicKeyTypeA,
            MfClassicActionDataDec);
        bool key_b_dec_allowed = mf_classic_is_allowed_access_data_block(
            &write_ctx->sec_tr,
            write_ctx->current_block,
            MfClassicKeyTypeB,
            MfClassicActionDataDec);

        int32_t source_value = 0;
        int32_t target_value = 0;
        if(!mf_classic_block_to_value(
               &instance->mfc_event_data.write_block_data.write_block, &source_value, NULL))
            break;
        if(!mf_classic_block_to_value(&write_ctx->tag_block, &target_value, NULL)) break;

        MfClassicKeyType auth_key_type = MfClassicKeyTypeA;
        MfClassicValueCommand value_cmd = MfClassicValueCommandIncrement;
        int32_t diff = source_value - target_value;
        if(diff > 0) {
            if(key_a_inc_allowed) {
                auth_key_type = MfClassicKeyTypeA;
                value_cmd = MfClassicValueCommandIncrement;
            } else if(key_b_inc_allowed) {
                auth_key_type = MfClassicKeyTypeB;
                value_cmd = MfClassicValueCommandIncrement;
            } else {
                FURI_LOG_D(TAG, "Unable to increment value block");
                break;
            }
        } else {
            if(key_a_dec_allowed) {
                auth_key_type = MfClassicKeyTypeA;
                value_cmd = MfClassicValueCommandDecrement;
                diff *= -1;
            } else if(key_b_dec_allowed) {
                auth_key_type = MfClassicKeyTypeB;
                value_cmd = MfClassicValueCommandDecrement;
                diff *= -1;
            } else {
                FURI_LOG_D(TAG, "Unable to decrement value block");
                break;
            }
        }

        MfClassicKey* key = (auth_key_type == MfClassicKeyTypeA) ? &write_ctx->sec_tr.key_a :
                                                                   &write_ctx->sec_tr.key_b;

        MfClassicError error =
            mf_classic_poller_auth(instance, write_ctx->current_block, key, auth_key_type, NULL);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_value_cmd(instance, write_ctx->current_block, value_cmd, diff);
        if(error != MfClassicErrorNone) break;

        error = mf_classic_poller_value_transfer(instance, write_ctx->current_block);
        if(error != MfClassicErrorNone) break;

    } while(false);

    mf_classic_poller_halt(instance);
    write_ctx->is_value_block = false;
    write_ctx->current_block++;
    instance->state = MfClassicPollerStateCheckWriteConditions;

    return command;
}

NfcCommand mf_classic_poller_handler_request_read_sector(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    MfClassicPollerReadContext* sec_read_ctx = &instance->mode_ctx.read_ctx;
    MfClassicPollerEventDataReadSectorRequest* sec_read =
        &instance->mfc_event_data.read_sector_request_data;
    instance->mfc_event.type = MfClassicPollerEventTypeRequestReadSector;
    command = instance->callback(instance->general_event, instance->context);

    if(!sec_read->key_provided) {
        instance->state = MfClassicPollerStateSuccess;
    } else {
        sec_read_ctx->current_sector = sec_read->sector_num;
        sec_read_ctx->key = sec_read->key;
        sec_read_ctx->key_type = sec_read->key_type;
        sec_read_ctx->current_block =
            mf_classic_get_first_block_num_of_sector(sec_read->sector_num);
        sec_read_ctx->auth_passed = false;
        instance->state = MfClassicPollerStateReadSectorBlocks;
    }

    return command;
}

NfcCommand mf_classic_poller_handler_request_read_sector_blocks(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;

    MfClassicPollerReadContext* sec_read_ctx = &instance->mode_ctx.read_ctx;

    do {
        MfClassicError error = MfClassicErrorNone;

        if(!sec_read_ctx->auth_passed) {
            uint64_t key = nfc_util_bytes2num(sec_read_ctx->key.data, sizeof(MfClassicKey));
            FURI_LOG_D(
                TAG,
                "Auth to block %d with key %c: %06llx",
                sec_read_ctx->current_block,
                sec_read_ctx->key_type == MfClassicKeyTypeA ? 'A' : 'B',
                key);
            error = mf_classic_poller_auth(
                instance,
                sec_read_ctx->current_block,
                &sec_read_ctx->key,
                sec_read_ctx->key_type,
                NULL);
            if(error != MfClassicErrorNone) break;

            sec_read_ctx->auth_passed = true;
            if(!mf_classic_is_key_found(
                   instance->data, sec_read_ctx->current_sector, sec_read_ctx->key_type)) {
                mf_classic_set_key_found(
                    instance->data, sec_read_ctx->current_sector, sec_read_ctx->key_type, key);
            }
        }
        if(mf_classic_is_block_read(instance->data, sec_read_ctx->current_block)) break;

        FURI_LOG_D(TAG, "Reading block %d", sec_read_ctx->current_block);
        MfClassicBlock read_block = {};
        error = mf_classic_poller_read_block(instance, sec_read_ctx->current_block, &read_block);
        if(error == MfClassicErrorNone) {
            mf_classic_set_block_read(instance->data, sec_read_ctx->current_block, &read_block);
            if(sec_read_ctx->key_type == MfClassicKeyTypeA) {
                mf_classic_poller_check_key_b_is_readable(
                    instance, sec_read_ctx->current_block, &read_block);
            }
        } else {
            mf_classic_poller_halt(instance);
            sec_read_ctx->auth_passed = false;
        }
    } while(false);

    uint8_t sec_tr_num = mf_classic_get_sector_trailer_num_by_sector(sec_read_ctx->current_sector);
    sec_read_ctx->current_block++;
    if(sec_read_ctx->current_block > sec_tr_num) {
        mf_classic_poller_halt(instance);
        instance->state = MfClassicPollerStateRequestReadSector;
    }

    return command;
}

NfcCommand mf_classic_poller_handler_request_key(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    instance->mfc_event.type = MfClassicPollerEventTypeRequestKey;
    command = instance->callback(instance->general_event, instance->context);
    if(instance->mfc_event_data.key_request_data.key_provided) {
        dict_attack_ctx->current_key = instance->mfc_event_data.key_request_data.key;
        instance->state = MfClassicPollerStateAuthKeyA;
    } else {
        instance->state = MfClassicPollerStateNextSector;
    }

    return command;
}

NfcCommand mf_classic_poller_handler_auth_a(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    if(mf_classic_is_key_found(
           instance->data, dict_attack_ctx->current_sector, MfClassicKeyTypeA)) {
        instance->state = MfClassicPollerStateAuthKeyB;
    } else {
        uint8_t block = mf_classic_get_first_block_num_of_sector(dict_attack_ctx->current_sector);
        uint64_t key = nfc_util_bytes2num(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Auth to block %d with key A: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeA, NULL);
        if(error == MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Key A found");
            mf_classic_set_key_found(
                instance->data, dict_attack_ctx->current_sector, MfClassicKeyTypeA, key);

            command = mf_classic_poller_handle_data_update(instance);
            dict_attack_ctx->current_key_type = MfClassicKeyTypeA;
            dict_attack_ctx->current_block = block;
            dict_attack_ctx->auth_passed = true;
            instance->state = MfClassicPollerStateReadSector;
        } else {
            mf_classic_poller_halt(instance);
            instance->state = MfClassicPollerStateAuthKeyB;
        }
    }

    return command;
}

NfcCommand mf_classic_poller_handler_auth_b(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    if(mf_classic_is_key_found(
           instance->data, dict_attack_ctx->current_sector, MfClassicKeyTypeB)) {
        if(mf_classic_is_key_found(
               instance->data, dict_attack_ctx->current_sector, MfClassicKeyTypeA)) {
            instance->state = MfClassicPollerStateNextSector;
        } else {
            instance->state = MfClassicPollerStateRequestKey;
        }
    } else {
        uint8_t block = mf_classic_get_first_block_num_of_sector(dict_attack_ctx->current_sector);
        uint64_t key = nfc_util_bytes2num(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Auth to block %d with key B: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeB, NULL);
        if(error == MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Key B found");
            mf_classic_set_key_found(
                instance->data, dict_attack_ctx->current_sector, MfClassicKeyTypeB, key);

            command = mf_classic_poller_handle_data_update(instance);
            dict_attack_ctx->current_key_type = MfClassicKeyTypeB;
            dict_attack_ctx->current_block = block;

            dict_attack_ctx->auth_passed = true;
            instance->state = MfClassicPollerStateReadSector;
        } else {
            mf_classic_poller_halt(instance);
            instance->state = MfClassicPollerStateRequestKey;
        }
    }

    return command;
}

NfcCommand mf_classic_poller_handler_next_sector(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    dict_attack_ctx->current_sector++;
    if(dict_attack_ctx->current_sector == instance->sectors_total) {
        instance->state = MfClassicPollerStateSuccess;
    } else {
        instance->mfc_event.type = MfClassicPollerEventTypeNextSector;
        instance->mfc_event_data.next_sector_data.current_sector = dict_attack_ctx->current_sector;
        command = instance->callback(instance->general_event, instance->context);
        instance->state = MfClassicPollerStateRequestKey;
    }

    return command;
}

NfcCommand mf_classic_poller_handler_read_sector(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    MfClassicError error = MfClassicErrorNone;
    uint8_t block_num = dict_attack_ctx->current_block;
    MfClassicBlock block = {};

    do {
        if(mf_classic_is_block_read(instance->data, block_num)) break;

        if(!dict_attack_ctx->auth_passed) {
            error = mf_classic_poller_auth(
                instance,
                block_num,
                &dict_attack_ctx->current_key,
                dict_attack_ctx->current_key_type,
                NULL);
            if(error != MfClassicErrorNone) {
                instance->state = MfClassicPollerStateNextSector;
                FURI_LOG_W(TAG, "Failed to re-auth. Go to next sector");
                break;
            }
        }

        FURI_LOG_D(TAG, "Reading block %d", block_num);
        error = mf_classic_poller_read_block(instance, block_num, &block);

        if(error != MfClassicErrorNone) {
            mf_classic_poller_halt(instance);
            dict_attack_ctx->auth_passed = false;
            FURI_LOG_D(TAG, "Failed to read block %d", block_num);
        } else {
            mf_classic_set_block_read(instance->data, block_num, &block);
            if(dict_attack_ctx->current_key_type == MfClassicKeyTypeA) {
                mf_classic_poller_check_key_b_is_readable(instance, block_num, &block);
            }
        }
    } while(false);

    uint8_t sec_tr_block_num =
        mf_classic_get_sector_trailer_num_by_sector(dict_attack_ctx->current_sector);
    dict_attack_ctx->current_block++;
    if(dict_attack_ctx->current_block > sec_tr_block_num) {
        mf_classic_poller_handle_data_update(instance);

        mf_classic_poller_halt(instance);
        dict_attack_ctx->auth_passed = false;

        if(dict_attack_ctx->current_sector == instance->sectors_total) {
            instance->state = MfClassicPollerStateNextSector;
        } else {
            dict_attack_ctx->reuse_key_sector = dict_attack_ctx->current_sector;
            instance->mfc_event.type = MfClassicPollerEventTypeKeyAttackStart;
            instance->mfc_event_data.key_attack_data.current_sector =
                dict_attack_ctx->reuse_key_sector;
            command = instance->callback(instance->general_event, instance->context);
            instance->state = MfClassicPollerStateKeyReuseStart;
        }
    }

    return command;
}

NfcCommand mf_classic_poller_handler_key_reuse_start(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    if(dict_attack_ctx->current_key_type == MfClassicKeyTypeA) {
        dict_attack_ctx->current_key_type = MfClassicKeyTypeB;
        instance->state = MfClassicPollerStateKeyReuseAuthKeyB;
    } else {
        dict_attack_ctx->reuse_key_sector++;
        if(dict_attack_ctx->reuse_key_sector == instance->sectors_total) {
            instance->mfc_event.type = MfClassicPollerEventTypeKeyAttackStop;
            command = instance->callback(instance->general_event, instance->context);
            instance->state = MfClassicPollerStateRequestKey;
        } else {
            instance->mfc_event.type = MfClassicPollerEventTypeKeyAttackStart;
            instance->mfc_event_data.key_attack_data.current_sector =
                dict_attack_ctx->reuse_key_sector;
            command = instance->callback(instance->general_event, instance->context);

            dict_attack_ctx->current_key_type = MfClassicKeyTypeA;
            instance->state = MfClassicPollerStateKeyReuseAuthKeyA;
        }
    }

    return command;
}

NfcCommand mf_classic_poller_handler_key_reuse_auth_key_a(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    if(mf_classic_is_key_found(
           instance->data, dict_attack_ctx->reuse_key_sector, MfClassicKeyTypeA)) {
        instance->state = MfClassicPollerStateKeyReuseStart;
    } else {
        uint8_t block =
            mf_classic_get_first_block_num_of_sector(dict_attack_ctx->reuse_key_sector);
        uint64_t key = nfc_util_bytes2num(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Key attack auth to block %d with key A: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeA, NULL);
        if(error == MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Key A found");
            mf_classic_set_key_found(
                instance->data, dict_attack_ctx->reuse_key_sector, MfClassicKeyTypeA, key);

            command = mf_classic_poller_handle_data_update(instance);
            dict_attack_ctx->current_key_type = MfClassicKeyTypeA;
            dict_attack_ctx->current_block = block;
            dict_attack_ctx->auth_passed = true;
            instance->state = MfClassicPollerStateKeyReuseReadSector;
        } else {
            mf_classic_poller_halt(instance);
            dict_attack_ctx->auth_passed = false;
            instance->state = MfClassicPollerStateKeyReuseStart;
        }
    }

    return command;
}

NfcCommand mf_classic_poller_handler_key_reuse_auth_key_b(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    if(mf_classic_is_key_found(
           instance->data, dict_attack_ctx->reuse_key_sector, MfClassicKeyTypeB)) {
        instance->state = MfClassicPollerStateKeyReuseStart;
    } else {
        uint8_t block =
            mf_classic_get_first_block_num_of_sector(dict_attack_ctx->reuse_key_sector);
        uint64_t key = nfc_util_bytes2num(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Key attack auth to block %d with key B: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeB, NULL);
        if(error == MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Key B found");
            mf_classic_set_key_found(
                instance->data, dict_attack_ctx->reuse_key_sector, MfClassicKeyTypeB, key);

            command = mf_classic_poller_handle_data_update(instance);
            dict_attack_ctx->current_key_type = MfClassicKeyTypeB;
            dict_attack_ctx->current_block = block;

            dict_attack_ctx->auth_passed = true;
            instance->state = MfClassicPollerStateKeyReuseReadSector;
        } else {
            mf_classic_poller_halt(instance);
            dict_attack_ctx->auth_passed = false;
            instance->state = MfClassicPollerStateKeyReuseStart;
        }
    }

    return command;
}

NfcCommand mf_classic_poller_handler_key_reuse_read_sector(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    MfClassicError error = MfClassicErrorNone;
    uint8_t block_num = dict_attack_ctx->current_block;
    MfClassicBlock block = {};

    do {
        if(mf_classic_is_block_read(instance->data, block_num)) break;

        if(!dict_attack_ctx->auth_passed) {
            error = mf_classic_poller_auth(
                instance,
                block_num,
                &dict_attack_ctx->current_key,
                dict_attack_ctx->current_key_type,
                NULL);
            if(error != MfClassicErrorNone) {
                instance->state = MfClassicPollerStateKeyReuseStart;
                break;
            }
        }

        FURI_LOG_D(TAG, "Reading block %d", block_num);
        error = mf_classic_poller_read_block(instance, block_num, &block);

        if(error != MfClassicErrorNone) {
            mf_classic_poller_halt(instance);
            dict_attack_ctx->auth_passed = false;
            FURI_LOG_D(TAG, "Failed to read block %d", block_num);
        } else {
            mf_classic_set_block_read(instance->data, block_num, &block);
            if(dict_attack_ctx->current_key_type == MfClassicKeyTypeA) {
                mf_classic_poller_check_key_b_is_readable(instance, block_num, &block);
            }
        }
    } while(false);

    uint16_t sec_tr_block_num =
        mf_classic_get_sector_trailer_num_by_sector(dict_attack_ctx->reuse_key_sector);
    dict_attack_ctx->current_block++;
    if(dict_attack_ctx->current_block > sec_tr_block_num) {
        mf_classic_poller_halt(instance);
        dict_attack_ctx->auth_passed = false;

        mf_classic_poller_handle_data_update(instance);
        instance->state = MfClassicPollerStateKeyReuseStart;
    }

    return command;
}

NfcCommand mf_classic_poller_handler_success(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    instance->mfc_event.type = MfClassicPollerEventTypeSuccess;
    command = instance->callback(instance->general_event, instance->context);

    return command;
}

NfcCommand mf_classic_poller_handler_fail(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    instance->mfc_event.type = MfClassicPollerEventTypeFail;
    command = instance->callback(instance->general_event, instance->context);
    instance->state = MfClassicPollerStateDetectType;

    return command;
}

static const MfClassicPollerReadHandler
    mf_classic_poller_dict_attack_handler[MfClassicPollerStateNum] = {
        [MfClassicPollerStateDetectType] = mf_classic_poller_handler_detect_type,
        [MfClassicPollerStateStart] = mf_classic_poller_handler_start,
        [MfClassicPollerStateRequestSectorTrailer] =
            mf_classic_poller_handler_request_sector_trailer,
        [MfClassicPollerStateCheckWriteConditions] = mf_classic_handler_check_write_conditions,
        [MfClassicPollerStateReadBlock] = mf_classic_poller_handler_read_block,
        [MfClassicPollerStateWriteBlock] = mf_classic_poller_handler_write_block,
        [MfClassicPollerStateWriteValueBlock] = mf_classic_poller_handler_write_value_block,
        [MfClassicPollerStateNextSector] = mf_classic_poller_handler_next_sector,
        [MfClassicPollerStateRequestKey] = mf_classic_poller_handler_request_key,
        [MfClassicPollerStateRequestReadSector] = mf_classic_poller_handler_request_read_sector,
        [MfClassicPollerStateReadSectorBlocks] =
            mf_classic_poller_handler_request_read_sector_blocks,
        [MfClassicPollerStateAuthKeyA] = mf_classic_poller_handler_auth_a,
        [MfClassicPollerStateAuthKeyB] = mf_classic_poller_handler_auth_b,
        [MfClassicPollerStateReadSector] = mf_classic_poller_handler_read_sector,
        [MfClassicPollerStateKeyReuseStart] = mf_classic_poller_handler_key_reuse_start,
        [MfClassicPollerStateKeyReuseAuthKeyA] = mf_classic_poller_handler_key_reuse_auth_key_a,
        [MfClassicPollerStateKeyReuseAuthKeyB] = mf_classic_poller_handler_key_reuse_auth_key_b,
        [MfClassicPollerStateKeyReuseReadSector] = mf_classic_poller_handler_key_reuse_read_sector,
        [MfClassicPollerStateSuccess] = mf_classic_poller_handler_success,
        [MfClassicPollerStateFail] = mf_classic_poller_handler_fail,
};

NfcCommand mf_classic_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);
    furi_assert(context);

    MfClassicPoller* instance = context;
    Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        if(instance->card_state == MfClassicCardStateLost) {
            instance->card_state = MfClassicCardStateDetected;
            instance->mfc_event.type = MfClassicPollerEventTypeCardDetected;
            instance->callback(instance->general_event, instance->context);
        }
        command = mf_classic_poller_dict_attack_handler[instance->state](instance);
    } else if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeError) {
        if(instance->card_state == MfClassicCardStateDetected) {
            instance->card_state = MfClassicCardStateLost;
            instance->mfc_event.type = MfClassicPollerEventTypeCardLost;
            command = instance->callback(instance->general_event, instance->context);
        }
    }

    return command;
}

bool mf_classic_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);
    furi_assert(context);

    Iso14443_3aPoller* iso3_poller = event.instance;
    Iso14443_3aPollerEvent* iso14443_3a_event = event.event_data;
    bool detected = false;
    const uint8_t auth_cmd[] = {MF_CLASSIC_CMD_AUTH_KEY_A, 0};
    BitBuffer* tx_buffer = bit_buffer_alloc(COUNT_OF(auth_cmd));
    bit_buffer_copy_bytes(tx_buffer, auth_cmd, COUNT_OF(auth_cmd));
    BitBuffer* rx_buffer = bit_buffer_alloc(sizeof(MfClassicNt));

    if(iso14443_3a_event->type == Iso14443_3aPollerEventTypeReady) {
        Iso14443_3aError error = iso14443_3a_poller_send_standard_frame(
            iso3_poller, tx_buffer, rx_buffer, MF_CLASSIC_FWT_FC);
        if(error == Iso14443_3aErrorWrongCrc) {
            if(bit_buffer_get_size_bytes(rx_buffer) == sizeof(MfClassicNt)) {
                detected = true;
            }
        }
    }

    bit_buffer_free(tx_buffer);
    bit_buffer_free(rx_buffer);

    return detected;
}

void mf_classic_poller_set_callback(
    MfClassicPoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

const MfClassicData* mf_classic_poller_get_data(const MfClassicPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

const NfcPollerBase mf_classic_poller = {
    .alloc = (NfcPollerAlloc)mf_classic_poller_alloc,
    .free = (NfcPollerFree)mf_classic_poller_free,
    .set_callback = (NfcPollerSetCallback)mf_classic_poller_set_callback,
    .run = (NfcPollerRun)mf_classic_poller_run,
    .detect = (NfcPollerDetect)mf_classic_poller_detect,
    .get_data = (NfcPollerGetData)mf_classic_poller_get_data,
};
