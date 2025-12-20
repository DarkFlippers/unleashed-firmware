#include "../nfc_app_i.h"

#include <bit_lib/bit_lib.h>
#include <dolphin/dolphin.h>
#include <toolbox/stream/buffered_file_stream.h>

#define TAG       "NfcMfClassicDictAttack"
#define BIT(x, n) ((x) >> (n) & 1)

// TODO FL-3926: Fix lag when leaving the dictionary attack view after Hardnested
// TODO FL-3926: Re-enters backdoor detection between user and system dictionary if no backdoor is found

// KeysDict structure definition for inline CUID dictionary allocation
struct KeysDict {
    Stream* stream;
    size_t key_size;
    size_t key_size_symbols;
    size_t total_keys;
};

typedef enum {
    DictAttackStateCUIDDictInProgress,
    DictAttackStateUserDictInProgress,
    DictAttackStateSystemDictInProgress,
} DictAttackState;

NfcCommand nfc_dict_attack_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcCommand command = NfcCommandContinue;
    MfClassicPollerEvent* mfc_event = event.event_data;

    NfcApp* instance = context;
    if(mfc_event->type == MfClassicPollerEventTypeCardDetected) {
        instance->nfc_dict_context.is_card_present = true;
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
    } else if(mfc_event->type == MfClassicPollerEventTypeCardLost) {
        instance->nfc_dict_context.is_card_present = false;
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardLost);
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        uint32_t state =
            scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack);
        bool is_cuid_dict = (state == DictAttackStateCUIDDictInProgress);

        const MfClassicData* mfc_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);

        // Select mode based on dictionary type
        if(is_cuid_dict) {
            mfc_event->data->poller_mode.mode = MfClassicPollerModeDictAttackCUID;
        } else if(instance->nfc_dict_context.enhanced_dict) {
            mfc_event->data->poller_mode.mode = MfClassicPollerModeDictAttackEnhanced;
        } else {
            mfc_event->data->poller_mode.mode = MfClassicPollerModeDictAttackStandard;
        }

        mfc_event->data->poller_mode.data = mfc_data;
        instance->nfc_dict_context.sectors_total =
            mf_classic_get_total_sectors_num(mfc_data->type);
        mf_classic_get_read_sectors_and_keys(
            mfc_data,
            &instance->nfc_dict_context.sectors_read,
            &instance->nfc_dict_context.keys_found);
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestKey) {
        uint32_t state =
            scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack);
        bool is_cuid_dict = (state == DictAttackStateCUIDDictInProgress);

        MfClassicKey key = {};
        bool key_found = false;

        if(is_cuid_dict) {
            // CUID dictionary: read 7 bytes (1 byte key_idx + 6 bytes key) and filter by exact key_idx
            uint16_t target_key_idx = instance->nfc_dict_context.current_key_idx;

            // Check if this key index exists in the bitmap (only valid for 0-255)
            if(target_key_idx < 256 &&
               BIT(instance->nfc_dict_context.cuid_key_indices_bitmap[target_key_idx / 8],
                   target_key_idx % 8)) {
                uint8_t key_with_idx[sizeof(MfClassicKey) + 1];

                while(keys_dict_get_next_key(
                    instance->nfc_dict_context.dict, key_with_idx, sizeof(MfClassicKey) + 1)) {
                    // Extract key_idx from first byte
                    uint8_t key_idx = key_with_idx[0];

                    instance->nfc_dict_context.dict_keys_current++;

                    // Only use key if it matches the exact current key index
                    if(key_idx == (uint8_t)target_key_idx) {
                        // Copy the actual key (starts at byte 1)
                        memcpy(key.data, &key_with_idx[1], sizeof(MfClassicKey));
                        key_found = true;
                        break;
                    }
                }
            }
        } else {
            // Standard dictionary: read 12 bytes
            if(keys_dict_get_next_key(
                   instance->nfc_dict_context.dict, key.data, sizeof(MfClassicKey))) {
                key_found = true;
                instance->nfc_dict_context.dict_keys_current++;
            }
        }

        if(key_found) {
            mfc_event->data->key_request_data.key = key;
            // In CUID mode, set key_type based on key_idx (odd = B, even = A)
            if(is_cuid_dict) {
                uint16_t target_key_idx = instance->nfc_dict_context.current_key_idx;
                mfc_event->data->key_request_data.key_type =
                    (target_key_idx % 2 == 0) ? MfClassicKeyTypeA : MfClassicKeyTypeB;
            }
            mfc_event->data->key_request_data.key_provided = true;
            if(instance->nfc_dict_context.dict_keys_current % 10 == 0) {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
            }
        } else {
            mfc_event->data->key_request_data.key_provided = false;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeDataUpdate) {
        MfClassicPollerEventDataUpdate* data_update = &mfc_event->data->data_update;
        instance->nfc_dict_context.sectors_read = data_update->sectors_read;
        instance->nfc_dict_context.keys_found = data_update->keys_found;
        instance->nfc_dict_context.current_sector = data_update->current_sector;
        instance->nfc_dict_context.nested_phase = data_update->nested_phase;
        instance->nfc_dict_context.prng_type = data_update->prng_type;
        instance->nfc_dict_context.backdoor = data_update->backdoor;
        instance->nfc_dict_context.nested_target_key = data_update->nested_target_key;
        instance->nfc_dict_context.msb_count = data_update->msb_count;
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfc_event->type == MfClassicPollerEventTypeNextSector) {
        uint32_t state =
            scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack);
        bool is_cuid_dict = (state == DictAttackStateCUIDDictInProgress);

        keys_dict_rewind(instance->nfc_dict_context.dict);
        instance->nfc_dict_context.dict_keys_current = 0;

        // In CUID mode, increment the key index and calculate sector from it
        if(is_cuid_dict) {
            instance->nfc_dict_context.current_key_idx++;
            // Calculate sector from key_idx (each sector has 2 keys: A and B)
            instance->nfc_dict_context.current_sector =
                instance->nfc_dict_context.current_key_idx / 2;
            // Write back to event data so poller can read it
            mfc_event->data->next_sector_data.current_sector =
                instance->nfc_dict_context.current_sector;
        } else {
            instance->nfc_dict_context.current_sector =
                mfc_event->data->next_sector_data.current_sector;
        }

        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfc_event->type == MfClassicPollerEventTypeFoundKeyA) {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfc_event->type == MfClassicPollerEventTypeFoundKeyB) {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfc_event->type == MfClassicPollerEventTypeKeyAttackStart) {
        instance->nfc_dict_context.key_attack_current_sector =
            mfc_event->data->key_attack_data.current_sector;
        instance->nfc_dict_context.is_key_attack = true;
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfc_event->type == MfClassicPollerEventTypeKeyAttackStop) {
        keys_dict_rewind(instance->nfc_dict_context.dict);
        instance->nfc_dict_context.is_key_attack = false;
        instance->nfc_dict_context.dict_keys_current = 0;
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        const MfClassicData* mfc_data = nfc_poller_get_data(instance->poller);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, mfc_data);
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackComplete);
        command = NfcCommandStop;
    }

    return command;
}

