#include "../nfc_app_i.h"
#include <dolphin/dolphin.h>

#define TAG "NfcMfUlCDictAttack"

// TODO: Support card_detected properly

enum {
    DictAttackStateUserDictInProgress,
    DictAttackStateSystemDictInProgress,
};

NfcCommand nfc_mf_ultralight_c_dict_attack_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfUltralight);
    NfcCommand command = NfcCommandContinue;
    NfcApp* instance = context;
    MfUltralightPollerEvent* poller_event = event.event_data;

    if(poller_event->type == MfUltralightPollerEventTypeRequestMode) {
        poller_event->data->poller_mode = MfUltralightPollerModeDictAttack;
        command = NfcCommandContinue;
    } else if(poller_event->type == MfUltralightPollerEventTypeRequestKey) {
        MfUltralightC3DesAuthKey key = {};
        if(keys_dict_get_next_key(
               instance->mf_ultralight_c_dict_context.dict,
               key.data,
               sizeof(MfUltralightC3DesAuthKey))) {
            poller_event->data->key_request_data.key = key;
            poller_event->data->key_request_data.key_provided = true;
            instance->mf_ultralight_c_dict_context.dict_keys_current++;

            if(instance->mf_ultralight_c_dict_context.dict_keys_current % 10 == 0) {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
            }
        } else {
            poller_event->data->key_request_data.key_provided = false;
        }
    } else if(poller_event->type == MfUltralightPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfUltralight, nfc_poller_get_data(instance->poller));
        // Check if this is a successful authentication by looking at the poller's auth context
        const MfUltralightData* data = nfc_poller_get_data(instance->poller);

        // Update page information
        dict_attack_set_pages_read(instance->dict_attack, data->pages_read);
        dict_attack_set_pages_total(instance->dict_attack, data->pages_total);

        if(data->pages_read == data->pages_total) {
            // Full read indicates successful authentication in dict attack mode
            instance->mf_ultralight_c_dict_context.auth_success = true;
            dict_attack_set_key_found(instance->dict_attack, true);
        }
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackComplete);
        command = NfcCommandStop;
    }
    return command;
}

void nfc_scene_mf_ultralight_c_dict_attack_dict_attack_result_callback(
    DictAttackEvent event,
    void* context) {
    furi_assert(context);
    NfcApp* instance = context;
    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

void nfc_scene_mf_ultralight_c_dict_attack_prepare_view(NfcApp* instance) {
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightCDictAttack);

    // Set attack type to Ultralight C
    dict_attack_set_type(instance->dict_attack, DictAttackTypeMfUltralightC);

    if(state == DictAttackStateUserDictInProgress) {
        do {
            if(!keys_dict_check_presence(NFC_APP_MF_ULTRALIGHT_C_DICT_USER_PATH)) {
                state = DictAttackStateSystemDictInProgress;
                break;
            }
            instance->mf_ultralight_c_dict_context.dict = keys_dict_alloc(
                NFC_APP_MF_ULTRALIGHT_C_DICT_USER_PATH,
                KeysDictModeOpenAlways,
                sizeof(MfUltralightC3DesAuthKey));
            if(keys_dict_get_total_keys(instance->mf_ultralight_c_dict_context.dict) == 0) {
                keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
                state = DictAttackStateSystemDictInProgress;
                break;
            }
            dict_attack_set_header(instance->dict_attack, "MFUL C User Dictionary");
        } while(false);
    }
    if(state == DictAttackStateSystemDictInProgress) {
        instance->mf_ultralight_c_dict_context.dict = keys_dict_alloc(
            NFC_APP_MF_ULTRALIGHT_C_DICT_SYSTEM_PATH,
            KeysDictModeOpenExisting,
            sizeof(MfUltralightC3DesAuthKey));
        dict_attack_set_header(instance->dict_attack, "MFUL C System Dictionary");
    }

    instance->mf_ultralight_c_dict_context.dict_keys_total =
        keys_dict_get_total_keys(instance->mf_ultralight_c_dict_context.dict);
    dict_attack_set_total_dict_keys(
        instance->dict_attack, instance->mf_ultralight_c_dict_context.dict_keys_total);
    instance->mf_ultralight_c_dict_context.dict_keys_current = 0;
    dict_attack_set_current_dict_key(
        instance->dict_attack, instance->mf_ultralight_c_dict_context.dict_keys_current);

    // Set initial Ultralight C specific values
    dict_attack_set_key_found(instance->dict_attack, false);
    dict_attack_set_pages_total(instance->dict_attack, 48); // Ultralight C page count
    dict_attack_set_pages_read(instance->dict_attack, 0);

    dict_attack_set_callback(
        instance->dict_attack,
        nfc_scene_mf_ultralight_c_dict_attack_dict_attack_result_callback,
        instance);
    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfUltralightCDictAttack, state);
}

