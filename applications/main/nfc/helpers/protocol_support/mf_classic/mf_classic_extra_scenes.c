#include "mf_classic_extra_scenes.h"

#include "nfc/nfc_app_i.h"
#include "../nfc_protocol_support_gui_common.h"
#include <bit_lib/bit_lib.h>
#include <dolphin/dolphin.h>
#include <toolbox/stream/buffered_file_stream.h>
#include <nfc/protocols/mf_classic/mf_classic_listener.h>
#include "loader/loader.h"
#include <nfc/protocols/mf_classic/mf_classic_poller.h>

enum {
    NfcSceneMfClassicUpdateInitialStateCardSearch,
    NfcSceneMfClassicUpdateInitialStateCardFound,
};

// ---- dict_attack -------------------------------------------------------
#undef TAG
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

static NfcCommand nfc_dict_attack_worker_callback(NfcGenericEvent event, void* context) {
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
        instance->nfc_dict_context.poller_has_card_data = true;
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
            // Standard dictionary: keys are sizeof(MfClassicKey) bytes
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

static void nfc_dict_attack_dict_attack_result_callback(DictAttackEvent event, void* context) {
    furi_assert(context);
    NfcApp* instance = context;

    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

static void mf_classic_scene_dict_attack_update_view(NfcApp* instance) {
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

// Per-UID ("CUID") dictionary path for the detected card (MFKey-recovered keys live here).
static FuriString* mf_classic_scene_dict_attack_cuid_dict_path(NfcApp* instance) {
    size_t cuid_len = 0;
    const uint8_t* cuid = nfc_device_get_uid(instance->nfc_device, &cuid_len);
    return furi_string_alloc_printf(
        "%s/mf_classic_dict_%08lx.nfc",
        EXT_PATH("nfc/assets"),
        (uint32_t)bit_lib_bytes_to_num_be(cuid + (cuid_len - 4), 4));
}

static void mf_classic_scene_dict_attack_prepare_view(NfcApp* instance) {
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack);
    if(state == DictAttackStateCUIDDictInProgress) {
        FuriString* cuid_dict_path = mf_classic_scene_dict_attack_cuid_dict_path(instance);

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
                keys_dict_free(dict);
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
    mf_classic_scene_dict_attack_update_view(instance);

    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfClassicDictAttack, state);
}

static void mf_classic_scene_dict_attack_start_poller(NfcApp* instance) {
    // Every dictionary phase runs on its own poller, so the latch has to be cleared per phase.
    instance->nfc_dict_context.poller_has_card_data = false;
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
    nfc_poller_start(instance->poller, nfc_dict_attack_worker_callback, instance);
}

static void mf_classic_scene_dict_attack_on_enter(NfcApp* instance) {
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfClassicDictAttack, DictAttackStateCUIDDictInProgress);

    // A per-UID ("CUID") dictionary can be large; scanning it in prepare_view blocks the UI, so
    // show a labelled animated loading view meanwhile.
    FuriString* cuid_dict_path = mf_classic_scene_dict_attack_cuid_dict_path(instance);
    bool show_loading = keys_dict_check_presence(furi_string_get_cstr(cuid_dict_path));
    furi_string_free(cuid_dict_path);

    if(show_loading) nfc_show_loading_label_popup(instance, "CUID dictionary\nis loading", true);
    mf_classic_scene_dict_attack_prepare_view(instance);
    if(show_loading) nfc_show_loading_label_popup(instance, NULL, false);

    dict_attack_set_card_state(instance->dict_attack, true);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);

    mf_classic_scene_dict_attack_start_poller(instance);
}

static void mf_classic_scene_dict_attack_notify_read(NfcApp* instance) {
    // Grade the device data, not the poller's: a Skip before activation leaves the poller empty.
    const MfClassicData* mfc_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);
    bool is_card_fully_read = mf_classic_is_card_read(mfc_data);
    if(is_card_fully_read) {
        notification_message(instance->notifications, &sequence_success);
    } else {
        notification_message(instance->notifications, &sequence_semi_success);
    }
}

