#include "mf_classic_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "MfClassicPoller"

// TODO FL-3926: Buffer writes for Hardnested, set state to Log when finished and sum property matches
// TODO FL-3926: Store target key in CUID dictionary
// TODO FL-3926: Dead code for malloc returning NULL?
// TODO FL-3926: Auth1 static encrypted exists (rare)
// TODO FL-3926: Use keys found by NFC plugins, cached keys

#define MF_CLASSIC_MAX_BUFF_SIZE (64)

// Ordered by frequency, labeled chronologically
const MfClassicBackdoorKeyPair mf_classic_backdoor_keys[] = {
    {{{0xa3, 0x96, 0xef, 0xa4, 0xe2, 0x4f}}, MfClassicBackdoorAuth3}, // Fudan (static encrypted)
    {{{0xa3, 0x16, 0x67, 0xa8, 0xce, 0xc1}}, MfClassicBackdoorAuth1}, // Fudan, Infineon, NXP
    {{{0x51, 0x8b, 0x33, 0x54, 0xe7, 0x60}}, MfClassicBackdoorAuth2}, // Fudan
};
const size_t mf_classic_backdoor_keys_count = COUNT_OF(mf_classic_backdoor_keys);
const uint16_t valid_sums[] =
    {0, 32, 56, 64, 80, 96, 104, 112, 120, 128, 136, 144, 152, 160, 176, 192, 200, 224, 256};

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

    // Clean up resources in MfClassicPollerDictAttackContext
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    // Free the dictionaries
    if(dict_attack_ctx->mf_classic_system_dict) {
        keys_dict_free(dict_attack_ctx->mf_classic_system_dict);
        dict_attack_ctx->mf_classic_system_dict = NULL;
    }
    if(dict_attack_ctx->mf_classic_user_dict) {
        keys_dict_free(dict_attack_ctx->mf_classic_user_dict);
        dict_attack_ctx->mf_classic_user_dict = NULL;
    }

    // Free the nested nonce array if it exists
    if(dict_attack_ctx->nested_nonce.nonces) {
        free(dict_attack_ctx->nested_nonce.nonces);
        dict_attack_ctx->nested_nonce.nonces = NULL;
        dict_attack_ctx->nested_nonce.count = 0;
    }

    free(instance);
}

static NfcCommand mf_classic_poller_handle_data_update(MfClassicPoller* instance) {
    MfClassicPollerEventDataUpdate* data_update = &instance->mfc_event_data.data_update;

    mf_classic_get_read_sectors_and_keys(
        instance->data, &data_update->sectors_read, &data_update->keys_found);
    data_update->current_sector = instance->mode_ctx.dict_attack_ctx.current_sector;
    data_update->nested_phase = instance->mode_ctx.dict_attack_ctx.nested_phase;
    data_update->prng_type = instance->mode_ctx.dict_attack_ctx.prng_type;
    data_update->backdoor = instance->mode_ctx.dict_attack_ctx.backdoor;
    data_update->nested_target_key = instance->mode_ctx.dict_attack_ctx.nested_target_key;
    data_update->msb_count = instance->mode_ctx.dict_attack_ctx.msb_count;
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
        uint64_t key_b = bit_lib_bytes_to_num_be(sec_tr->key_b.data, sizeof(MfClassicKey));
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
        MfClassicError error =
            mf_classic_poller_get_nt(instance, 254, MfClassicKeyTypeA, NULL, false);
        if(error == MfClassicErrorNone) {
            instance->data->type = MfClassicType4k;
            instance->state = MfClassicPollerStateStart;
            instance->current_type_check = MfClassicType4k;
            FURI_LOG_D(TAG, "4K detected");
        } else {
            instance->current_type_check = MfClassicType1k;
        }
    } else if(instance->current_type_check == MfClassicType1k) {
        MfClassicError error =
            mf_classic_poller_get_nt(instance, 62, MfClassicKeyTypeA, NULL, false);
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

    if(instance->mfc_event_data.poller_mode.mode == MfClassicPollerModeDictAttackStandard) {
        mf_classic_copy(instance->data, instance->mfc_event_data.poller_mode.data);
        instance->state = MfClassicPollerStateRequestKey;
    } else if(instance->mfc_event_data.poller_mode.mode == MfClassicPollerModeDictAttackEnhanced) {
        mf_classic_copy(instance->data, instance->mfc_event_data.poller_mode.data);
        instance->state = MfClassicPollerStateAnalyzeBackdoor;
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
            instance, write_ctx->current_block, auth_key, write_ctx->key_type_read, NULL, false);
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
                instance,
                write_ctx->current_block,
                auth_key,
                write_ctx->key_type_write,
                NULL,
                false);
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

        MfClassicError error = mf_classic_poller_auth(
            instance, write_ctx->current_block, key, auth_key_type, NULL, false);
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
            uint64_t key = bit_lib_bytes_to_num_be(sec_read_ctx->key.data, sizeof(MfClassicKey));
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
                NULL,
                false);
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

NfcCommand mf_classic_poller_handler_analyze_backdoor(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandReset;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;
    instance->mode_ctx.dict_attack_ctx.enhanced_dict = true;

    size_t current_key_index =
        mf_classic_backdoor_keys_count - 1; // Default to the last valid index

    // Find the current key in the backdoor_keys array
    for(size_t i = 0; i < mf_classic_backdoor_keys_count; i++) {
        if(memcmp(
               dict_attack_ctx->current_key.data,
               mf_classic_backdoor_keys[i].key.data,
               sizeof(MfClassicKey)) == 0) {
            current_key_index = i;
            break;
        }
    }

    // Choose the next key to try
    size_t next_key_index = (current_key_index + 1) % mf_classic_backdoor_keys_count;
    uint8_t backdoor_version = mf_classic_backdoor_keys[next_key_index].type - 1;

    FURI_LOG_D(TAG, "Trying backdoor v%d", backdoor_version);
    dict_attack_ctx->current_key = mf_classic_backdoor_keys[next_key_index].key;

    // Attempt backdoor authentication
    MfClassicError error = mf_classic_poller_auth(
        instance, 0, &dict_attack_ctx->current_key, MfClassicKeyTypeA, NULL, true);
    if((next_key_index == 0) &&
       (error == MfClassicErrorProtocol || error == MfClassicErrorTimeout)) {
        FURI_LOG_D(TAG, "No backdoor identified");
        dict_attack_ctx->backdoor = MfClassicBackdoorNone;
        instance->state = MfClassicPollerStateRequestKey;
    } else if(error == MfClassicErrorNone) {
        FURI_LOG_I(TAG, "Backdoor identified: v%d", backdoor_version);
        dict_attack_ctx->backdoor = mf_classic_backdoor_keys[next_key_index].type;
        instance->state = MfClassicPollerStateBackdoorReadSector;
    } else if(
        (error == MfClassicErrorAuth) &&
        (next_key_index == (mf_classic_backdoor_keys_count - 1))) {
        // We've tried all backdoor keys, this is a unique key and an important research finding
        furi_crash("New backdoor: please report!");
    }

    return command;
}