void nfc_dict_attack_dict_attack_result_callback(DictAttackEvent event, void* context) {
    furi_assert(context);
    NfcApp* instance = context;

    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

static void nfc_scene_mf_classic_dict_attack_update_view(NfcApp* instance) {
    NfcMfClassicDictAttackContext* mfc_dict = &instance->nfc_dict_context;

    if(mfc_dict->is_key_attack) {
        dict_attack_set_key_attack(instance->dict_attack, mfc_dict->key_attack_current_sector);
    } else {
        dict_attack_reset_key_attack(instance->dict_attack);
        dict_attack_set_sectors_total(instance->dict_attack, mfc_dict->sectors_total);
        dict_attack_set_sectors_read(instance->dict_attack, mfc_dict->sectors_read);
        dict_attack_set_keys_found(instance->dict_attack, mfc_dict->keys_found);
        dict_attack_set_current_dict_key(instance->dict_attack, mfc_dict->dict_keys_current);
        dict_attack_set_current_sector(instance->dict_attack, mfc_dict->current_sector);
        dict_attack_set_nested_phase(instance->dict_attack, mfc_dict->nested_phase);
        dict_attack_set_prng_type(instance->dict_attack, mfc_dict->prng_type);
        dict_attack_set_backdoor(instance->dict_attack, mfc_dict->backdoor);
        dict_attack_set_nested_target_key(instance->dict_attack, mfc_dict->nested_target_key);
        dict_attack_set_msb_count(instance->dict_attack, mfc_dict->msb_count);
    }
}

static void nfc_scene_mf_classic_dict_attack_prepare_view(NfcApp* instance) {
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack);
    if(state == DictAttackStateCUIDDictInProgress) {
        size_t cuid_len = 0;
        const uint8_t* cuid = nfc_device_get_uid(instance->nfc_device, &cuid_len);
        FuriString* cuid_dict_path = furi_string_alloc_printf(
            "%s/mf_classic_dict_%08lx.nfc",
            EXT_PATH("nfc/assets"),
            (uint32_t)bit_lib_bytes_to_num_be(cuid + (cuid_len - 4), 4));

        do {
            if(!keys_dict_check_presence(furi_string_get_cstr(cuid_dict_path))) {
                state = DictAttackStateUserDictInProgress;
                break;
            }

            // Manually create KeysDict and scan once to count + populate bitmap
            KeysDict* dict = malloc(sizeof(KeysDict));
            Storage* storage = furi_record_open(RECORD_STORAGE);
            dict->stream = buffered_file_stream_alloc(storage);
            dict->key_size = sizeof(MfClassicKey) + 1;
            dict->key_size_symbols = dict->key_size * 2 + 1;
            dict->total_keys = 0;

            if(!buffered_file_stream_open(
                   dict->stream,
                   furi_string_get_cstr(cuid_dict_path),
                   FSAM_READ_WRITE,
                   FSOM_OPEN_EXISTING)) {
                buffered_file_stream_close(dict->stream);
                free(dict);
                state = DictAttackStateUserDictInProgress;
                break;
            }

            // Allocate and populate bitmap of key indices present in CUID dictionary
            instance->nfc_dict_context.cuid_key_indices_bitmap = malloc(32);
            memset(instance->nfc_dict_context.cuid_key_indices_bitmap, 0, 32);

            // Scan dictionary once to count keys and populate bitmap
            uint8_t key_with_idx[dict->key_size];
            while(keys_dict_get_next_key(dict, key_with_idx, dict->key_size)) {
                uint8_t key_idx = key_with_idx[0];
                // Set bit for this key index
                instance->nfc_dict_context.cuid_key_indices_bitmap[key_idx / 8] |=
                    (1 << (key_idx % 8));
                dict->total_keys++;
            }
            keys_dict_rewind(dict);

            if(dict->total_keys == 0) {
                keys_dict_free(dict);
                free(instance->nfc_dict_context.cuid_key_indices_bitmap);
                instance->nfc_dict_context.cuid_key_indices_bitmap = NULL;
                state = DictAttackStateUserDictInProgress;
                break;
            }

            instance->nfc_dict_context.dict = dict;
            dict_attack_set_header(instance->dict_attack, "MF Classic CUID Dictionary");
            instance->nfc_dict_context.current_key_idx = 0; // Initialize key index for CUID mode
        } while(false);

        furi_string_free(cuid_dict_path);
    }
    if(state == DictAttackStateUserDictInProgress) {
        do {
            instance->nfc_dict_context.enhanced_dict = true;

            if(keys_dict_check_presence(NFC_APP_MF_CLASSIC_DICT_SYSTEM_NESTED_PATH)) {
                storage_common_remove(
                    instance->storage, NFC_APP_MF_CLASSIC_DICT_SYSTEM_NESTED_PATH);
            }
            if(keys_dict_check_presence(NFC_APP_MF_CLASSIC_DICT_SYSTEM_PATH)) {
                storage_common_copy(
                    instance->storage,
                    NFC_APP_MF_CLASSIC_DICT_SYSTEM_PATH,
                    NFC_APP_MF_CLASSIC_DICT_SYSTEM_NESTED_PATH);
            }

            if(!keys_dict_check_presence(NFC_APP_MF_CLASSIC_DICT_USER_PATH)) {
                state = DictAttackStateSystemDictInProgress;
                break;
            }

            if(keys_dict_check_presence(NFC_APP_MF_CLASSIC_DICT_USER_NESTED_PATH)) {
                storage_common_remove(instance->storage, NFC_APP_MF_CLASSIC_DICT_USER_NESTED_PATH);
            }
            storage_common_copy(
                instance->storage,
                NFC_APP_MF_CLASSIC_DICT_USER_PATH,
                NFC_APP_MF_CLASSIC_DICT_USER_NESTED_PATH);

            instance->nfc_dict_context.dict = keys_dict_alloc(
                NFC_APP_MF_CLASSIC_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfClassicKey));
            if(keys_dict_get_total_keys(instance->nfc_dict_context.dict) == 0) {
                keys_dict_free(instance->nfc_dict_context.dict);
                state = DictAttackStateSystemDictInProgress;
                break;
            }

            dict_attack_set_header(instance->dict_attack, "MF Classic User Dictionary");
        } while(false);
    }
    if(state == DictAttackStateSystemDictInProgress) {
        instance->nfc_dict_context.dict = keys_dict_alloc(
            NFC_APP_MF_CLASSIC_DICT_SYSTEM_PATH, KeysDictModeOpenExisting, sizeof(MfClassicKey));
        dict_attack_set_header(instance->dict_attack, "MF Classic System Dictionary");
    }

    instance->nfc_dict_context.dict_keys_total =
        keys_dict_get_total_keys(instance->nfc_dict_context.dict);
    dict_attack_set_total_dict_keys(
        instance->dict_attack, instance->nfc_dict_context.dict_keys_total);
    instance->nfc_dict_context.dict_keys_current = 0;

    dict_attack_set_callback(
        instance->dict_attack, nfc_dict_attack_dict_attack_result_callback, instance);
    nfc_scene_mf_classic_dict_attack_update_view(instance);

    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack, state);
}