static bool mf_classic_scene_dict_attack_on_event(NfcApp* instance, SceneManagerEvent event) {
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
                mf_classic_scene_dict_attack_prepare_view(instance);
                mf_classic_scene_dict_attack_start_poller(instance);
                consumed = true;
            } else if(state == DictAttackStateUserDictInProgress && !(ran_nested_dict)) {
                nfc_poller_stop(instance->poller);
                nfc_poller_free(instance->poller);
                keys_dict_free(instance->nfc_dict_context.dict);
                scene_manager_set_scene_state(
                    instance->scene_manager,
                    NfcSceneMfClassicDictAttack,
                    DictAttackStateSystemDictInProgress);
                mf_classic_scene_dict_attack_prepare_view(instance);
                mf_classic_scene_dict_attack_start_poller(instance);
                consumed = true;
            } else {
                mf_classic_scene_dict_attack_notify_read(instance);
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
            mf_classic_scene_dict_attack_update_view(instance);
        } else if(event.event == NfcCustomEventDictAttackSkip) {
            // Skip is live from the first frame, "Lost the tag!" screen included, so this phase's
            // poller may still be blank -- adopting it would wipe the dump we came in with.
            if(instance->nfc_dict_context.poller_has_card_data) {
                const MfClassicData* mfc_data = nfc_poller_get_data(instance->poller);
                nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, mfc_data);
            } else {
                FURI_LOG_W(TAG, "Skipped before card activation, keeping the loaded card");
            }
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
                    mf_classic_scene_dict_attack_prepare_view(instance);
                    mf_classic_scene_dict_attack_start_poller(instance);
                } else {
                    mf_classic_scene_dict_attack_notify_read(instance);
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
                    mf_classic_scene_dict_attack_prepare_view(instance);
                    mf_classic_scene_dict_attack_start_poller(instance);
                } else {
                    mf_classic_scene_dict_attack_notify_read(instance);
                    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                    dolphin_deed(DolphinDeedNfcReadSuccess);
                }
                consumed = true;
            } else {
                mf_classic_scene_dict_attack_notify_read(instance);
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

static void mf_classic_scene_dict_attack_on_exit(NfcApp* instance) {
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
    instance->nfc_dict_context.poller_has_card_data = false;
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

// ---- detect_reader -----------------------------------------------------
#undef TAG
#define NXP_MANUFACTURER_ID (0x04)

#define NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX        (10U)
#define NFC_SCENE_DETECT_READER_WAIT_NONCES_TIMEOUT_MS (1000)

static const NotificationSequence sequence_detect_reader = {
    &message_green_255,
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

static void mf_classic_scene_detect_reader_view_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventViewExit);
}

static NfcCommand mf_classic_scene_detect_listener_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcApp* instance = context;
    MfClassicListenerEvent* mfc_event = event.event_data;

    if(mfc_event->type == MfClassicListenerEventTypeAuthContextPartCollected) {
        MfClassicAuthContext* auth_ctx = &mfc_event->data->auth_context;
        mfkey32_logger_add_nonce(instance->mfkey32_logger, auth_ctx);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerUpdate);
    }

    return NfcCommandContinue;
}

static void mf_classic_scene_timer_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventTimerExpired);
}

