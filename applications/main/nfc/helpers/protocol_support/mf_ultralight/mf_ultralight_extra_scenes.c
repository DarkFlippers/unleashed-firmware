#include "mf_ultralight_extra_scenes.h"

#include "nfc/nfc_app_i.h"
#include "../nfc_protocol_support_gui_common.h"
#include <dolphin/dolphin.h>

// Shared by both dictionary-attack scenes below.
enum {
    DictAttackStateUserDictInProgress,
    DictAttackStateSystemDictInProgress,
};

// ---- c_dict_attack -----------------------------------------------------
#undef TAG
#define TAG "NfcMfUlCDictAttack"

// TODO: Support card_detected properly

static NfcCommand
    nfc_mf_ultralight_c_dict_attack_worker_callback(NfcGenericEvent event, void* context) {
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

static void mf_ultralight_scene_c_dict_attack_dict_attack_result_callback(
    DictAttackEvent event,
    void* context) {
    furi_assert(context);
    NfcApp* instance = context;
    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

static void mf_ultralight_scene_c_dict_attack_prepare_view(NfcApp* instance) {
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightCDictAttack);

    // Set attack type to Ultralight C
    dict_attack_set_type(instance->dict_attack, DictAttackTypeMfUltralightC);

    // Guard: if a previous write phase left a dict handle open, close it now.
    // Without this, navigating write->back->read->dict-attack would open the same
    // file twice, corrupting VFS state and causing a ViewPort lockup.
    if(instance->mf_ultralight_c_dict_context.dict) {
        keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
        instance->mf_ultralight_c_dict_context.dict = NULL;
        instance->mf_ultralight_c_write_context.dict_state = NfcMfUltralightCWriteDictIdle;
    }

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
        mf_ultralight_scene_c_dict_attack_dict_attack_result_callback,
        instance);
    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfUltralightCDictAttack, state);
}

static void mf_ultralight_scene_c_dict_attack_on_enter(NfcApp* instance) {
    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightCDictAttack,
        DictAttackStateUserDictInProgress);
    mf_ultralight_scene_c_dict_attack_prepare_view(instance);

    // Setup and start worker
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(instance->poller, nfc_mf_ultralight_c_dict_attack_worker_callback, instance);

    dict_attack_set_card_state(instance->dict_attack, true);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);
}

static bool mf_ultralight_scene_c_dict_attack_on_event(NfcApp* instance, SceneManagerEvent event) {
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
                    instance->mf_ultralight_c_dict_context.dict = NULL;
                    scene_manager_set_scene_state(
                        instance->scene_manager,
                        NfcSceneMfUltralightCDictAttack,
                        DictAttackStateSystemDictInProgress);
                    mf_ultralight_scene_c_dict_attack_prepare_view(instance);
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
                instance->mf_ultralight_c_dict_context.dict = NULL;
                scene_manager_set_scene_state(
                    instance->scene_manager,
                    NfcSceneMfUltralightCDictAttack,
                    DictAttackStateSystemDictInProgress);
                mf_ultralight_scene_c_dict_attack_prepare_view(instance);
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

static void mf_ultralight_scene_c_dict_attack_on_exit(NfcApp* instance) {
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);
    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightCDictAttack,
        DictAttackStateUserDictInProgress);
    keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
    instance->mf_ultralight_c_dict_context.dict = NULL;
    instance->mf_ultralight_c_dict_context.dict_keys_total = 0;
    instance->mf_ultralight_c_dict_context.dict_keys_current = 0;
    instance->mf_ultralight_c_dict_context.auth_success = false;
    instance->mf_ultralight_c_dict_context.is_card_present = false;
    nfc_blink_stop(instance);
}

// ---- aes_dict_attack ---------------------------------------------------
#undef TAG
#define TAG "NfcMfUlAesDictAttack"