void nfc_scene_mf_classic_dict_attack_on_enter(void* context) {
    NfcApp* instance = context;

    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfClassicDictAttack, DictAttackStateCUIDDictInProgress);
    nfc_scene_mf_classic_dict_attack_prepare_view(instance);
    dict_attack_set_card_state(instance->dict_attack, true);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
    nfc_poller_start(instance->poller, nfc_dict_attack_worker_callback, instance);
}

static void nfc_scene_mf_classic_dict_attack_notify_read(NfcApp* instance) {
    const MfClassicData* mfc_data = nfc_poller_get_data(instance->poller);
    bool is_card_fully_read = mf_classic_is_card_read(mfc_data);
    if(is_card_fully_read) {
        notification_message(instance->notifications, &sequence_success);
    } else {
        notification_message(instance->notifications, &sequence_semi_success);
    }
}

bool nfc_scene_mf_classic_dict_attack_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack);
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventDictAttackComplete) {
            bool ran_nested_dict = instance->nfc_dict_context.nested_phase !=
                                   MfClassicNestedPhaseNone;
            if(state == DictAttackStateCUIDDictInProgress) {
                nfc_poller_stop(instance->poller);
                nfc_poller_free(instance->poller);
                keys_dict_free(instance->nfc_dict_context.dict);
                if(instance->nfc_dict_context.cuid_key_indices_bitmap) {
                    free(instance->nfc_dict_context.cuid_key_indices_bitmap);
                    instance->nfc_dict_context.cuid_key_indices_bitmap = NULL;
                }
                scene_manager_set_scene_state(
                    instance->scene_manager,
                    NfcSceneMfClassicDictAttack,
                    DictAttackStateUserDictInProgress);
                nfc_scene_mf_classic_dict_attack_prepare_view(instance);
                instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
                nfc_poller_start(instance->poller, nfc_dict_attack_worker_callback, instance);
                consumed = true;
            } else if(state == DictAttackStateUserDictInProgress && !(ran_nested_dict)) {
                nfc_poller_stop(instance->poller);
                nfc_poller_free(instance->poller);
                keys_dict_free(instance->nfc_dict_context.dict);
                scene_manager_set_scene_state(
                    instance->scene_manager,
                    NfcSceneMfClassicDictAttack,
                    DictAttackStateSystemDictInProgress);
                nfc_scene_mf_classic_dict_attack_prepare_view(instance);
                instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
                nfc_poller_start(instance->poller, nfc_dict_attack_worker_callback, instance);
                consumed = true;
            } else {
                nfc_scene_mf_classic_dict_attack_notify_read(instance);
                scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                dolphin_deed(DolphinDeedNfcReadSuccess);
                consumed = true;
            }
        } else if(event.event == NfcCustomEventCardDetected) {
            dict_attack_set_card_state(instance->dict_attack, true);
            consumed = true;
        } else if(event.event == NfcCustomEventCardLost) {
            dict_attack_set_card_state(instance->dict_attack, false);
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackDataUpdate) {
            nfc_scene_mf_classic_dict_attack_update_view(instance);
        } else if(event.event == NfcCustomEventDictAttackSkip) {
            const MfClassicData* mfc_data = nfc_poller_get_data(instance->poller);
            nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, mfc_data);
            bool ran_nested_dict = instance->nfc_dict_context.nested_phase !=
                                   MfClassicNestedPhaseNone;
            if(state == DictAttackStateCUIDDictInProgress) {
                if(instance->nfc_dict_context.is_card_present) {
                    nfc_poller_stop(instance->poller);
                    nfc_poller_free(instance->poller);
                    keys_dict_free(instance->nfc_dict_context.dict);
                    if(instance->nfc_dict_context.cuid_key_indices_bitmap) {
                        free(instance->nfc_dict_context.cuid_key_indices_bitmap);
                        instance->nfc_dict_context.cuid_key_indices_bitmap = NULL;
                    }
                    scene_manager_set_scene_state(
                        instance->scene_manager,
                        NfcSceneMfClassicDictAttack,
                        DictAttackStateUserDictInProgress);
                    nfc_scene_mf_classic_dict_attack_prepare_view(instance);
                    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
                    nfc_poller_start(instance->poller, nfc_dict_attack_worker_callback, instance);
                } else {
                    nfc_scene_mf_classic_dict_attack_notify_read(instance);
                    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                    dolphin_deed(DolphinDeedNfcReadSuccess);
                }
                consumed = true;
            } else if(state == DictAttackStateUserDictInProgress && !(ran_nested_dict)) {
                if(instance->nfc_dict_context.is_card_present) {
                    nfc_poller_stop(instance->poller);
                    nfc_poller_free(instance->poller);
                    keys_dict_free(instance->nfc_dict_context.dict);
                    scene_manager_set_scene_state(
                        instance->scene_manager,
                        NfcSceneMfClassicDictAttack,
                        DictAttackStateSystemDictInProgress);
                    nfc_scene_mf_classic_dict_attack_prepare_view(instance);
                    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
                    nfc_poller_start(instance->poller, nfc_dict_attack_worker_callback, instance);
                } else {
                    nfc_scene_mf_classic_dict_attack_notify_read(instance);
                    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                    dolphin_deed(DolphinDeedNfcReadSuccess);
                }
                consumed = true;
            } else {
                nfc_scene_mf_classic_dict_attack_notify_read(instance);
                scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                dolphin_deed(DolphinDeedNfcReadSuccess);
                consumed = true;
            }
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_mf_classic_dict_attack_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    dict_attack_reset(instance->dict_attack);
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfClassicDictAttack, DictAttackStateCUIDDictInProgress);

    keys_dict_free(instance->nfc_dict_context.dict);

    // Free CUID bitmap if allocated
    if(instance->nfc_dict_context.cuid_key_indices_bitmap) {
        free(instance->nfc_dict_context.cuid_key_indices_bitmap);
        instance->nfc_dict_context.cuid_key_indices_bitmap = NULL;
    }

    instance->nfc_dict_context.current_sector = 0;
    instance->nfc_dict_context.sectors_total = 0;
    instance->nfc_dict_context.sectors_read = 0;
    instance->nfc_dict_context.keys_found = 0;
    instance->nfc_dict_context.dict_keys_total = 0;
    instance->nfc_dict_context.dict_keys_current = 0;
    instance->nfc_dict_context.is_key_attack = false;
    instance->nfc_dict_context.key_attack_current_sector = 0;
    instance->nfc_dict_context.is_card_present = false;
    instance->nfc_dict_context.nested_phase = MfClassicNestedPhaseNone;
    instance->nfc_dict_context.prng_type = MfClassicPrngTypeUnknown;
    instance->nfc_dict_context.backdoor = MfClassicBackdoorUnknown;
    instance->nfc_dict_context.nested_target_key = 0;
    instance->nfc_dict_context.msb_count = 0;
    instance->nfc_dict_context.enhanced_dict = false;
    instance->nfc_dict_context.current_key_idx = 0;

    // Clean up temporary files used for nested dictionary attack
    if(keys_dict_check_presence(NFC_APP_MF_CLASSIC_DICT_USER_NESTED_PATH)) {
        storage_common_remove(instance->storage, NFC_APP_MF_CLASSIC_DICT_USER_NESTED_PATH);
    }
    if(keys_dict_check_presence(NFC_APP_MF_CLASSIC_DICT_SYSTEM_NESTED_PATH)) {
        storage_common_remove(instance->storage, NFC_APP_MF_CLASSIC_DICT_SYSTEM_NESTED_PATH);
    }

    nfc_blink_stop(instance);
}
