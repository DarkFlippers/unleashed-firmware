#include "../nfc_i.h"
#include <dolphin/dolphin.h>

#define TAG "NfcMfClassicDictAttack"

typedef enum {
    DictAttackStateIdle,
    DictAttackStateUserDictInProgress,
    DictAttackStateFlipperDictInProgress,
} DictAttackState;

bool nfc_dict_attack_worker_callback(NfcWorkerEvent event, void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, event);
    return true;
}

void nfc_dict_attack_dict_attack_result_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventDictAttackSkip);
}

static void nfc_scene_mf_classic_dict_attack_update_view(Nfc* nfc) {
    MfClassicData* data = &nfc->dev->dev_data.mf_classic_data;
    uint8_t sectors_read = 0;
    uint8_t keys_found = 0;

    // Calculate found keys and read sectors
    mf_classic_get_read_sectors_and_keys(data, &sectors_read, &keys_found);
    dict_attack_set_keys_found(nfc->dict_attack, keys_found);
    dict_attack_set_sector_read(nfc->dict_attack, sectors_read);
}

static void nfc_scene_mf_classic_dict_attack_prepare_view(Nfc* nfc, DictAttackState state) {
    MfClassicData* data = &nfc->dev->dev_data.mf_classic_data;
    NfcMfClassicDictAttackData* dict_attack_data = &nfc->dev->dev_data.mf_classic_dict_attack_data;
    NfcWorkerState worker_state = NfcWorkerStateReady;
    MfClassicDict* dict = NULL;

    // Identify scene state
    if(state == DictAttackStateIdle) {
        if(mf_classic_dict_check_presence(MfClassicDictTypeUser)) {
            state = DictAttackStateUserDictInProgress;
        } else {
            state = DictAttackStateFlipperDictInProgress;
        }
    } else if(state == DictAttackStateUserDictInProgress) {
        state = DictAttackStateFlipperDictInProgress;
    }

    // Setup view
    if(state == DictAttackStateUserDictInProgress) {
        worker_state = NfcWorkerStateMfClassicDictAttack;
        dict_attack_set_header(nfc->dict_attack, "MF Classic User Dictionary");
        dict = mf_classic_dict_alloc(MfClassicDictTypeUser);

        // If failed to load user dictionary - try the system dictionary
        if(!dict) {
            FURI_LOG_E(TAG, "User dictionary not found");
            state = DictAttackStateFlipperDictInProgress;
        }
    }
    if(state == DictAttackStateFlipperDictInProgress) {
        worker_state = NfcWorkerStateMfClassicDictAttack;
        dict_attack_set_header(nfc->dict_attack, "MF Classic System Dictionary");
        dict = mf_classic_dict_alloc(MfClassicDictTypeSystem);
        if(!dict) {
            FURI_LOG_E(TAG, "Flipper dictionary not found");
            // Pass through to let the worker handle the failure
        }
    }
    // Free previous dictionary
    if(dict_attack_data->dict) {
        mf_classic_dict_free(dict_attack_data->dict);
    }
    dict_attack_data->dict = dict;
    scene_manager_set_scene_state(nfc->scene_manager, NfcSceneMfClassicDictAttack, state);
    dict_attack_set_callback(nfc->dict_attack, nfc_dict_attack_dict_attack_result_callback, nfc);
    dict_attack_set_current_sector(nfc->dict_attack, 0);
    dict_attack_set_card_detected(nfc->dict_attack, data->type);
    dict_attack_set_total_dict_keys(
        nfc->dict_attack, dict ? mf_classic_dict_get_total_keys(dict) : 0);
    nfc_scene_mf_classic_dict_attack_update_view(nfc);
    nfc_worker_start(
        nfc->worker, worker_state, &nfc->dev->dev_data, nfc_dict_attack_worker_callback, nfc);
}