// Mirrors the Ultralight-C dictionary attack, but feeds 16-byte AES-128 keys to the poller's
// AES 3-pass auth. The poller tries one key per activation (a wrong key unselects the card), so
// the scene just streams keys from the user dict, then the system dict.

static NfcCommand
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
    mf_ultralight_scene_aes_dict_attack_result_callback(DictAttackEvent event, void* context) {
    furi_assert(context);
    NfcApp* instance = context;
    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

static void mf_ultralight_scene_aes_dict_attack_prepare_view(NfcApp* instance) {
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
        instance->dict_attack, mf_ultralight_scene_aes_dict_attack_result_callback, instance);
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfUltralightAesDictAttack, state);
}

static void mf_ultralight_scene_aes_dict_attack_on_enter(NfcApp* instance) {
    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightAesDictAttack,
        DictAttackStateUserDictInProgress);
    mf_ultralight_scene_aes_dict_attack_prepare_view(instance);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(
        instance->poller, nfc_mf_ultralight_aes_dict_attack_worker_callback, instance);

    dict_attack_set_card_state(instance->dict_attack, true);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);
}

// Tear down the user-dict phase and restart the poller against the system dictionary. The dict
// handle is freed by prepare_view's guard (poller is already stopped), so it isn't freed here.
static void mf_ultralight_scene_aes_dict_attack_restart_system(NfcApp* instance) {
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);
    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightAesDictAttack,
        DictAttackStateSystemDictInProgress);
    mf_ultralight_scene_aes_dict_attack_prepare_view(instance);
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(
        instance->poller, nfc_mf_ultralight_aes_dict_attack_worker_callback, instance);
}

static void mf_ultralight_scene_aes_dict_attack_finish(NfcApp* instance, bool success) {
    notification_message(
        instance->notifications, success ? &sequence_success : &sequence_semi_success);
    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
    dolphin_deed(DolphinDeedNfcReadSuccess);
}

static bool
    mf_ultralight_scene_aes_dict_attack_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightAesDictAttack);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventDictAttackComplete) {
            bool auth_success = instance->mf_ultralight_aes_dict_context.auth_success;
            if(state == DictAttackStateUserDictInProgress && !auth_success) {
                // User dict exhausted without success: fall through to the system dictionary.
                mf_ultralight_scene_aes_dict_attack_restart_system(instance);
            } else {
                mf_ultralight_scene_aes_dict_attack_finish(instance, auth_success);
            }
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackDataUpdate) {
            dict_attack_set_current_dict_key(
                instance->dict_attack, instance->mf_ultralight_aes_dict_context.dict_keys_current);
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackSkip) {
            if(state == DictAttackStateUserDictInProgress) {
                mf_ultralight_scene_aes_dict_attack_restart_system(instance);
            } else {
                mf_ultralight_scene_aes_dict_attack_finish(instance, false);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

static void mf_ultralight_scene_aes_dict_attack_on_exit(NfcApp* instance) {
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

// ---- unlock_menu -------------------------------------------------------
#undef TAG
enum SubmenuIndex {
    SubmenuIndexMfUlUnlockMenuReader,
    SubmenuIndexMfUlUnlockMenuAmeebo,
    SubmenuIndexMfUlUnlockMenuXiaomi,
    SubmenuIndexMfUlUnlockMenuManual,
};

static void mf_ultralight_scene_unlock_menu_submenu_callback(void* context, uint32_t index) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

static void mf_ultralight_scene_unlock_menu_on_enter(NfcApp* nfc) {
    Submenu* submenu = nfc->submenu;

    uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
    if(nfc_device_get_protocol(nfc->nfc_device) == NfcProtocolMfUltralight) {
        const MfUltralightData* mfu_data =
            nfc_device_get_data(nfc->nfc_device, NfcProtocolMfUltralight);
        // Hide for MFU-C since it uses 3DES
        if(mfu_data->type != MfUltralightTypeMfulC) {
            submenu_add_item(
                submenu,
                "Unlock With Reader",
                SubmenuIndexMfUlUnlockMenuReader,
                mf_ultralight_scene_unlock_menu_submenu_callback,
                nfc);
        }
    }
    submenu_add_item(
        submenu,
        "Auth As Ameebo",
        SubmenuIndexMfUlUnlockMenuAmeebo,
        mf_ultralight_scene_unlock_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Auth As Xiaomi Air Purifier",
        SubmenuIndexMfUlUnlockMenuXiaomi,
        mf_ultralight_scene_unlock_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Enter Password Manually",
        SubmenuIndexMfUlUnlockMenuManual,
        mf_ultralight_scene_unlock_menu_submenu_callback,
        nfc);
    submenu_set_selected_item(submenu, state);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

static bool mf_ultralight_scene_unlock_menu_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexMfUlUnlockMenuManual) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeManual;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightKeyInput);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuAmeebo) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeAmiibo;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuXiaomi) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeXiaomi;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuReader) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeReader;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightCapturePass);
            consumed = true;
        }
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneMfUltralightUnlockMenu, event.event);
    }
    return consumed;
}