static void mf_classic_scene_detect_reader_on_enter(NfcApp* instance) {
    if(nfc_device_get_protocol(instance->nfc_device) == NfcProtocolInvalid) {
        Iso14443_3aData iso3_data = {
            .uid_len = 7,
            .uid = {0},
            .atqa = {0x44, 0x00},
            .sak = 0x08,
        };
        iso3_data.uid[0] = NXP_MANUFACTURER_ID;
        furi_hal_random_fill_buf(&iso3_data.uid[1], iso3_data.uid_len - 1);
        MfClassicData* mfc_data = mf_classic_alloc();

        mfc_data->type = MfClassicType4k;
        iso14443_3a_copy(mfc_data->iso14443_3a_data, &iso3_data);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, mfc_data);

        mf_classic_free(mfc_data);
    }

    const Iso14443_3aData* iso3_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_3a);
    uint32_t cuid = iso14443_3a_get_cuid(iso3_data);

    instance->mfkey32_logger = mfkey32_logger_alloc(cuid);
    instance->timer =
        furi_timer_alloc(mf_classic_scene_timer_callback, FuriTimerTypeOnce, instance);

    detect_reader_set_nonces_max(instance->detect_reader, NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX);
    detect_reader_set_callback(
        instance->detect_reader, mf_classic_scene_detect_reader_view_callback, instance);

    notification_message(instance->notifications, &sequence_detect_reader);

    instance->listener = nfc_listener_alloc(
        instance->nfc,
        NfcProtocolMfClassic,
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic));
    nfc_listener_start(instance->listener, mf_classic_scene_detect_listener_callback, instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDetectReader);
}

static bool mf_classic_scene_detect_reader_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerUpdate) {
            furi_timer_stop(instance->timer);
            notification_message(instance->notifications, &sequence_blink_start_cyan);

            size_t nonces_pairs = 2 * mfkey32_logger_get_params_num(instance->mfkey32_logger);
            detect_reader_set_state(instance->detect_reader, DetectReaderStateReaderDetected);
            detect_reader_set_nonces_collected(instance->detect_reader, nonces_pairs);
            if(nonces_pairs >= NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX) {
                if(instance->listener) {
                    nfc_listener_stop(instance->listener);
                    nfc_listener_free(instance->listener);
                    instance->listener = NULL;
                }
                detect_reader_set_state(instance->detect_reader, DetectReaderStateDone);
                nfc_blink_stop(instance);
                notification_message(instance->notifications, &sequence_single_vibro);
                notification_message(instance->notifications, &sequence_set_green_255);
            } else {
                furi_timer_start(instance->timer, NFC_SCENE_DETECT_READER_WAIT_NONCES_TIMEOUT_MS);
            }
            consumed = true;
        } else if(event.event == NfcCustomEventTimerExpired) {
            detect_reader_set_state(instance->detect_reader, DetectReaderStateReaderLost);
            nfc_blink_stop(instance);
            notification_message(instance->notifications, &sequence_detect_reader);
        } else if(event.event == NfcCustomEventViewExit) {
            if(instance->listener) {
                nfc_listener_stop(instance->listener);
                nfc_listener_free(instance->listener);
                instance->listener = NULL;
            }
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicMfkeyNoncesInfo);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(instance->listener) {
            nfc_listener_stop(instance->listener);
            nfc_listener_free(instance->listener);
            instance->listener = NULL;
        }
        mfkey32_logger_free(instance->mfkey32_logger);
        if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneSaveSuccess)) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneStart);
        } else if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneReadSuccess)) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneReadSuccess);
        }
    }

    return consumed;
}

static void mf_classic_scene_detect_reader_on_exit(NfcApp* instance) {
    // Clear view
    detect_reader_reset(instance->detect_reader);

    furi_timer_stop(instance->timer);
    furi_timer_free(instance->timer);

    // Stop notifications
    nfc_blink_stop(instance);
    notification_message(instance->notifications, &sequence_reset_green);
}

// ---- mfkey_complete ----------------------------------------------------
#undef TAG
typedef enum {
    NfcSceneMfClassicMfKeyCompleteStateAppMissing,
    NfcSceneMfClassicMfKeyCompleteStateAppPresent,
} NfcSceneMfClassicMfKeyCompleteState;

