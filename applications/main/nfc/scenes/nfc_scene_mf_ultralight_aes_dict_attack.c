#include "../nfc_app_i.h"
#include <dolphin/dolphin.h>

#define TAG "NfcMfUlAesDictAttack"

// Mirrors the Ultralight-C dictionary attack, but feeds 16-byte AES-128 keys to the poller's
// AES 3-pass auth. The poller tries one key per activation (a wrong key unselects the card), so
// the scene just streams keys from the user dict, then the system dict.

enum {
    DictAttackStateUserDictInProgress,
    DictAttackStateSystemDictInProgress,
};

NfcCommand
    nfc_mf_ultralight_aes_dict_attack_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfUltralight);
    NfcCommand command = NfcCommandContinue;
    NfcApp* instance = context;
    MfUltralightPollerEvent* poller_event = event.event_data;

    if(poller_event->type == MfUltralightPollerEventTypeRequestMode) {
        poller_event->data->poller_mode = MfUltralightPollerModeDictAttack;
    } else if(poller_event->type == MfUltralightPollerEventTypeRequestKey) {
        MfUltralightAesKey key = {};
        if(keys_dict_get_next_key(
               instance->mf_ultralight_aes_dict_context.dict,
               key.data,
               sizeof(MfUltralightAesKey))) {
            poller_event->data->key_request_data.aes_key = key;
            poller_event->data->key_request_data.key_provided = true;
            instance->mf_ultralight_aes_dict_context.dict_keys_current++;

            if(instance->mf_ultralight_aes_dict_context.dict_keys_current % 10 == 0) {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
            }
        } else {
            poller_event->data->key_request_data.key_provided = false;
        }
    } else if(poller_event->type == MfUltralightPollerEventTypeAuthSuccess) {
        // A key authenticated. Record success from the auth event itself, not from a byte-perfect
        // 60-page read: a dropped frame during the post-auth read must not discard a found key.
        instance->mf_ultralight_aes_dict_context.auth_success = true;
        dict_attack_set_key_found(instance->dict_attack, true);
    } else if(
        poller_event->type == MfUltralightPollerEventTypeReadSuccess ||
        poller_event->type == MfUltralightPollerEventTypeReadFailed) {
        // ReadFailed also surfaces the ISO error path (card removed / comms glitch). Ending the
        // round here is what stops the poller from re-detecting and spinning forever.
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfUltralight, nfc_poller_get_data(instance->poller));
        const MfUltralightData* data = nfc_poller_get_data(instance->poller);

        dict_attack_set_pages_read(instance->dict_attack, data->pages_read);
        dict_attack_set_pages_total(instance->dict_attack, data->pages_total);

        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackComplete);
        command = NfcCommandStop;
    }
    return command;
}

static void
    nfc_scene_mf_ultralight_aes_dict_attack_result_callback(DictAttackEvent event, void* context) {
    furi_assert(context);
    NfcApp* instance = context;
    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

static void nfc_scene_mf_ultralight_aes_dict_attack_prepare_view(NfcApp* instance) {
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightAesDictAttack);

    dict_attack_set_type(instance->dict_attack, DictAttackTypeMfUltralightAES);

    // Guard: close any dict handle left open by a prior phase before opening another.
    if(instance->mf_ultralight_aes_dict_context.dict) {
        keys_dict_free(instance->mf_ultralight_aes_dict_context.dict);
        instance->mf_ultralight_aes_dict_context.dict = NULL;
    }

    if(state == DictAttackStateUserDictInProgress) {
        do {
            if(!keys_dict_check_presence(NFC_APP_MF_ULTRALIGHT_AES_DICT_USER_PATH)) {
                state = DictAttackStateSystemDictInProgress;
                break;
            }
            instance->mf_ultralight_aes_dict_context.dict = keys_dict_alloc(
                NFC_APP_MF_ULTRALIGHT_AES_DICT_USER_PATH,
                KeysDictModeOpenAlways,
                sizeof(MfUltralightAesKey));
            if(keys_dict_get_total_keys(instance->mf_ultralight_aes_dict_context.dict) == 0) {
                keys_dict_free(instance->mf_ultralight_aes_dict_context.dict);
                instance->mf_ultralight_aes_dict_context.dict = NULL;
                state = DictAttackStateSystemDictInProgress;
                break;
            }
            dict_attack_set_header(instance->dict_attack, "MFUL AES User Dictionary");
        } while(false);
    }
    if(state == DictAttackStateSystemDictInProgress) {
        instance->mf_ultralight_aes_dict_context.dict = keys_dict_alloc(
            NFC_APP_MF_ULTRALIGHT_AES_DICT_SYSTEM_PATH,
            KeysDictModeOpenExisting,
            sizeof(MfUltralightAesKey));
        dict_attack_set_header(instance->dict_attack, "MFUL AES System Dictionary");
    }

    instance->mf_ultralight_aes_dict_context.dict_keys_total =
        keys_dict_get_total_keys(instance->mf_ultralight_aes_dict_context.dict);
    dict_attack_set_total_dict_keys(
        instance->dict_attack, instance->mf_ultralight_aes_dict_context.dict_keys_total);
    instance->mf_ultralight_aes_dict_context.dict_keys_current = 0;
    dict_attack_set_current_dict_key(
        instance->dict_attack, instance->mf_ultralight_aes_dict_context.dict_keys_current);

    dict_attack_set_key_found(instance->dict_attack, false);
    dict_attack_set_pages_total(
        instance->dict_attack, mf_ultralight_get_pages_total(MfUltralightTypeUltralightAES));
    dict_attack_set_pages_read(instance->dict_attack, 0);

    dict_attack_set_callback(
        instance->dict_attack, nfc_scene_mf_ultralight_aes_dict_attack_result_callback, instance);
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfUltralightAesDictAttack, state);
}