NfcCommand mf_classic_poller_handler_backdoor_read_sector(MfClassicPoller* instance) {
    // TODO FL-3926: Reauth not needed
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;
    MfClassicError error = MfClassicErrorNone;
    MfClassicBlock block = {};

    uint8_t current_sector = mf_classic_get_sector_by_block(dict_attack_ctx->current_block);
    uint8_t blocks_in_sector = mf_classic_get_blocks_num_in_sector(current_sector);
    uint8_t first_block_in_sector = mf_classic_get_first_block_num_of_sector(current_sector);

    do {
        if(dict_attack_ctx->current_block >= instance->sectors_total * 4) {
            // We've read all blocks, reset current_block and move to next state
            dict_attack_ctx->current_block = 0;
            instance->state = MfClassicPollerStateNestedController;
            break;
        }

        // Authenticate with the backdoor key
        error = mf_classic_poller_auth(
            instance,
            first_block_in_sector, // Authenticate to the first block of the sector
            &(dict_attack_ctx->current_key),
            MfClassicKeyTypeA,
            NULL,
            true);

        if(error != MfClassicErrorNone) {
            FURI_LOG_E(
                TAG, "Failed to authenticate with backdoor key for sector %d", current_sector);
            break;
        }

        // Read all blocks in the sector
        for(uint8_t block_in_sector = 0; block_in_sector < blocks_in_sector; block_in_sector++) {
            uint8_t block_to_read = first_block_in_sector + block_in_sector;

            error = mf_classic_poller_read_block(instance, block_to_read, &block);

            if(error != MfClassicErrorNone) {
                FURI_LOG_E(TAG, "Failed to read block %d", block_to_read);
                break;
            }

            // Set the block as read in the data structure
            mf_classic_set_block_read(instance->data, block_to_read, &block);
        }

        if(error != MfClassicErrorNone) {
            break;
        }

        // Move to the next sector
        current_sector++;
        dict_attack_ctx->current_block = mf_classic_get_first_block_num_of_sector(current_sector);

        // Update blocks_in_sector and first_block_in_sector for the next sector
        if(current_sector < instance->sectors_total) {
            blocks_in_sector = mf_classic_get_blocks_num_in_sector(current_sector);
            first_block_in_sector = mf_classic_get_first_block_num_of_sector(current_sector);
        }

        // Halt the card after each sector to reset the authentication state
        mf_classic_poller_halt(instance);

        // Send an event to the app that a sector has been read
        command = mf_classic_poller_handle_data_update(instance);

    } while(false);

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
        uint64_t key =
            bit_lib_bytes_to_num_be(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Auth to block %d with key A: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeA, NULL, false);
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
        uint64_t key =
            bit_lib_bytes_to_num_be(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Auth to block %d with key B: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeB, NULL, false);
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
                NULL,
                false);
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

    do {
        if(dict_attack_ctx->current_key_type == MfClassicKeyTypeA) {
            dict_attack_ctx->current_key_type = MfClassicKeyTypeB;
            instance->state = MfClassicPollerStateKeyReuseAuthKeyB;
        } else {
            dict_attack_ctx->reuse_key_sector++;
            if(dict_attack_ctx->reuse_key_sector == instance->sectors_total) {
                instance->mfc_event.type = MfClassicPollerEventTypeKeyAttackStop;
                command = instance->callback(instance->general_event, instance->context);
                // Nested entrypoint
                bool nested_active = dict_attack_ctx->nested_phase != MfClassicNestedPhaseNone;
                if((dict_attack_ctx->enhanced_dict) &&
                   ((nested_active &&
                     (dict_attack_ctx->nested_phase != MfClassicNestedPhaseFinished)) ||
                    (!(nested_active) && !(mf_classic_is_card_read(instance->data))))) {
                    instance->state = MfClassicPollerStateNestedController;
                    break;
                }
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
    } while(false);

    return command;
}

NfcCommand mf_classic_poller_handler_key_reuse_start_no_offset(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    instance->mfc_event.type = MfClassicPollerEventTypeKeyAttackStart;
    instance->mfc_event_data.key_attack_data.current_sector = dict_attack_ctx->reuse_key_sector;
    command = instance->callback(instance->general_event, instance->context);
    if(dict_attack_ctx->current_key_type == MfClassicKeyTypeA) {
        instance->state = MfClassicPollerStateKeyReuseAuthKeyA;
    } else {
        instance->state = MfClassicPollerStateKeyReuseAuthKeyB;
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
        uint64_t key =
            bit_lib_bytes_to_num_be(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Key attack auth to block %d with key A: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeA, NULL, false);
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
        uint64_t key =
            bit_lib_bytes_to_num_be(dict_attack_ctx->current_key.data, sizeof(MfClassicKey));
        FURI_LOG_D(TAG, "Key attack auth to block %d with key B: %06llx", block, key);

        MfClassicError error = mf_classic_poller_auth(
            instance, block, &dict_attack_ctx->current_key, MfClassicKeyTypeB, NULL, false);
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
                NULL,
                false);
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

// Helper function to add a nonce to the array
static bool add_nested_nonce(
    MfClassicNestedNonceArray* array,
    uint32_t cuid,
    uint16_t key_idx,
    uint32_t nt,
    uint32_t nt_enc,
    uint8_t par,
    uint16_t dist) {
    MfClassicNestedNonce* new_nonces;
    if(array->count == 0) {
        new_nonces = malloc(sizeof(MfClassicNestedNonce));
    } else {
        new_nonces = realloc(array->nonces, (array->count + 1) * sizeof(MfClassicNestedNonce));
    }
    if(new_nonces == NULL) return false;

    array->nonces = new_nonces;
    array->nonces[array->count].cuid = cuid;
    array->nonces[array->count].key_idx = key_idx;
    array->nonces[array->count].nt = nt;
    array->nonces[array->count].nt_enc = nt_enc;
    array->nonces[array->count].par = par;
    array->nonces[array->count].dist = dist;
    array->count++;
    return true;
}

NfcCommand mf_classic_poller_handler_nested_analyze_prng(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;
    uint8_t hard_nt_count = 0;

    for(uint8_t i = 0; i < dict_attack_ctx->nested_nonce.count; i++) {
        MfClassicNestedNonce* nonce = &dict_attack_ctx->nested_nonce.nonces[i];
        if(!crypto1_is_weak_prng_nonce(nonce->nt)) hard_nt_count++;
    }

    if(hard_nt_count >= MF_CLASSIC_NESTED_NT_HARD_MINIMUM) {
        dict_attack_ctx->prng_type = MfClassicPrngTypeHard;
        FURI_LOG_D(TAG, "Detected Hard PRNG");
    } else {
        dict_attack_ctx->prng_type = MfClassicPrngTypeWeak;
        FURI_LOG_D(TAG, "Detected Weak PRNG");
    }

    instance->state = MfClassicPollerStateNestedController;
    return command;
}

NfcCommand mf_classic_poller_handler_nested_collect_nt(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandReset;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    MfClassicNt nt = {};
    MfClassicError error = mf_classic_poller_get_nt(instance, 0, MfClassicKeyTypeA, &nt, false);

    if(error != MfClassicErrorNone) {
        dict_attack_ctx->prng_type = MfClassicPrngTypeNoTag;
        FURI_LOG_E(TAG, "Failed to collect nt");
    } else {
        FURI_LOG_T(TAG, "nt: %02x%02x%02x%02x", nt.data[0], nt.data[1], nt.data[2], nt.data[3]);
        uint32_t nt_data = bit_lib_bytes_to_num_be(nt.data, sizeof(MfClassicNt));
        if(!add_nested_nonce(
               &dict_attack_ctx->nested_nonce,
               iso14443_3a_get_cuid(instance->data->iso14443_3a_data),
               0,
               nt_data,
               0,
               0,
               0)) {
            dict_attack_ctx->prng_type = MfClassicPrngTypeNoTag;
        }
    }

    instance->state = MfClassicPollerStateNestedController;
    return command;
}

NfcCommand mf_classic_poller_handler_nested_calibrate(MfClassicPoller* instance) {
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;
    uint32_t nt_enc_temp_arr[MF_CLASSIC_NESTED_CALIBRATION_COUNT];
    uint16_t distances[MF_CLASSIC_NESTED_CALIBRATION_COUNT - 1] = {0};

    dict_attack_ctx->d_min = UINT16_MAX;
    dict_attack_ctx->d_max = 0;
    uint8_t block =
        mf_classic_get_first_block_num_of_sector(dict_attack_ctx->nested_known_key_sector);
    uint32_t cuid = iso14443_3a_get_cuid(instance->data->iso14443_3a_data);

    MfClassicAuthContext auth_ctx = {};
    MfClassicError error;

    uint32_t nt_prev = 0;
    uint32_t nt_enc_prev = 0;
    uint32_t same_nt_enc_cnt = 0;
    uint8_t nt_enc_collected = 0;
    bool use_backdoor = (dict_attack_ctx->backdoor != MfClassicBackdoorNone);

    // Step 1: Perform full authentication once
    error = mf_classic_poller_auth(
        instance,
        block,
        &dict_attack_ctx->nested_known_key,
        dict_attack_ctx->nested_known_key_type,
        &auth_ctx,
        use_backdoor);

    if(error != MfClassicErrorNone) {
        FURI_LOG_E(TAG, "Failed to perform full authentication");
        instance->state = MfClassicPollerStateNestedCalibrate;
        return command;
    }

    FURI_LOG_D(TAG, "Full authentication successful");

    nt_prev = bit_lib_bytes_to_num_be(auth_ctx.nt.data, sizeof(MfClassicNt));

    if((dict_attack_ctx->static_encrypted) &&
       (dict_attack_ctx->backdoor == MfClassicBackdoorAuth3)) {
        command = NfcCommandReset;
        uint8_t target_block =
            mf_classic_get_first_block_num_of_sector(dict_attack_ctx->nested_target_key / 4);
        MfClassicKeyType target_key_type =
            ((dict_attack_ctx->nested_target_key % 4) < 2) ? MfClassicKeyTypeA : MfClassicKeyTypeB;
        error = mf_classic_poller_auth_nested(
            instance,
            target_block,
            &dict_attack_ctx->nested_known_key,
            target_key_type,
            &auth_ctx,
            use_backdoor,
            false);

        if(error != MfClassicErrorNone) {
            FURI_LOG_E(TAG, "Failed to perform nested authentication for static encrypted tag");
            instance->state = MfClassicPollerStateNestedCalibrate;
            return command;
        }

        uint32_t nt_enc = bit_lib_bytes_to_num_be(auth_ctx.nt.data, sizeof(MfClassicNt));
        // Store the decrypted static encrypted nonce
        dict_attack_ctx->static_encrypted_nonce =
            crypto1_decrypt_nt_enc(cuid, nt_enc, dict_attack_ctx->nested_known_key);

        dict_attack_ctx->calibrated = true;

        FURI_LOG_D(TAG, "Static encrypted tag calibrated. Decrypted nonce: %08lx", nt_enc);

        instance->state = MfClassicPollerStateNestedController;
        return command;
    }

    // Original calibration logic for non-static encrypted tags
    // Step 2: Perform nested authentication multiple times
    for(uint8_t collection_cycle = 0; collection_cycle < MF_CLASSIC_NESTED_CALIBRATION_COUNT;
        collection_cycle++) {
        error = mf_classic_poller_auth_nested(
            instance,
            block,
            &dict_attack_ctx->nested_known_key,
            dict_attack_ctx->nested_known_key_type,
            &auth_ctx,
            use_backdoor,
            false);

        if(error != MfClassicErrorNone) {
            FURI_LOG_E(TAG, "Failed to perform nested authentication %u", collection_cycle);
            continue;
        }

        nt_enc_temp_arr[collection_cycle] =
            bit_lib_bytes_to_num_be(auth_ctx.nt.data, sizeof(MfClassicNt));
        nt_enc_collected++;
    }

    for(int i = 0; i < nt_enc_collected; i++) {
        if(nt_enc_temp_arr[i] == nt_enc_prev) {
            same_nt_enc_cnt++;
            if(same_nt_enc_cnt > 3) {
                dict_attack_ctx->static_encrypted = true;
                break;
            }
        } else {
            same_nt_enc_cnt = 0;
            nt_enc_prev = nt_enc_temp_arr[i];
        }
    }

    if(dict_attack_ctx->static_encrypted) {
        FURI_LOG_D(TAG, "Static encrypted nonce detected");
        dict_attack_ctx->calibrated = true;
        instance->state = MfClassicPollerStateNestedController;
        return command;
    }

    // Find the distance between each nonce
    FURI_LOG_D(TAG, "Calculating distance between nonces");
    uint64_t known_key = bit_lib_bytes_to_num_be(dict_attack_ctx->nested_known_key.data, 6);
    uint8_t valid_distances = 0;
    for(uint32_t collection_cycle = 1; collection_cycle < MF_CLASSIC_NESTED_CALIBRATION_COUNT;
        collection_cycle++) {
        bool found = false;
        uint32_t decrypted_nt_enc = crypto1_decrypt_nt_enc(
            cuid, nt_enc_temp_arr[collection_cycle], dict_attack_ctx->nested_known_key);
        for(int i = 0; i < 65535; i++) {
            uint32_t nth_successor = crypto1_prng_successor(nt_prev, i);
            if(nth_successor == decrypted_nt_enc) {
                FURI_LOG_D(TAG, "nt_enc (plain) %08lx", nth_successor);
                FURI_LOG_D(TAG, "dist from nt prev: %i", i);
                distances[valid_distances++] = i;
                nt_prev = nth_successor;
                found = true;
                break;
            }
        }
        if(!found) {
            FURI_LOG_E(
                TAG,
                "Failed to find distance for nt_enc %08lx",
                nt_enc_temp_arr[collection_cycle]);
            FURI_LOG_E(
                TAG, "using key %06llx and uid %08lx, nt_prev is %08lx", known_key, cuid, nt_prev);
        }
    }

    // Calculate median and standard deviation
    if(valid_distances > 0) {
        // Sort the distances array (bubble sort)
        for(uint8_t i = 0; i < valid_distances - 1; i++) {
            for(uint8_t j = 0; j < valid_distances - i - 1; j++) {
                if(distances[j] > distances[j + 1]) {
                    uint16_t temp = distances[j];
                    distances[j] = distances[j + 1];
                    distances[j + 1] = temp;
                }
            }
        }

        // Calculate median
        uint16_t median =
            (valid_distances % 2 == 0) ?
                (distances[valid_distances / 2 - 1] + distances[valid_distances / 2]) / 2 :
                distances[valid_distances / 2];

        // Calculate standard deviation
        float sum = 0, sum_sq = 0;
        for(uint8_t i = 0; i < valid_distances; i++) {
            sum += distances[i];
            sum_sq += (float)distances[i] * distances[i];
        }
        float mean = sum / valid_distances;
        float variance = (sum_sq / valid_distances) - (mean * mean);
        float std_dev = sqrtf(variance);

        // Filter out values over 3 standard deviations away from the median
        for(uint8_t i = 0; i < valid_distances; i++) {
            if(fabsf((float)distances[i] - median) <= 3 * std_dev) {
                if(distances[i] < dict_attack_ctx->d_min) dict_attack_ctx->d_min = distances[i];
                if(distances[i] > dict_attack_ctx->d_max) dict_attack_ctx->d_max = distances[i];
            }
        }

        // Some breathing room
        dict_attack_ctx->d_min = (dict_attack_ctx->d_min > 3) ? dict_attack_ctx->d_min - 3 : 0;
        dict_attack_ctx->d_max += 3;
    }

    furi_assert(dict_attack_ctx->d_min <= dict_attack_ctx->d_max);
    dict_attack_ctx->calibrated = true;
    instance->state = MfClassicPollerStateNestedController;

    mf_classic_poller_halt(instance);
    uint16_t d_dist = dict_attack_ctx->d_max - dict_attack_ctx->d_min;
    FURI_LOG_D(
        TAG,
        "Calibration completed: min=%u max=%u static=%s",
        dict_attack_ctx->d_min,
        dict_attack_ctx->d_max,
        ((d_dist >= 3) && (d_dist <= 6)) ? "true" : "false");

    return command;
}

static inline void set_byte_found(uint8_t* found, uint8_t byte) {
    SET_PACKED_BIT(found, byte);
}

static inline bool is_byte_found(uint8_t* found, uint8_t byte) {
    return GET_PACKED_BIT(found, byte) != 0;
}

NfcCommand mf_classic_poller_handler_nested_collect_nt_enc(MfClassicPoller* instance) {
    // TODO FL-3926: Handle when nonce is not collected (retry counter? Do not increment nested_target_key)
    // TODO FL-3926: Look into using MfClassicNt more
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    do {
        uint8_t block =
            mf_classic_get_first_block_num_of_sector(dict_attack_ctx->nested_known_key_sector);
        uint32_t cuid = iso14443_3a_get_cuid(instance->data->iso14443_3a_data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error;

        bool use_backdoor = (dict_attack_ctx->backdoor != MfClassicBackdoorNone);
        bool is_weak = dict_attack_ctx->prng_type == MfClassicPrngTypeWeak;
        uint8_t nonce_pair_index = is_weak ? (dict_attack_ctx->nested_target_key % 2) : 0;
        uint8_t nt_enc_per_collection =
            (is_weak && !(dict_attack_ctx->static_encrypted)) ?
                ((dict_attack_ctx->attempt_count + 2) + nonce_pair_index) :
                1;
        uint8_t target_sector = dict_attack_ctx->nested_target_key / (is_weak ? 4 : 2);
        MfClassicKeyType target_key_type =
            (dict_attack_ctx->nested_target_key % (is_weak ? 4 : 2) < (is_weak ? 2 : 1)) ?
                MfClassicKeyTypeA :
                MfClassicKeyTypeB;
        uint8_t target_block = mf_classic_get_sector_trailer_num_by_sector(target_sector);
        uint32_t nt_enc_temp_arr[nt_enc_per_collection];
        uint8_t nt_enc_collected = 0;
        uint8_t parity = 0;

        // Step 1: Perform full authentication once
        error = mf_classic_poller_auth(
            instance,
            block,
            &dict_attack_ctx->nested_known_key,
            dict_attack_ctx->nested_known_key_type,
            &auth_ctx,
            use_backdoor);

        if(error != MfClassicErrorNone) {
            FURI_LOG_E(TAG, "Failed to perform full authentication");
            break;
        }

        FURI_LOG_D(TAG, "Full authentication successful");

        // Step 2: Perform nested authentication a variable number of times to get nt_enc at a different PRNG offset
        // eg. Collect most commonly observed nonce from 3 auths to known sector and 4th to target, then separately the
        //     most commonly observed nonce from 4 auths to known sector and 5th to target. This gets us a nonce pair,
        //     at a known distance (confirmed by parity bits) telling us the nt_enc plain.
        for(uint8_t collection_cycle = 0; collection_cycle < (nt_enc_per_collection - 1);
            collection_cycle++) {
            // This loop must match the calibrated loop
            error = mf_classic_poller_auth_nested(
                instance,
                block,
                &dict_attack_ctx->nested_known_key,
                dict_attack_ctx->nested_known_key_type,
                &auth_ctx,
                use_backdoor,
                false);

            if(error != MfClassicErrorNone) {
                FURI_LOG_E(TAG, "Failed to perform nested authentication %u", collection_cycle);
                break;
            }

            nt_enc_temp_arr[collection_cycle] =
                bit_lib_bytes_to_num_be(auth_ctx.nt.data, sizeof(MfClassicNt));
            nt_enc_collected++;
        }
        error = mf_classic_poller_auth_nested(
            instance,
            target_block,
            &dict_attack_ctx->nested_known_key,
            target_key_type,
            &auth_ctx,
            false,
            true);

        if(nt_enc_collected != (nt_enc_per_collection - 1)) {
            FURI_LOG_E(TAG, "Failed to collect sufficient nt_enc values");
            break;
        }

        uint32_t nt_enc = bit_lib_bytes_to_num_be(auth_ctx.nt.data, sizeof(MfClassicNt));
        // Collect parity bits
        const uint8_t* parity_data = bit_buffer_get_parity(instance->rx_plain_buffer);
        for(int i = 0; i < 4; i++) {
            parity = (parity << 1) | (((parity_data[0] >> i) & 0x01) ^ 0x01);
        }

        uint32_t nt_prev = 0, decrypted_nt_prev = 0, found_nt = 0;
        uint16_t dist = 0;
        if(is_weak && !(dict_attack_ctx->static_encrypted)) {
            // Ensure this isn't the same nonce as the previous collection
            if((dict_attack_ctx->nested_nonce.count == 1) &&
               (dict_attack_ctx->nested_nonce.nonces[0].nt_enc == nt_enc)) {
                FURI_LOG_D(TAG, "Duplicate nonce, dismissing collection attempt");
                break;
            }

            // Decrypt the previous nonce
            nt_prev = nt_enc_temp_arr[nt_enc_collected - 1];
            decrypted_nt_prev =
                crypto1_decrypt_nt_enc(cuid, nt_prev, dict_attack_ctx->nested_known_key);

            // Find matching nt_enc plain at expected distance
            found_nt = 0;
            uint8_t found_nt_cnt = 0;
            uint16_t current_dist = dict_attack_ctx->d_min;
            while(current_dist <= dict_attack_ctx->d_max) {
                uint32_t nth_successor = crypto1_prng_successor(decrypted_nt_prev, current_dist);
                if(crypto1_nonce_matches_encrypted_parity_bits(
                       nth_successor, nth_successor ^ nt_enc, parity)) {
                    found_nt_cnt++;
                    if(found_nt_cnt > 1) {
                        FURI_LOG_D(TAG, "Ambiguous nonce, dismissing collection attempt");
                        break;
                    }
                    found_nt = nth_successor;
                }
                current_dist++;
            }
            if(found_nt_cnt != 1) {
                break;
            }
        } else if(dict_attack_ctx->static_encrypted) {
            if(dict_attack_ctx->backdoor == MfClassicBackdoorAuth3) {
                found_nt = dict_attack_ctx->static_encrypted_nonce;
            } else {
                dist = UINT16_MAX;
            }
        } else {
            // Hardnested
            if(!is_byte_found(dict_attack_ctx->nt_enc_msb, (nt_enc >> 24) & 0xFF)) {
                set_byte_found(dict_attack_ctx->nt_enc_msb, (nt_enc >> 24) & 0xFF);
                dict_attack_ctx->msb_count++;
                // Add unique parity to sum
                dict_attack_ctx->msb_par_sum += nfc_util_even_parity32(parity & 0x08);
            }
            parity ^= 0x0F;
        }

        // Add the nonce to the array
        if(add_nested_nonce(
               &dict_attack_ctx->nested_nonce,
               cuid,
               dict_attack_ctx->nested_target_key,
               found_nt,
               nt_enc,
               parity,
               dist)) {
            dict_attack_ctx->auth_passed = true;
        } else {
            FURI_LOG_E(TAG, "Failed to add nested nonce to array. OOM?");
        }

        FURI_LOG_D(
            TAG,
            "Target: %u (nonce pair %u, key type %s, block %u)",
            dict_attack_ctx->nested_target_key,
            nonce_pair_index,
            (target_key_type == MfClassicKeyTypeA) ? "A" : "B",
            target_block);
        FURI_LOG_T(TAG, "cuid: %08lx", cuid);
        FURI_LOG_T(TAG, "nt_enc: %08lx", nt_enc);
        FURI_LOG_T(
            TAG,
            "parity: %u%u%u%u",
            ((parity >> 3) & 1),
            ((parity >> 2) & 1),
            ((parity >> 1) & 1),
            (parity & 1));
        FURI_LOG_T(TAG, "nt_enc prev: %08lx", nt_prev);
        FURI_LOG_T(TAG, "nt_enc prev decrypted: %08lx", decrypted_nt_prev);
    } while(false);

    instance->state = MfClassicPollerStateNestedController;

    mf_classic_poller_halt(instance);
    return command;
}

static MfClassicKey* search_dicts_for_nonce_key(
    MfClassicPollerDictAttackContext* dict_attack_ctx,
    MfClassicNestedNonceArray* nonce_array,
    KeysDict* system_dict,
    KeysDict* user_dict,
    bool is_weak) {
    MfClassicKey stack_key;
    KeysDict* dicts[] = {user_dict, system_dict};
    bool is_resumed = dict_attack_ctx->nested_phase == MfClassicNestedPhaseDictAttackResume;
    bool found_resume_point = false;

    for(int i = 0; i < 2; i++) {
        if(!dicts[i]) continue;
        keys_dict_rewind(dicts[i]);
        while(keys_dict_get_next_key(dicts[i], stack_key.data, sizeof(MfClassicKey))) {
            if(is_resumed && !found_resume_point) {
                found_resume_point =
                    (memcmp(
                         dict_attack_ctx->current_key.data,
                         stack_key.data,
                         sizeof(MfClassicKey)) == 0);
                continue;
            }
            bool full_match = true;
            for(uint8_t j = 0; j < nonce_array->count; j++) {
                // Verify nonce matches encrypted parity bits for all nonces
                uint32_t nt_enc_plain = crypto1_decrypt_nt_enc(
                    nonce_array->nonces[j].cuid, nonce_array->nonces[j].nt_enc, stack_key);
                if(is_weak) {
                    full_match &= crypto1_is_weak_prng_nonce(nt_enc_plain);
                    if(!full_match) break;
                }
                full_match &= crypto1_nonce_matches_encrypted_parity_bits(
                    nt_enc_plain,
                    nt_enc_plain ^ nonce_array->nonces[j].nt_enc,
                    nonce_array->nonces[j].par);
                if(!full_match) break;
            }
            if(full_match) {
                MfClassicKey* new_candidate = malloc(sizeof(MfClassicKey));
                if(new_candidate == NULL) return NULL; // malloc failed
                memcpy(new_candidate, &stack_key, sizeof(MfClassicKey));
                return new_candidate;
            }
        }
    }

    return NULL;
}

NfcCommand mf_classic_poller_handler_nested_dict_attack(MfClassicPoller* instance) {
    // TODO FL-3926: Handle when nonce is not collected (retry counter? Do not increment nested_target_key)
    // TODO FL-3926: Look into using MfClassicNt more
    NfcCommand command = NfcCommandContinue;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;

    do {
        uint8_t block =
            mf_classic_get_first_block_num_of_sector(dict_attack_ctx->nested_known_key_sector);
        uint32_t cuid = iso14443_3a_get_cuid(instance->data->iso14443_3a_data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error;

        bool use_backdoor_for_initial_auth = (dict_attack_ctx->backdoor != MfClassicBackdoorNone);
        bool is_weak = dict_attack_ctx->prng_type == MfClassicPrngTypeWeak;
        bool is_last_iter_for_hard_key =
            ((!is_weak) && ((dict_attack_ctx->nested_target_key % 8) == 7));
        MfClassicKeyType target_key_type =
            (((is_weak) && ((dict_attack_ctx->nested_target_key % 2) == 0)) ||
             ((!is_weak) && ((dict_attack_ctx->nested_target_key % 16) < 8))) ?
                MfClassicKeyTypeA :
                MfClassicKeyTypeB;
        uint8_t target_sector = dict_attack_ctx->nested_target_key / (is_weak ? 2 : 16);
        uint8_t target_block = mf_classic_get_sector_trailer_num_by_sector(target_sector);
        uint8_t parity = 0;

        if(((is_weak) && (dict_attack_ctx->nested_nonce.count == 0)) ||
           ((!is_weak) && (dict_attack_ctx->nested_nonce.count < 8))) {
            // Step 1: Perform full authentication once
            error = mf_classic_poller_auth(
                instance,
                block,
                &dict_attack_ctx->nested_known_key,
                dict_attack_ctx->nested_known_key_type,
                &auth_ctx,
                use_backdoor_for_initial_auth);

            if(error != MfClassicErrorNone) {
                FURI_LOG_E(TAG, "Failed to perform full authentication");
                dict_attack_ctx->auth_passed = false;
                break;
            }

            FURI_LOG_D(TAG, "Full authentication successful");

            // Step 2: Collect nested nt and parity
            error = mf_classic_poller_auth_nested(
                instance,
                target_block,
                &dict_attack_ctx->nested_known_key,
                target_key_type,
                &auth_ctx,
                false,
                true);

            if(error != MfClassicErrorNone) {
                FURI_LOG_E(TAG, "Failed to perform nested authentication");
                dict_attack_ctx->auth_passed = false;
                break;
            }

            uint32_t nt_enc = bit_lib_bytes_to_num_be(auth_ctx.nt.data, sizeof(MfClassicNt));
            // Collect parity bits
            const uint8_t* parity_data = bit_buffer_get_parity(instance->rx_plain_buffer);
            for(int i = 0; i < 4; i++) {
                parity = (parity << 1) | (((parity_data[0] >> i) & 0x01) ^ 0x01);
            }

            bool success = add_nested_nonce(
                &dict_attack_ctx->nested_nonce,
                cuid,
                dict_attack_ctx->nested_target_key,
                0,
                nt_enc,
                parity,
                0);
            if(!success) {
                FURI_LOG_E(TAG, "Failed to add nested nonce to array. OOM?");
                dict_attack_ctx->auth_passed = false;
                break;
            }

            dict_attack_ctx->auth_passed = true;
        }
        // If we have sufficient nonces, search the dictionaries for the key
        if((is_weak && (dict_attack_ctx->nested_nonce.count == 1)) ||
           (is_last_iter_for_hard_key && (dict_attack_ctx->nested_nonce.count == 8))) {
            // Identify key candidates
            MfClassicKey* key_candidate = search_dicts_for_nonce_key(
                dict_attack_ctx,
                &dict_attack_ctx->nested_nonce,
                dict_attack_ctx->mf_classic_system_dict,
                dict_attack_ctx->mf_classic_user_dict,
                is_weak);
            if(key_candidate != NULL) {
                FURI_LOG_I(
                    TAG,
                    "Found key candidate %06llx",
                    bit_lib_bytes_to_num_be(key_candidate->data, sizeof(MfClassicKey)));
                dict_attack_ctx->current_key = *key_candidate;
                dict_attack_ctx->reuse_key_sector = target_sector;
                dict_attack_ctx->current_key_type = target_key_type;
                free(key_candidate);
                break;
            } else {
                free(dict_attack_ctx->nested_nonce.nonces);
                dict_attack_ctx->nested_nonce.nonces = NULL;
                dict_attack_ctx->nested_nonce.count = 0;
            }
        }

        FURI_LOG_D(
            TAG,
            "Target: %u (key type %s, block %u) cuid: %08lx",
            dict_attack_ctx->nested_target_key,
            (target_key_type == MfClassicKeyTypeA) ? "A" : "B",
            target_block,
            cuid);
    } while(false);

    instance->state = MfClassicPollerStateNestedController;

    mf_classic_poller_halt(instance);
    return command;
}

NfcCommand mf_classic_poller_handler_nested_log(MfClassicPoller* instance) {
    furi_assert(instance->mode_ctx.dict_attack_ctx.nested_nonce.count > 0);
    furi_assert(instance->mode_ctx.dict_attack_ctx.nested_nonce.nonces);

    NfcCommand command = NfcCommandContinue;
    bool params_saved = false;
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = buffered_file_stream_alloc(storage);
    FuriString* temp_str = furi_string_alloc();
    bool weak_prng = dict_attack_ctx->prng_type == MfClassicPrngTypeWeak;
    bool static_encrypted = dict_attack_ctx->static_encrypted;

    do {
        if(weak_prng && (!(static_encrypted)) && (dict_attack_ctx->nested_nonce.count != 2)) {
            FURI_LOG_E(
                TAG,
                "MfClassicPollerStateNestedLog expected 2 nonces, received %zu",
                dict_attack_ctx->nested_nonce.count);
            break;
        }

        uint32_t nonce_pair_count = dict_attack_ctx->prng_type == MfClassicPrngTypeWeak ?
                                        1 :
                                        dict_attack_ctx->nested_nonce.count;

        if(!buffered_file_stream_open(
               stream, MF_CLASSIC_NESTED_LOGS_FILE_PATH, FSAM_WRITE, FSOM_OPEN_APPEND))
            break;

        bool params_write_success = true;
        for(size_t i = 0; i < nonce_pair_count; i++) {
            MfClassicNestedNonce* nonce = &dict_attack_ctx->nested_nonce.nonces[i];
            // TODO FL-3926: Avoid repeating logic here
            uint8_t nonce_sector = nonce->key_idx / (weak_prng ? 4 : 2);
            MfClassicKeyType nonce_key_type =
                (nonce->key_idx % (weak_prng ? 4 : 2) < (weak_prng ? 2 : 1)) ? MfClassicKeyTypeA :
                                                                               MfClassicKeyTypeB;
            furi_string_printf(
                temp_str,
                "Sec %d key %c cuid %08lx",
                nonce_sector,
                (nonce_key_type == MfClassicKeyTypeA) ? 'A' : 'B',
                nonce->cuid);
            for(uint8_t nt_idx = 0; nt_idx < ((weak_prng && (!(static_encrypted))) ? 2 : 1);
                nt_idx++) {
                if(nt_idx == 1) {
                    nonce = &dict_attack_ctx->nested_nonce.nonces[i + 1];
                }
                furi_string_cat_printf(
                    temp_str,
                    " nt%u %08lx ks%u %08lx par%u ",
                    nt_idx,
                    nonce->nt,
                    nt_idx,
                    nonce->nt_enc ^ nonce->nt,
                    nt_idx);
                for(uint8_t pb = 0; pb < 4; pb++) {
                    furi_string_cat_printf(temp_str, "%u", (nonce->par >> (3 - pb)) & 1);
                }
            }
            if(dict_attack_ctx->prng_type == MfClassicPrngTypeWeak) {
                furi_string_cat_printf(temp_str, " dist %u\n", nonce->dist);
            } else {
                furi_string_cat_printf(temp_str, "\n");
            }
            if(!stream_write_string(stream, temp_str)) {
                params_write_success = false;
                break;
            }
        }
        if(!params_write_success) break;

        params_saved = true;
    } while(false);

    furi_assert(params_saved);
    free(dict_attack_ctx->nested_nonce.nonces);
    dict_attack_ctx->nested_nonce.nonces = NULL;
    dict_attack_ctx->nested_nonce.count = 0;
    furi_string_free(temp_str);
    buffered_file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);
    instance->state = MfClassicPollerStateNestedController;
    return command;
}

bool mf_classic_nested_is_target_key_found(MfClassicPoller* instance, bool is_dict_attack) {
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;
    bool is_weak = dict_attack_ctx->prng_type == MfClassicPrngTypeWeak;
    uint16_t nested_target_key = dict_attack_ctx->nested_target_key;

    MfClassicKeyType target_key_type;
    uint8_t target_sector;

    if(is_dict_attack) {
        target_key_type = (((is_weak) && ((nested_target_key % 2) == 0)) ||
                           ((!is_weak) && ((nested_target_key % 16) < 8))) ?
                              MfClassicKeyTypeA :
                              MfClassicKeyTypeB;
        target_sector = is_weak ? (nested_target_key / 2) : (nested_target_key / 16);
    } else {
        target_key_type = (((is_weak) && ((nested_target_key % 4) < 2)) ||
                           ((!is_weak) && ((nested_target_key % 2) == 0))) ?
                              MfClassicKeyTypeA :
                              MfClassicKeyTypeB;
        target_sector = is_weak ? (nested_target_key / 4) : (nested_target_key / 2);
    }

    return mf_classic_is_key_found(instance->data, target_sector, target_key_type);
}

bool is_valid_sum(uint16_t sum) {
    for(size_t i = 0; i < 19; i++) {
        if(sum == valid_sums[i]) {
            return true;
        }
    }
    return false;
}

NfcCommand mf_classic_poller_handler_nested_controller(MfClassicPoller* instance) {
    // This function guides the nested attack through its phases, and iterates over the target keys
    NfcCommand command = mf_classic_poller_handle_data_update(instance);
    MfClassicPollerDictAttackContext* dict_attack_ctx = &instance->mode_ctx.dict_attack_ctx;
    bool initial_dict_attack_iter = false;
    if(dict_attack_ctx->nested_phase == MfClassicNestedPhaseNone) {
        dict_attack_ctx->auth_passed = true;
        bool backdoor_present = (dict_attack_ctx->backdoor != MfClassicBackdoorNone);
        if(!(backdoor_present)) {
            for(uint8_t sector = 0; sector < instance->sectors_total; sector++) {
                for(uint8_t key_type = 0; key_type < 2; key_type++) {
                    if(mf_classic_is_key_found(instance->data, sector, key_type)) {
                        dict_attack_ctx->nested_known_key =
                            mf_classic_get_key(instance->data, sector, key_type);
                        dict_attack_ctx->nested_known_key_sector = sector;
                        dict_attack_ctx->nested_known_key_type = key_type;
                        break;
                    }
                }
            }
            dict_attack_ctx->nested_phase = MfClassicNestedPhaseAnalyzePRNG;
        } else {
            dict_attack_ctx->nested_known_key = dict_attack_ctx->current_key;
            dict_attack_ctx->nested_known_key_sector = 0;
            dict_attack_ctx->nested_known_key_type = MfClassicKeyTypeA;
            dict_attack_ctx->prng_type = MfClassicPrngTypeWeak;
            if(dict_attack_ctx->backdoor == MfClassicBackdoorAuth3) {
                dict_attack_ctx->static_encrypted = true;
            }
            dict_attack_ctx->nested_phase = MfClassicNestedPhaseDictAttack;
            initial_dict_attack_iter = true;
        }
    }
    // Identify PRNG type
    if(dict_attack_ctx->nested_phase == MfClassicNestedPhaseAnalyzePRNG) {
        if(dict_attack_ctx->nested_nonce.count < MF_CLASSIC_NESTED_ANALYZE_NT_COUNT) {
            instance->state = MfClassicPollerStateNestedCollectNt;
            return command;
        } else if(
            (dict_attack_ctx->nested_nonce.count == MF_CLASSIC_NESTED_ANALYZE_NT_COUNT) &&
            (dict_attack_ctx->prng_type == MfClassicPrngTypeUnknown)) {
            instance->state = MfClassicPollerStateNestedAnalyzePRNG;
            return command;
        } else if(dict_attack_ctx->prng_type == MfClassicPrngTypeNoTag) {
            FURI_LOG_E(TAG, "No tag detected");
            // Free nonce array
            // TODO FL-3926: Consider using .count here
            if(dict_attack_ctx->nested_nonce.nonces) {
                free(dict_attack_ctx->nested_nonce.nonces);
                dict_attack_ctx->nested_nonce.nonces = NULL;
                dict_attack_ctx->nested_nonce.count = 0;
            }
            instance->state = MfClassicPollerStateFail;
            return command;
        }
        if(dict_attack_ctx->nested_nonce.nonces) {
            // Free nonce array
            // TODO FL-3926: Consider using .count here
            free(dict_attack_ctx->nested_nonce.nonces);
            dict_attack_ctx->nested_nonce.nonces = NULL;
            dict_attack_ctx->nested_nonce.count = 0;
        }
        dict_attack_ctx->nested_phase = MfClassicNestedPhaseDictAttack;
        initial_dict_attack_iter = true;
    }
    // Accelerated nested dictionary attack
    bool is_weak = dict_attack_ctx->prng_type == MfClassicPrngTypeWeak;
    uint16_t dict_target_key_max = (dict_attack_ctx->prng_type == MfClassicPrngTypeWeak) ?
                                       (instance->sectors_total * 2) :
                                       (instance->sectors_total * 16);
    if(dict_attack_ctx->nested_phase == MfClassicNestedPhaseDictAttackVerify) {
        if(!(mf_classic_nested_is_target_key_found(instance, true)) &&
           (dict_attack_ctx->nested_nonce.count > 0)) {
            dict_attack_ctx->nested_phase = MfClassicNestedPhaseDictAttackResume;
            instance->state = MfClassicPollerStateNestedDictAttack;
            return command;
        } else {
            dict_attack_ctx->auth_passed = true;
            if(dict_attack_ctx->nested_nonce.count > 0) {
                // Free nonce array
                furi_assert(dict_attack_ctx->nested_nonce.nonces);
                free(dict_attack_ctx->nested_nonce.nonces);
                dict_attack_ctx->nested_nonce.nonces = NULL;
                dict_attack_ctx->nested_nonce.count = 0;
            }
            dict_attack_ctx->nested_phase = MfClassicNestedPhaseDictAttack;
        }
    }
    if((dict_attack_ctx->nested_phase == MfClassicNestedPhaseDictAttack ||
        dict_attack_ctx->nested_phase == MfClassicNestedPhaseDictAttackResume) &&
       (dict_attack_ctx->nested_target_key < dict_target_key_max)) {
        bool is_last_iter_for_hard_key =
            ((!is_weak) && ((dict_attack_ctx->nested_target_key % 8) == 7));
        if(initial_dict_attack_iter) {
            // Initialize dictionaries
            // Note: System dict should always exist
            dict_attack_ctx->mf_classic_system_dict =
                keys_dict_check_presence(MF_CLASSIC_NESTED_SYSTEM_DICT_PATH) ?
                    keys_dict_alloc(
                        MF_CLASSIC_NESTED_SYSTEM_DICT_PATH,
                        KeysDictModeOpenExisting,
                        sizeof(MfClassicKey)) :
                    NULL;

            dict_attack_ctx->mf_classic_user_dict =
                keys_dict_check_presence(MF_CLASSIC_NESTED_USER_DICT_PATH) ?
                    keys_dict_alloc(
                        MF_CLASSIC_NESTED_USER_DICT_PATH,
                        KeysDictModeOpenExisting,
                        sizeof(MfClassicKey)) :
                    NULL;
        }
        if((is_weak && (dict_attack_ctx->nested_nonce.count == 1)) ||
           (is_last_iter_for_hard_key && (dict_attack_ctx->nested_nonce.count == 8))) {
            // Key verify and reuse
            dict_attack_ctx->nested_phase = MfClassicNestedPhaseDictAttackVerify;
            dict_attack_ctx->auth_passed = false;
            instance->state = MfClassicPollerStateKeyReuseStartNoOffset;
            return command;
        } else if(dict_attack_ctx->nested_phase == MfClassicNestedPhaseDictAttackResume) {
            dict_attack_ctx->nested_phase = MfClassicNestedPhaseDictAttack;
            dict_attack_ctx->auth_passed = true;
        }
        if(!(dict_attack_ctx->auth_passed)) {
            dict_attack_ctx->attempt_count++;
        } else if(!(initial_dict_attack_iter)) {
            dict_attack_ctx->nested_target_key++;
            dict_attack_ctx->attempt_count = 0;
        }
        dict_attack_ctx->auth_passed = true;
        if(dict_attack_ctx->nested_target_key == dict_target_key_max) {
            if(dict_attack_ctx->mf_classic_system_dict) {
                keys_dict_free(dict_attack_ctx->mf_classic_system_dict);
                dict_attack_ctx->mf_classic_system_dict = NULL;
            }
            if(dict_attack_ctx->mf_classic_user_dict) {
                keys_dict_free(dict_attack_ctx->mf_classic_user_dict);
                dict_attack_ctx->mf_classic_user_dict = NULL;
            }
            dict_attack_ctx->nested_target_key = 0;
            if(mf_classic_is_card_read(instance->data)) {
                // All keys have been collected
                FURI_LOG_D(TAG, "All keys collected and sectors read");
                dict_attack_ctx->nested_phase = MfClassicNestedPhaseFinished;
                instance->state = MfClassicPollerStateSuccess;
                return command;
            }
            if(dict_attack_ctx->backdoor == MfClassicBackdoorAuth3) {
                // Skip initial calibration for static encrypted backdoored tags
                dict_attack_ctx->calibrated = true;
            }
            dict_attack_ctx->nested_phase = MfClassicNestedPhaseCalibrate;
            instance->state = MfClassicPollerStateNestedController;
            return command;
        }
        if(dict_attack_ctx->attempt_count == 0) {
            // Check if the nested target key is a known key
            if(mf_classic_nested_is_target_key_found(instance, true)) {
                // Continue to next key
                instance->state = MfClassicPollerStateNestedController;
                return command;
            }
        }
        if(dict_attack_ctx->attempt_count >= 3) {
            // Unpredictable, skip
            FURI_LOG_E(TAG, "Failed to collect nonce, skipping key");
            dict_attack_ctx->nested_target_key++;
            dict_attack_ctx->attempt_count = 0;
        }
        instance->state = MfClassicPollerStateNestedDictAttack;
        return command;
    }
    // Calibration
    bool initial_collect_nt_enc_iter = false;
    bool recalibrated = false;
    if(!(dict_attack_ctx->calibrated)) {
        if(dict_attack_ctx->prng_type == MfClassicPrngTypeWeak) {
            instance->state = MfClassicPollerStateNestedCalibrate;
            return command;
        }
        initial_collect_nt_enc_iter = true;
        dict_attack_ctx->calibrated = true;
        dict_attack_ctx->nested_phase = MfClassicNestedPhaseCollectNtEnc;
    } else if(dict_attack_ctx->nested_phase == MfClassicNestedPhaseCalibrate) {
        initial_collect_nt_enc_iter = true;
        dict_attack_ctx->nested_phase = MfClassicNestedPhaseCollectNtEnc;
    } else if(dict_attack_ctx->nested_phase == MfClassicNestedPhaseRecalibrate) {
        recalibrated = true;
        dict_attack_ctx->nested_phase = MfClassicNestedPhaseCollectNtEnc;
    }
    // Collect and log nonces
    if(dict_attack_ctx->nested_phase == MfClassicNestedPhaseCollectNtEnc) {
        if(((is_weak) && (dict_attack_ctx->nested_nonce.count == 2)) ||
           ((is_weak) && (dict_attack_ctx->backdoor == MfClassicBackdoorAuth3) &&
            (dict_attack_ctx->nested_nonce.count == 1)) ||
           ((!(is_weak)) && (dict_attack_ctx->nested_nonce.count > 0))) {
            instance->state = MfClassicPollerStateNestedLog;
            return command;
        }
        uint16_t nonce_collect_key_max;
        if(dict_attack_ctx->prng_type == MfClassicPrngTypeWeak) {
            nonce_collect_key_max = instance->sectors_total * 4;
        } else {
            nonce_collect_key_max = instance->sectors_total * 2;
        }
        // Target all remaining sectors, key A and B
        if(dict_attack_ctx->nested_target_key < nonce_collect_key_max) {
            if((!(is_weak)) && (dict_attack_ctx->msb_count == (UINT8_MAX + 1))) {
                if(is_valid_sum(dict_attack_ctx->msb_par_sum)) {
                    // All Hardnested nonces collected
                    dict_attack_ctx->nested_target_key++;
                    dict_attack_ctx->current_key_checked = false;
                    instance->state = MfClassicPollerStateNestedController;
                } else {
                    // Nonces do not match an expected sum
                    dict_attack_ctx->attempt_count++;
                    instance->state = MfClassicPollerStateNestedCollectNtEnc;
                }
                dict_attack_ctx->msb_count = 0;
                dict_attack_ctx->msb_par_sum = 0;
                memset(dict_attack_ctx->nt_enc_msb, 0, sizeof(dict_attack_ctx->nt_enc_msb));
                return command;
            }
            if(initial_collect_nt_enc_iter) {
                dict_attack_ctx->current_key_checked = false;
            }
            if(!(dict_attack_ctx->auth_passed) && !(initial_collect_nt_enc_iter)) {
                dict_attack_ctx->attempt_count++;
            } else {
                if(is_weak && !(initial_collect_nt_enc_iter) && !(recalibrated)) {
                    if(!(dict_attack_ctx->static_encrypted)) {
                        dict_attack_ctx->nested_target_key++;
                    } else {
                        dict_attack_ctx->nested_target_key += 2;
                    }
                    if(dict_attack_ctx->nested_target_key % 2 == 0) {
                        dict_attack_ctx->current_key_checked = false;
                    }
                }
                dict_attack_ctx->attempt_count = 0;
            }
            dict_attack_ctx->auth_passed = true;

            // If we have tried to collect this nonce too many times, skip
            if((is_weak && (dict_attack_ctx->attempt_count >= MF_CLASSIC_NESTED_RETRY_MAXIMUM)) ||
               (!(is_weak) &&
                (dict_attack_ctx->attempt_count >= MF_CLASSIC_NESTED_HARD_RETRY_MAXIMUM))) {
                // Unpredictable, skip
                FURI_LOG_W(TAG, "Failed to collect nonce, skipping key");
                if(dict_attack_ctx->nested_nonce.nonces) {
                    free(dict_attack_ctx->nested_nonce.nonces);
                    dict_attack_ctx->nested_nonce.nonces = NULL;
                    dict_attack_ctx->nested_nonce.count = 0;
                }
                if(is_weak) {
                    dict_attack_ctx->nested_target_key += 2;
                    dict_attack_ctx->current_key_checked = false;
                } else {
                    dict_attack_ctx->msb_count = 0;
                    dict_attack_ctx->msb_par_sum = 0;
                    memset(dict_attack_ctx->nt_enc_msb, 0, sizeof(dict_attack_ctx->nt_enc_msb));
                    dict_attack_ctx->nested_target_key++;
                    dict_attack_ctx->current_key_checked = false;
                }
                dict_attack_ctx->attempt_count = 0;
            }

            FURI_LOG_D(
                TAG,
                "Nested target key: %u (max: %u)",
                dict_attack_ctx->nested_target_key,
                nonce_collect_key_max);

            if(!(dict_attack_ctx->current_key_checked)) {
                if(dict_attack_ctx->nested_target_key == nonce_collect_key_max) {
                    // All nonces have been collected
                    FURI_LOG_D(TAG, "All nonces collected");
                    instance->state = MfClassicPollerStateNestedController;
                    return command;
                }

                dict_attack_ctx->current_key_checked = true;

                // Check if the nested target key is a known key
                if(mf_classic_nested_is_target_key_found(instance, false)) {
                    // Continue to next key
                    if(!(dict_attack_ctx->static_encrypted)) {
                        dict_attack_ctx->nested_target_key++;
                        dict_attack_ctx->current_key_checked = false;
                    }
                    instance->state = MfClassicPollerStateNestedController;
                    return command;
                }

                // If it is not a known key, we'll need to calibrate for static encrypted backdoored tags
                if((dict_attack_ctx->backdoor == MfClassicBackdoorAuth3) &&
                   (dict_attack_ctx->nested_target_key < nonce_collect_key_max) &&
                   !(recalibrated)) {
                    dict_attack_ctx->calibrated = false;
                    dict_attack_ctx->nested_phase = MfClassicNestedPhaseRecalibrate;
                    instance->state = MfClassicPollerStateNestedController;
                    return command;
                }
            }
            FURI_LOG_T(TAG, "Collecting a nonce");

            // Collect a nonce
            dict_attack_ctx->auth_passed = false;
            instance->state = MfClassicPollerStateNestedCollectNtEnc;
            return command;
        }
    }
    dict_attack_ctx->nested_target_key = 0;
    dict_attack_ctx->nested_phase = MfClassicNestedPhaseFinished;
    instance->state = MfClassicPollerStateSuccess;
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
        [MfClassicPollerStateAnalyzeBackdoor] = mf_classic_poller_handler_analyze_backdoor,
        [MfClassicPollerStateBackdoorReadSector] = mf_classic_poller_handler_backdoor_read_sector,
        [MfClassicPollerStateRequestKey] = mf_classic_poller_handler_request_key,
        [MfClassicPollerStateRequestReadSector] = mf_classic_poller_handler_request_read_sector,
        [MfClassicPollerStateReadSectorBlocks] =
            mf_classic_poller_handler_request_read_sector_blocks,
        [MfClassicPollerStateAuthKeyA] = mf_classic_poller_handler_auth_a,
        [MfClassicPollerStateAuthKeyB] = mf_classic_poller_handler_auth_b,
        [MfClassicPollerStateReadSector] = mf_classic_poller_handler_read_sector,
        [MfClassicPollerStateKeyReuseStart] = mf_classic_poller_handler_key_reuse_start,
        [MfClassicPollerStateKeyReuseStartNoOffset] =
            mf_classic_poller_handler_key_reuse_start_no_offset,
        [MfClassicPollerStateKeyReuseAuthKeyA] = mf_classic_poller_handler_key_reuse_auth_key_a,
        [MfClassicPollerStateKeyReuseAuthKeyB] = mf_classic_poller_handler_key_reuse_auth_key_b,
        [MfClassicPollerStateKeyReuseReadSector] = mf_classic_poller_handler_key_reuse_read_sector,
        [MfClassicPollerStateNestedAnalyzePRNG] = mf_classic_poller_handler_nested_analyze_prng,
        [MfClassicPollerStateNestedCalibrate] = mf_classic_poller_handler_nested_calibrate,
        [MfClassicPollerStateNestedCollectNt] = mf_classic_poller_handler_nested_collect_nt,
        [MfClassicPollerStateNestedController] = mf_classic_poller_handler_nested_controller,
        [MfClassicPollerStateNestedCollectNtEnc] = mf_classic_poller_handler_nested_collect_nt_enc,
        [MfClassicPollerStateNestedDictAttack] = mf_classic_poller_handler_nested_dict_attack,
        [MfClassicPollerStateNestedLog] = mf_classic_poller_handler_nested_log,
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