static void
    mf_classic_scene_mfkey_complete_callback(GuiButtonType result, InputType type, void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

static void mf_classic_scene_mfkey_complete_on_enter(NfcApp* instance) {
    widget_add_string_element(
        instance->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Completed!");

    NfcSceneMfClassicMfKeyCompleteState scene_state =
        storage_common_exists(instance->storage, NFC_MFKEY32_APP_PATH) ?
            NfcSceneMfClassicMfKeyCompleteStateAppPresent :
            NfcSceneMfClassicMfKeyCompleteStateAppMissing;
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfClassicMfkeyComplete, scene_state);

    if(scene_state == NfcSceneMfClassicMfKeyCompleteStateAppMissing) {
        widget_add_string_multiline_element(
            instance->widget,
            64,
            13,
            AlignCenter,
            AlignTop,
            FontSecondary,
            "Now use Mfkey32 to extract \nkeys: r.flipper.net/nfc-tools");
        widget_add_icon_element(instance->widget, 50, 39, &I_MFKey_qr_25x25);
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "Finish",
            mf_classic_scene_mfkey_complete_callback,
            instance);
    } else {
        widget_add_string_multiline_element(
            instance->widget,
            60,
            16,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "Now run Mfkey32\n to extract \nkeys");
        widget_add_icon_element(instance->widget, 5, 18, &I_WarningDolphin_45x42);
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "Run",
            mf_classic_scene_mfkey_complete_callback,
            instance);
    }
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool mf_classic_scene_mfkey_complete_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            NfcSceneMfClassicMfKeyCompleteState scene_state = scene_manager_get_scene_state(
                instance->scene_manager, NfcSceneMfClassicMfkeyComplete);
            if(scene_state == NfcSceneMfClassicMfKeyCompleteStateAppMissing) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcSceneStart);
            } else {
                nfc_app_run_external(instance, NFC_MFKEY32_APP_PATH);
            }
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        const uint32_t prev_scenes[] = {NfcSceneSavedMenu, NfcSceneStart};
        consumed = scene_manager_search_and_switch_to_previous_scene_one_of(
            instance->scene_manager, prev_scenes, COUNT_OF(prev_scenes));
    }

    return consumed;
}

static void mf_classic_scene_mfkey_complete_on_exit(NfcApp* instance) {
    widget_reset(instance->widget);
}

// ---- mfkey_nonces_info -------------------------------------------------
#undef TAG
static void mf_classic_scene_mfkey_nonces_info_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

static void mf_classic_scene_mfkey_nonces_info_on_enter(NfcApp* instance) {
    FuriString* temp_str = furi_string_alloc();

    size_t mfkey_params_saved = mfkey32_logger_get_params_num(instance->mfkey32_logger);
    furi_string_printf(temp_str, "Nonce pairs saved: %zu\n", mfkey_params_saved);
    widget_add_string_element(
        instance->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(temp_str));
    widget_add_string_element(
        instance->widget, 0, 12, AlignLeft, AlignTop, FontSecondary, "Authenticated sectors:");

    mfkey32_logger_get_params_data(instance->mfkey32_logger, temp_str);
    widget_add_text_scroll_element(
        instance->widget, 0, 22, 128, 42, furi_string_get_cstr(temp_str));
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeCenter,
        "OK",
        mf_classic_scene_mfkey_nonces_info_callback,
        instance);

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool
    mf_classic_scene_mfkey_nonces_info_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            if(mfkey32_logger_save_params(
                   instance->mfkey32_logger, NFC_APP_MFKEY32_LOGS_FILE_PATH)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicMfkeyComplete);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcSceneStart);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        const uint32_t prev_scenes[] = {NfcSceneSavedMenu, NfcSceneStart};
        consumed = scene_manager_search_and_switch_to_previous_scene_one_of(
            instance->scene_manager, prev_scenes, COUNT_OF(prev_scenes));
    }

    return consumed;
}

static void mf_classic_scene_mfkey_nonces_info_on_exit(NfcApp* instance) {
    mfkey32_logger_free(instance->mfkey32_logger);

    // Clear view
    widget_reset(instance->widget);
}

// ---- show_keys ---------------------------------------------------------
#undef TAG
#define TAG "NfcMfClassicShowKeys"

static void
    mf_classic_scene_show_keys_callback(GuiButtonType button, InputType type, void* context) {
    NfcApp* instance = context;
    if(button == GuiButtonTypeLeft && type == InputTypeShort) {
        scene_manager_previous_scene(instance->scene_manager);
    }
}