static void mf_ultralight_scene_unlock_menu_on_exit(NfcApp* nfc) {
    submenu_reset(nfc->submenu);
}

// ---- unlock_warn -------------------------------------------------------
#undef TAG
static void mf_ultralight_scene_unlock_warn_dialog_callback(DialogExResult result, void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

static void mf_ultralight_scene_unlock_warn_on_enter(NfcApp* nfc) {
    DialogEx* dialog_ex = nfc->dialog_ex;

    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, mf_ultralight_scene_unlock_warn_dialog_callback);

    MfUltralightAuthType type = nfc->mf_ul_auth->type;
    if((type == MfUltralightAuthTypeReader) || (type == MfUltralightAuthTypeManual)) {
        // Build dialog text
        FuriString* password_str =
            furi_string_alloc_set_str("Try to unlock the card with\npassword: ");
        for(size_t i = 0; i < sizeof(nfc->mf_ul_auth->password.data); i++) {
            furi_string_cat_printf(password_str, "%02X ", nfc->mf_ul_auth->password.data[i]);
        }
        furi_string_cat_str(password_str, "\nWarning: incorrect password\nwill block the card!");
        nfc_text_store_set(nfc, furi_string_get_cstr(password_str));
        furi_string_free(password_str);

        const char* message = (type == MfUltralightAuthTypeReader) ? "Password Captured!" :
                                                                     "Risky Action!";
        dialog_ex_set_header(dialog_ex, message, 64, 0, AlignCenter, AlignTop);
        dialog_ex_set_text(dialog_ex, nfc->text_store, 64, 10, AlignCenter, AlignTop);
        dialog_ex_set_left_button_text(dialog_ex, "Cancel");
        dialog_ex_set_right_button_text(dialog_ex, "Continue");

        if(type == MfUltralightAuthTypeReader) {
            notification_message(nfc->notifications, &sequence_set_green_255);
        }
    } else {
        dialog_ex_set_header(dialog_ex, "Risky action!", 64, 4, AlignCenter, AlignTop);
        dialog_ex_set_text(
            dialog_ex, "Wrong password\ncan block your\ncard.", 4, 18, AlignLeft, AlignTop);
        dialog_ex_set_icon(dialog_ex, 83, 22, &I_WarningDolphinFlip_45x42);
        dialog_ex_set_center_button_text(dialog_ex, "OK");
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

static bool mf_ultralight_scene_unlock_warn_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    MfUltralightAuthType type = nfc->mf_ul_auth->type;
    if((type == MfUltralightAuthTypeReader) || (type == MfUltralightAuthTypeManual)) {
        if(event.type == SceneManagerEventTypeCustom) {
            if(event.event == DialogExResultRight) {
                const NfcProtocol mfu_protocol[] = {NfcProtocolMfUltralight};
                nfc_detected_protocols_set(
                    nfc->detected_protocols, mfu_protocol, COUNT_OF(mfu_protocol));
                scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
                dolphin_deed(DolphinDeedNfcRead);
                consumed = true;
            } else if(event.event == DialogExResultLeft) {
                if(type == MfUltralightAuthTypeReader) {
                    consumed = scene_manager_search_and_switch_to_previous_scene(
                        nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
                } else {
                    consumed = scene_manager_previous_scene(nfc->scene_manager);
                }
            }
        } else if(event.type == SceneManagerEventTypeBack) {
            // Cannot press back
            consumed = true;
        }
    } else {
        if(event.type == SceneManagerEventTypeCustom) {
            if(event.event == DialogExResultCenter) {
                const NfcProtocol mfu_protocol[] = {NfcProtocolMfUltralight};
                nfc_detected_protocols_set(
                    nfc->detected_protocols, mfu_protocol, COUNT_OF(mfu_protocol));
                scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
                dolphin_deed(DolphinDeedNfcRead);
                consumed = true;
            }
        }
    }

    return consumed;
}

static void mf_ultralight_scene_unlock_warn_on_exit(NfcApp* nfc) {
    dialog_ex_reset(nfc->dialog_ex);
    nfc_text_store_clear(nfc);

    notification_message_block(nfc->notifications, &sequence_reset_green);
}

// ---- key_input ---------------------------------------------------------
#undef TAG
static void mf_ultralight_scene_key_input_byte_input_callback(void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventByteInputDone);
}

static void mf_ultralight_scene_key_input_on_enter(NfcApp* nfc) {
    // Setup view
    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter the password in hex");
    byte_input_set_result_callback(
        byte_input,
        mf_ultralight_scene_key_input_byte_input_callback,
        NULL,
        nfc,
        nfc->mf_ul_auth->password.data,
        4);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewByteInput);
}