void nfc_scene_mf_classic_dict_attack_on_enter(void* context) {
    Nfc* nfc = context;
    nfc_scene_mf_classic_dict_attack_prepare_view(nfc, DictAttackStateIdle);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(nfc);
    notification_message(nfc->notifications, &sequence_display_backlight_enforce_on);
}

bool nfc_scene_mf_classic_dict_attack_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    MfClassicData* data = &nfc->dev->dev_data.mf_classic_data;
    bool consumed = false;

    uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfClassicDictAttack);
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcWorkerEventSuccess) {
            if(state == DictAttackStateUserDictInProgress) {
                nfc_worker_stop(nfc->worker);
                nfc_scene_mf_classic_dict_attack_prepare_view(nfc, state);
                consumed = true;
            } else {
                notification_message(nfc->notifications, &sequence_success);
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicReadSuccess);
                DOLPHIN_DEED(DolphinDeedNfcReadSuccess);
                consumed = true;
            }
        } else if(event.event == NfcWorkerEventAborted) {
            if(state == DictAttackStateUserDictInProgress &&
               dict_attack_get_card_state(nfc->dict_attack)) {
                nfc_scene_mf_classic_dict_attack_prepare_view(nfc, state);
                consumed = true;
            } else {
                notification_message(nfc->notifications, &sequence_success);
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicReadSuccess);
                // Counting failed attempts too
                DOLPHIN_DEED(DolphinDeedNfcReadSuccess);
                consumed = true;
            }
        } else if(event.event == NfcWorkerEventCardDetected) {
            dict_attack_set_card_detected(nfc->dict_attack, data->type);
            consumed = true;
        } else if(event.event == NfcWorkerEventNoCardDetected) {
            dict_attack_set_card_removed(nfc->dict_attack);
            consumed = true;
        } else if(event.event == NfcWorkerEventFoundKeyA) {
            dict_attack_inc_keys_found(nfc->dict_attack);
            consumed = true;
        } else if(event.event == NfcWorkerEventFoundKeyB) {
            dict_attack_inc_keys_found(nfc->dict_attack);
            consumed = true;
        } else if(event.event == NfcWorkerEventNewSector) {
            nfc_scene_mf_classic_dict_attack_update_view(nfc);
            dict_attack_inc_current_sector(nfc->dict_attack);
            consumed = true;
        } else if(event.event == NfcWorkerEventNewDictKeyBatch) {
            nfc_scene_mf_classic_dict_attack_update_view(nfc);
            dict_attack_inc_current_dict_key(nfc->dict_attack, NFC_DICT_KEY_BATCH_SIZE);
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackSkip) {
            if(state == DictAttackStateUserDictInProgress) {
                nfc_worker_stop(nfc->worker);
                consumed = true;
            } else if(state == DictAttackStateFlipperDictInProgress) {
                nfc_worker_stop(nfc->worker);
                consumed = true;
            }
        } else if(event.event == NfcWorkerEventKeyAttackStart) {
            dict_attack_set_key_attack(
                nfc->dict_attack,
                true,
                nfc->dev->dev_data.mf_classic_dict_attack_data.current_sector);
        } else if(event.event == NfcWorkerEventKeyAttackStop) {
            dict_attack_set_key_attack(nfc->dict_attack, false, 0);
        } else if(event.event == NfcWorkerEventKeyAttackNextSector) {
            dict_attack_inc_key_attack_current_sector(nfc->dict_attack);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_mf_classic_dict_attack_on_exit(void* context) {
    Nfc* nfc = context;
    NfcMfClassicDictAttackData* dict_attack_data = &nfc->dev->dev_data.mf_classic_dict_attack_data;
    // Stop worker
    nfc_worker_stop(nfc->worker);
    if(dict_attack_data->dict) {
        mf_classic_dict_free(dict_attack_data->dict);
        dict_attack_data->dict = NULL;
    }
    dict_attack_reset(nfc->dict_attack);
    nfc_blink_stop(nfc);
    notification_message(nfc->notifications, &sequence_display_backlight_enforce_auto);
}