static void mf_classic_scene_show_keys_on_enter(NfcApp* instance) {
    const MfClassicData* mfc_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);

    furi_string_reset(instance->text_box_store);
    nfc_append_filename_string_when_present(instance, instance->text_box_store);

    furi_string_cat_printf(instance->text_box_store, "\e#Found MFC Keys:");

    uint8_t num_sectors = mf_classic_get_total_sectors_num(mfc_data->type);
    uint8_t found_keys_a = 0, found_keys_b = 0;
    for(uint8_t i = 0; i < num_sectors; i++) {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(mfc_data, i);

        bool key_a = FURI_BIT(mfc_data->key_a_mask, i);
        bool key_b = FURI_BIT(mfc_data->key_b_mask, i);

        if(key_a || key_b) {
            furi_string_cat_printf(instance->text_box_store, "\n  -> Sector %d\n\e*AccBits:", i);
            for(uint8_t j = 0; j < MF_CLASSIC_ACCESS_BYTES_SIZE; j++) {
                furi_string_cat_printf(
                    instance->text_box_store, " %02X", sec_tr->access_bits.data[j]);
            }
        }

        if(key_a) {
            found_keys_a++;
            furi_string_cat_printf(instance->text_box_store, "\n\e*A:");
            for(uint8_t j = 0; j < MF_CLASSIC_KEY_SIZE; j++)
                furi_string_cat_printf(instance->text_box_store, " %02X", sec_tr->key_a.data[j]);
        }
        if(key_b) {
            found_keys_b++;
            furi_string_cat_printf(instance->text_box_store, "\n\e*B:");
            for(uint8_t j = 0; j < MF_CLASSIC_KEY_SIZE; j++)
                furi_string_cat_printf(instance->text_box_store, " %02X", sec_tr->key_b.data[j]);
        }
    }

    furi_string_cat_printf(
        instance->text_box_store,
        "\nTotal keys found:\n -> %d/%d A keys\n -> %d/%d B keys",
        found_keys_a,
        num_sectors,
        found_keys_b,
        num_sectors);

    widget_add_text_scroll_element(
        instance->widget, 2, 2, 124, 60, furi_string_get_cstr(instance->text_box_store));
    widget_add_button_element(
        instance->widget, GuiButtonTypeLeft, "Back", mf_classic_scene_show_keys_callback, instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool mf_classic_scene_show_keys_on_event(NfcApp* nfc, SceneManagerEvent event) {
    UNUSED(nfc);
    UNUSED(event);
    return false;
}

static void mf_classic_scene_show_keys_on_exit(NfcApp* instance) {
    widget_reset(instance->widget);
    furi_string_reset(instance->text_box_store);
}

// ---- update_initial ----------------------------------------------------
#undef TAG
#define TAG "NfcMfClassicUpdateInitial"

// Overlay onto `base` only what `fresh` actually recovered. The read poller is not seeded with our
// dump -- only the dictionary modes are -- and it reports success as soon as the key cache runs
// out, so a sector that failed to authenticate this pass is simply absent from `fresh`; replacing
// outright would drop its key and blocks. Trailers pass through safely: mf_classic_set_block_read()
// copies only the access bytes out of one, and keys travel through mf_classic_set_key_found().
// Bounds are the array maxima rather than either card's geometry, because the key cache offers
// every sector `base` holds a key for -- `fresh` can come back with sectors past its own type.
static void
    mf_classic_scene_update_initial_merge(MfClassicData* base, const MfClassicData* fresh) {
    for(uint16_t block_num = 0; block_num < MF_CLASSIC_TOTAL_BLOCKS_MAX; block_num++) {
        if(!mf_classic_is_block_read(fresh, block_num)) continue;
        MfClassicBlock block = fresh->block[block_num]; // set_block_read() wants it mutable
        mf_classic_set_block_read(base, block_num, &block);
    }

    const MfClassicKeyType key_types[] = {MfClassicKeyTypeA, MfClassicKeyTypeB};
    for(uint8_t sector_num = 0; sector_num < MF_CLASSIC_TOTAL_SECTORS_MAX; sector_num++) {
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(fresh, sector_num);
        for(size_t i = 0; i < COUNT_OF(key_types); i++) {
            if(!mf_classic_is_key_found(fresh, sector_num, key_types[i])) continue;
            const MfClassicKey* key = (key_types[i] == MfClassicKeyTypeA) ? &sec_tr->key_a :
                                                                            &sec_tr->key_b;
            mf_classic_set_key_found(
                base,
                sector_num,
                key_types[i],
                bit_lib_bytes_to_num_be(key->data, sizeof(MfClassicKey)));
        }
    }
}

// The merge keeps the user's data safe but also hides a weak pass: the poller calls it a success
// once the key cache empties, and the popup says "Updated" either way. Log what the screen cannot.
static void mf_classic_scene_update_initial_log_gaps(
    const MfClassicData* dump,
    const MfClassicData* fresh) {
    uint8_t sectors_had = 0, keys_had = 0, sectors_got = 0, keys_got = 0;
    mf_classic_get_read_sectors_and_keys(dump, &sectors_had, &keys_had);
    mf_classic_get_read_sectors_and_keys(fresh, &sectors_got, &keys_got);
    if(sectors_got < sectors_had || keys_got < keys_had) {
        FURI_LOG_W(
            TAG,
            "Partial re-read: %u/%u sectors, %u/%u keys; keeping the rest of the dump",
            sectors_got,
            sectors_had,
            keys_got,
            keys_had);
    }
    if(dump->type != fresh->type) {
        FURI_LOG_W(
            TAG,
            "Type mismatch: dump %s, card %s; keeping the dump's",
            mf_classic_get_device_name(dump, NfcDeviceNameTypeShort),
            mf_classic_get_device_name(fresh, NfcDeviceNameTypeShort));
    }
}

static NfcCommand
    nfc_mf_classic_update_initial_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcCommand command = NfcCommandContinue;
    const MfClassicPollerEvent* mfc_event = event.event_data;
    NfcApp* instance = context;

    if(mfc_event->type == MfClassicPollerEventTypeCardDetected) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
    } else if(mfc_event->type == MfClassicPollerEventTypeCardLost) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardLost);
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        const MfClassicData* updated_data = nfc_poller_get_data(instance->poller);
        const MfClassicData* old_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);
        if(iso14443_3a_is_equal(updated_data->iso14443_3a_data, old_data->iso14443_3a_data)) {
            mfc_event->data->poller_mode.mode = MfClassicPollerModeRead;
        } else {
            view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
            command = NfcCommandStop;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestReadSector) {
        uint8_t sector_num = 0;
        MfClassicKey key = {};
        MfClassicKeyType key_type = MfClassicKeyTypeA;
        if(mf_classic_key_cache_get_next_key(
               instance->mfc_key_cache, &sector_num, &key, &key_type)) {
            mfc_event->data->read_sector_request_data.sector_num = sector_num;
            mfc_event->data->read_sector_request_data.key = key;
            mfc_event->data->read_sector_request_data.key_type = key_type;
            mfc_event->data->read_sector_request_data.key_provided = true;
        } else {
            mfc_event->data->read_sector_request_data.key_provided = false;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        const MfClassicData* updated_data = nfc_poller_get_data(instance->poller);
        MfClassicData* merged = mf_classic_alloc();
        nfc_device_copy_data(instance->nfc_device, NfcProtocolMfClassic, merged);
        mf_classic_scene_update_initial_log_gaps(merged, updated_data);
        mf_classic_scene_update_initial_merge(merged, updated_data);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, merged);
        mf_classic_free(merged);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerExit);
        command = NfcCommandStop;
    }

    return command;
}