static bool mf_ultralight_scene_key_input_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        }
    }
    return consumed;
}

static void mf_ultralight_scene_key_input_on_exit(NfcApp* nfc) {
    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}

// ---- capture_pass ------------------------------------------------------
#undef TAG
static NfcCommand
    mf_ultralight_scene_capture_pass_worker_callback(NfcGenericEvent event, void* context) {
    NfcApp* instance = context;
    MfUltralightListenerEvent* mfu_event = event.event_data;
    MfUltralightAuth* mauth = instance->mf_ul_auth;

    if(mfu_event->type == MfUltralightListenerEventTypeAuth) {
        mauth->password = mfu_event->data->password;
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, MfUltralightListenerEventTypeAuth);
    }

    return NfcCommandContinue;
}

static void mf_ultralight_scene_capture_pass_on_enter(NfcApp* instance) {
    // Setup view
    widget_add_string_multiline_element(
        instance->widget,
        54,
        30,
        AlignLeft,
        AlignCenter,
        FontPrimary,
        "Touch the\nreader to get\npassword...");
    widget_add_icon_element(instance->widget, 0, 15, &I_Modern_reader_18x34);
    widget_add_icon_element(instance->widget, 20, 12, &I_Move_flipper_26x39);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);

    // Start worker
    const MfUltralightData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolMfUltralight, data);
    nfc_listener_start(
        instance->listener, mf_ultralight_scene_capture_pass_worker_callback, instance);

    nfc_blink_read_start(instance);
}

static bool mf_ultralight_scene_capture_pass_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MfUltralightListenerEventTypeAuth) {
            notification_message(instance->notifications, &sequence_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        }
    }
    return consumed;
}

static void mf_ultralight_scene_capture_pass_on_exit(NfcApp* instance) {
    // Clear view
    nfc_listener_stop(instance->listener);
    nfc_listener_free(instance->listener);
    widget_reset(instance->widget);

    nfc_blink_stop(instance);
}

// ---- aes_dict_attack_warn ----------------------------------------------