void nfc_scene_mf_ultralight_c_dict_attack_on_enter(void* context) {
    NfcApp* instance = context;

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightCDictAttack,
        DictAttackStateUserDictInProgress);
    nfc_scene_mf_ultralight_c_dict_attack_prepare_view(instance);

    // Setup and start worker
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(instance->poller, nfc_mf_ultralight_c_dict_attack_worker_callback, instance);

    dict_attack_set_card_state(instance->dict_attack, true);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);
}

void nfc_scene_mf_ul_c_dict_attack_update_view(NfcApp* instance) {
    dict_attack_set_card_state(
        instance->dict_attack, instance->mf_ultralight_c_dict_context.is_card_present);
    dict_attack_set_current_dict_key(
        instance->dict_attack, instance->mf_ultralight_c_dict_context.dict_keys_current);
}

bool nfc_scene_mf_ultralight_c_dict_attack_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightCDictAttack);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventDictAttackComplete) {
            if(state == DictAttackStateUserDictInProgress) {
                if(instance->mf_ultralight_c_dict_context.auth_success) {
                    notification_message(instance->notifications, &sequence_success);
                    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                    dolphin_deed(DolphinDeedNfcReadSuccess);
                    consumed = true;
                } else {
                    nfc_poller_stop(instance->poller);
                    nfc_poller_free(instance->poller);
                    keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
                    scene_manager_set_scene_state(
                        instance->scene_manager,
                        NfcSceneMfUltralightCDictAttack,
                        DictAttackStateSystemDictInProgress);
                    nfc_scene_mf_ultralight_c_dict_attack_prepare_view(instance);
                    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
                    nfc_poller_start(
                        instance->poller,
                        nfc_mf_ultralight_c_dict_attack_worker_callback,
                        instance);
                    consumed = true;
                }
            } else {
                // Could check if card is fully read here like MFC dict attack, but found key means fully read
                if(instance->mf_ultralight_c_dict_context.auth_success) {
                    notification_message(instance->notifications, &sequence_success);
                } else {
                    notification_message(instance->notifications, &sequence_semi_success);
                }
                scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                dolphin_deed(DolphinDeedNfcReadSuccess);
                consumed = true;
            }
        } else if(event.event == NfcCustomEventDictAttackDataUpdate) {
            dict_attack_set_current_dict_key(
                instance->dict_attack, instance->mf_ultralight_c_dict_context.dict_keys_current);
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackSkip) {
            if(state == DictAttackStateUserDictInProgress) {
                nfc_poller_stop(instance->poller);
                nfc_poller_free(instance->poller);
                keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
                scene_manager_set_scene_state(
                    instance->scene_manager,
                    NfcSceneMfUltralightCDictAttack,
                    DictAttackStateSystemDictInProgress);
                nfc_scene_mf_ultralight_c_dict_attack_prepare_view(instance);
                instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
                nfc_poller_start(
                    instance->poller, nfc_mf_ultralight_c_dict_attack_worker_callback, instance);
            } else {
                notification_message(instance->notifications, &sequence_semi_success);
                scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                dolphin_deed(DolphinDeedNfcReadSuccess);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_mf_ultralight_c_dict_attack_on_exit(void* context) {
    NfcApp* instance = context;
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);
    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightCDictAttack,
        DictAttackStateUserDictInProgress);
    keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
    instance->mf_ultralight_c_dict_context.dict_keys_total = 0;
    instance->mf_ultralight_c_dict_context.dict_keys_current = 0;
    instance->mf_ultralight_c_dict_context.auth_success = false;
    instance->mf_ultralight_c_dict_context.is_card_present = false;
    nfc_blink_stop(instance);
}