static void mf_classic_scene_update_initial_setup_view(NfcApp* instance) {
    Popup* popup = instance->popup;
    popup_reset(popup);
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicUpdateInitial);

    if(state == NfcSceneMfClassicUpdateInitialStateCardSearch) {
        popup_set_text(
            instance->popup, "Use the source\ncard only", 128, 32, AlignRight, AlignCenter);
        popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    } else {
        popup_set_header(popup, "Updating\nDon't move...", 52, 32, AlignLeft, AlignCenter);
        popup_set_icon(popup, 12, 23, &A_Loading_24);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

static void mf_classic_scene_update_initial_on_enter(NfcApp* instance) {
    dolphin_deed(DolphinDeedNfcEmulate);

    const MfClassicData* mfc_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);
    mf_classic_key_cache_load_from_data(instance->mfc_key_cache, mfc_data);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfClassicUpdateInitial,
        NfcSceneMfClassicUpdateInitialStateCardSearch);
    mf_classic_scene_update_initial_setup_view(instance);

    // Setup and start worker
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
    nfc_poller_start(instance->poller, nfc_mf_classic_update_initial_worker_callback, instance);
    nfc_blink_emulate_start(instance);
}

static bool mf_classic_scene_update_initial_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventCardDetected) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfClassicUpdateInitial,
                NfcSceneMfClassicUpdateInitialStateCardFound);
            mf_classic_scene_update_initial_setup_view(instance);
            consumed = true;
        } else if(event.event == NfcCustomEventCardLost) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfClassicUpdateInitial,
                NfcSceneMfClassicUpdateInitialStateCardSearch);
            mf_classic_scene_update_initial_setup_view(instance);
            consumed = true;
        } else if(event.event == NfcCustomEventWrongCard) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneUpdateInitialWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventWorkerExit) {
            if(nfc_save_shadow_file(instance)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneUpdateInitialSuccess);
            } else {
                scene_manager_next_scene(instance->scene_manager, NfcSceneUpdateInitialWrongCard);
                consumed = true;
            }
        }
    }

    return consumed;
}