// UL-AES has an AUTHLIM, failed keys can lock the card permanently

static void
    mf_ultralight_scene_aes_dict_attack_warn_dialog_callback(DialogExResult result, void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

static void mf_ultralight_scene_aes_dict_attack_warn_on_enter(NfcApp* nfc) {
    DialogEx* dialog_ex = nfc->dialog_ex;

    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(
        dialog_ex, mf_ultralight_scene_aes_dict_attack_warn_dialog_callback);

    dialog_ex_set_header(dialog_ex, "Risky action!", 64, 4, AlignCenter, AlignTop);
    dialog_ex_set_text(dialog_ex, "Wrong keys can\nblock this card", 4, 18, AlignLeft, AlignTop);
    dialog_ex_set_icon(dialog_ex, 83, 22, &I_WarningDolphinFlip_45x42);
    dialog_ex_set_left_button_text(dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(dialog_ex, "Continue");

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

static bool
    mf_ultralight_scene_aes_dict_attack_warn_on_event(NfcApp* nfc, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultRight) {
            // Caller stores where to go next: dictionary attack or manual key entry
            uint32_t next_scene = scene_manager_get_scene_state(
                nfc->scene_manager, NfcSceneMfUltralightAesDictAttackWarn);
            scene_manager_next_scene(nfc->scene_manager, next_scene);
            consumed = true;
        } else if(event.event == DialogExResultLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        }
    }

    return consumed;
}

static void mf_ultralight_scene_aes_dict_attack_warn_on_exit(NfcApp* nfc) {
    dialog_ex_reset(nfc->dialog_ex);
}

const NfcProtocolSupportExtraScene mf_ultralight_extra_scenes[MfUltralightExtraSceneNum] = {
    [MfUltralightExtraSceneCDictAttack] =
        {
            .on_enter = mf_ultralight_scene_c_dict_attack_on_enter,
            .on_event = mf_ultralight_scene_c_dict_attack_on_event,
            .on_exit = mf_ultralight_scene_c_dict_attack_on_exit,
        },
    [MfUltralightExtraSceneAesDictAttack] =
        {
            .on_enter = mf_ultralight_scene_aes_dict_attack_on_enter,
            .on_event = mf_ultralight_scene_aes_dict_attack_on_event,
            .on_exit = mf_ultralight_scene_aes_dict_attack_on_exit,
        },
    [MfUltralightExtraSceneUnlockMenu] =
        {
            .on_enter = mf_ultralight_scene_unlock_menu_on_enter,
            .on_event = mf_ultralight_scene_unlock_menu_on_event,
            .on_exit = mf_ultralight_scene_unlock_menu_on_exit,
        },
    [MfUltralightExtraSceneUnlockWarn] =
        {
            .on_enter = mf_ultralight_scene_unlock_warn_on_enter,
            .on_event = mf_ultralight_scene_unlock_warn_on_event,
            .on_exit = mf_ultralight_scene_unlock_warn_on_exit,
        },
    [MfUltralightExtraSceneKeyInput] =
        {
            .on_enter = mf_ultralight_scene_key_input_on_enter,
            .on_event = mf_ultralight_scene_key_input_on_event,
            .on_exit = mf_ultralight_scene_key_input_on_exit,
        },
    [MfUltralightExtraSceneCapturePass] =
        {
            .on_enter = mf_ultralight_scene_capture_pass_on_enter,
            .on_event = mf_ultralight_scene_capture_pass_on_event,
            .on_exit = mf_ultralight_scene_capture_pass_on_exit,
        },
    [MfUltralightExtraSceneAesDictAttackWarn] =
        {
            .on_enter = mf_ultralight_scene_aes_dict_attack_warn_on_enter,
            .on_event = mf_ultralight_scene_aes_dict_attack_warn_on_event,
            .on_exit = mf_ultralight_scene_aes_dict_attack_warn_on_exit,
        },

};