void nfc_scene_mf_ultralight_aes_dict_attack_on_enter(void* context) {
    NfcApp* instance = context;

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightAesDictAttack,
        DictAttackStateUserDictInProgress);
    nfc_scene_mf_ultralight_aes_dict_attack_prepare_view(instance);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(
        instance->poller, nfc_mf_ultralight_aes_dict_attack_worker_callback, instance);

    dict_attack_set_card_state(instance->dict_attack, true);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);
}

// Tear down the user-dict phase and restart the poller against the system dictionary. The dict
// handle is freed by prepare_view's guard (poller is already stopped), so it isn't freed here.
static void nfc_scene_mf_ultralight_aes_dict_attack_restart_system(NfcApp* instance) {
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);
    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightAesDictAttack,
        DictAttackStateSystemDictInProgress);
    nfc_scene_mf_ultralight_aes_dict_attack_prepare_view(instance);
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(
        instance->poller, nfc_mf_ultralight_aes_dict_attack_worker_callback, instance);
}

static void nfc_scene_mf_ultralight_aes_dict_attack_finish(NfcApp* instance, bool success) {
    notification_message(
        instance->notifications, success ? &sequence_success : &sequence_semi_success);
    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
    dolphin_deed(DolphinDeedNfcReadSuccess);
}

bool nfc_scene_mf_ultralight_aes_dict_attack_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightAesDictAttack);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventDictAttackComplete) {
            bool auth_success = instance->mf_ultralight_aes_dict_context.auth_success;
            if(state == DictAttackStateUserDictInProgress && !auth_success) {
                // User dict exhausted without success: fall through to the system dictionary.
                nfc_scene_mf_ultralight_aes_dict_attack_restart_system(instance);
            } else {
                nfc_scene_mf_ultralight_aes_dict_attack_finish(instance, auth_success);
            }
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackDataUpdate) {
            dict_attack_set_current_dict_key(
                instance->dict_attack, instance->mf_ultralight_aes_dict_context.dict_keys_current);
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackSkip) {
            if(state == DictAttackStateUserDictInProgress) {
                nfc_scene_mf_ultralight_aes_dict_attack_restart_system(instance);
            } else {
                nfc_scene_mf_ultralight_aes_dict_attack_finish(instance, false);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_mf_ultralight_aes_dict_attack_on_exit(void* context) {
    NfcApp* instance = context;
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);
    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightAesDictAttack,
        DictAttackStateUserDictInProgress);
    keys_dict_free(instance->mf_ultralight_aes_dict_context.dict);
    instance->mf_ultralight_aes_dict_context.dict = NULL;
    instance->mf_ultralight_aes_dict_context.dict_keys_total = 0;
    instance->mf_ultralight_aes_dict_context.dict_keys_current = 0;
    instance->mf_ultralight_aes_dict_context.auth_success = false;
    nfc_blink_stop(instance);
}