static void mf_classic_scene_update_initial_on_exit(NfcApp* instance) {
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfClassicUpdateInitial,
        NfcSceneMfClassicUpdateInitialStateCardSearch);
    // Clear view
    popup_reset(instance->popup);

    nfc_blink_stop(instance);
}

const NfcProtocolSupportExtraScene mf_classic_extra_scenes[MfClassicExtraSceneNum] = {
    [MfClassicExtraSceneDictAttack] =
        {
            .on_enter = mf_classic_scene_dict_attack_on_enter,
            .on_event = mf_classic_scene_dict_attack_on_event,
            .on_exit = mf_classic_scene_dict_attack_on_exit,
        },
    [MfClassicExtraSceneDetectReader] =
        {
            .on_enter = mf_classic_scene_detect_reader_on_enter,
            .on_event = mf_classic_scene_detect_reader_on_event,
            .on_exit = mf_classic_scene_detect_reader_on_exit,
        },
    [MfClassicExtraSceneMfkeyComplete] =
        {
            .on_enter = mf_classic_scene_mfkey_complete_on_enter,
            .on_event = mf_classic_scene_mfkey_complete_on_event,
            .on_exit = mf_classic_scene_mfkey_complete_on_exit,
        },
    [MfClassicExtraSceneMfkeyNoncesInfo] =
        {
            .on_enter = mf_classic_scene_mfkey_nonces_info_on_enter,
            .on_event = mf_classic_scene_mfkey_nonces_info_on_event,
            .on_exit = mf_classic_scene_mfkey_nonces_info_on_exit,
        },
    [MfClassicExtraSceneShowKeys] =
        {
            .on_enter = mf_classic_scene_show_keys_on_enter,
            .on_event = mf_classic_scene_show_keys_on_event,
            .on_exit = mf_classic_scene_show_keys_on_exit,
        },
    [MfClassicExtraSceneUpdateInitial] =
        {
            .on_enter = mf_classic_scene_update_initial_on_enter,
            .on_event = mf_classic_scene_update_initial_on_event,
            .on_exit = mf_classic_scene_update_initial_on_exit,
        },
};
