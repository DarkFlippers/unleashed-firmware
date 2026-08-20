#include "mf_ultralight.h"
#include "mf_ultralight_extra_scenes.h"
#include "mf_ultralight_render.h"

#include <nfc/protocols/mf_ultralight/mf_ultralight_poller.h>
#include <toolbox/pretty_format.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../nfc_protocol_support_unlock_helper.h"

enum {
    SubmenuIndexUnlock = SubmenuIndexCommonMax,
    SubmenuIndexUnlockByReader,
    SubmenuIndexUnlockByPassword,
    SubmenuIndexDictAttack,
    SubmenuIndexWriteKeepKey, // ULC: write data pages, keep target card's existing key
    SubmenuIndexWriteCopyKey, // ULC: write all pages including key from source card
    SubmenuIndexRevealUid, // UL-AES Random ID: reveal the hidden real UID (all-zero UIDRetrKey)
};

enum {
    NfcSceneMoreInfoStateASCII,
    NfcSceneMoreInfoStateRawData,
};

static void nfc_scene_info_on_enter_mf_ultralight(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfUltralightData* data = nfc_device_get_data(device, NfcProtocolMfUltralight);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);

    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");

    nfc_render_mf_ultralight_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_more_info_on_enter_mf_ultralight(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfUltralightData* mfu = nfc_device_get_data(device, NfcProtocolMfUltralight);

    furi_string_reset(instance->text_box_store);
    uint32_t scene_state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMoreInfo);

    if(scene_state == NfcSceneMoreInfoStateASCII) {
        pretty_format_bytes_hex_canonical(
            instance->text_box_store,
            MF_ULTRALIGHT_PAGE_SIZE,
            PRETTY_FORMAT_FONT_MONOSPACE,
            (uint8_t*)mfu->page,
            mfu->pages_read * MF_ULTRALIGHT_PAGE_SIZE);

        widget_add_text_scroll_element(
            instance->widget, 0, 0, 128, 48, furi_string_get_cstr(instance->text_box_store));
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "Raw Data",
            nfc_protocol_support_common_widget_callback,
            instance);

        widget_add_button_element(
            instance->widget,
            GuiButtonTypeLeft,
            "Info",
            nfc_protocol_support_common_widget_callback,
            instance);
    } else if(scene_state == NfcSceneMoreInfoStateRawData) {
        nfc_render_mf_ultralight_dump(mfu, instance->text_box_store);
        widget_add_text_scroll_element(
            instance->widget, 0, 0, 128, 48, furi_string_get_cstr(instance->text_box_store));

        widget_add_button_element(
            instance->widget,
            GuiButtonTypeLeft,
            "ASCII",
            nfc_protocol_support_common_widget_callback,
            instance);
    }
}

static bool nfc_scene_more_info_on_event_mf_ultralight(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if((event.type == SceneManagerEventTypeCustom && event.event == GuiButtonTypeLeft) ||
       (event.type == SceneManagerEventTypeBack)) {
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneMoreInfo, NfcSceneMoreInfoStateASCII);
        scene_manager_previous_scene(instance->scene_manager);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom && event.event == GuiButtonTypeRight) {
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneMoreInfo, NfcSceneMoreInfoStateRawData);
        scene_manager_next_scene(instance->scene_manager, NfcSceneMoreInfo);
        consumed = true;
    }
    return consumed;
}

static NfcCommand
    nfc_scene_read_poller_callback_mf_ultralight(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfUltralight);

    NfcApp* instance = context;
    const MfUltralightPollerEvent* mf_ultralight_event = event.event_data;

    if(mf_ultralight_event->type == MfUltralightPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfUltralight, nfc_poller_get_data(instance->poller));

        const MfUltralightData* data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
        uint32_t event = (data->pages_read == data->pages_total) ? NfcCustomEventPollerSuccess :
                                                                   NfcCustomEventPollerIncomplete;
        view_dispatcher_send_custom_event(instance->view_dispatcher, event);
        return NfcCommandStop;
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeAuthRequest) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfUltralight, nfc_poller_get_data(instance->poller));
        const MfUltralightData* data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
        if(data->type == MfUltralightTypeUltralightAES) {
            // UL-AES auth is AES, not password, and it has an AUTHLIM - a failed auth is counted and
            // can permanently lock the card. So NEVER authenticate automatically on a plain read;
            // only when the user explicitly asked for it:
            //   - Manual key entry -> try their DataProtKey.
            //   - "Reveal Real UID" (Random ID cards) -> try the default all-zero UIDRetrKey to
            //     reveal the hidden static UID (kept in pages 0-1, shown as "Real UID" in config).
            //     A non-default UIDRetrKey fails and keeps showing the random UID.
            if(instance->mf_ul_auth->type == MfUltralightAuthTypeManual) {
                mf_ultralight_event->data->auth_context.skip_auth = false;
                mf_ultralight_event->data->auth_context.aes_key = instance->mf_ul_auth->aes_key;
                mf_ultralight_event->data->auth_context.aes_key_type = MfUltralightAesKeyTypeData;
            } else if(instance->mf_ul_auth->type == MfUltralightAuthTypeUidReveal) {
                const MfUltralightAesKey uid_key = {0};
                mf_ultralight_event->data->auth_context.skip_auth = false;
                mf_ultralight_event->data->auth_context.aes_key = uid_key;
                mf_ultralight_event->data->auth_context.aes_key_type = MfUltralightAesKeyTypeUid;
            } else {
                mf_ultralight_event->data->auth_context.skip_auth = true;
            }
        } else if(instance->mf_ul_auth->type == MfUltralightAuthTypeXiaomi) {
            if(mf_ultralight_generate_xiaomi_pass(
                   instance->mf_ul_auth,
                   data->iso14443_3a_data->uid,
                   data->iso14443_3a_data->uid_len)) {
                mf_ultralight_event->data->auth_context.skip_auth = false;
            }
        } else if(instance->mf_ul_auth->type == MfUltralightAuthTypeAmiibo) {
            if(mf_ultralight_generate_amiibo_pass(
                   instance->mf_ul_auth,
                   data->iso14443_3a_data->uid,
                   data->iso14443_3a_data->uid_len)) {
                mf_ultralight_event->data->auth_context.skip_auth = false;
            }
        } else if(
            instance->mf_ul_auth->type == MfUltralightAuthTypeManual ||
            instance->mf_ul_auth->type == MfUltralightAuthTypeReader) {
            mf_ultralight_event->data->auth_context.skip_auth = false;
        } else {
            mf_ultralight_event->data->auth_context.skip_auth = true;
        }
        if(!mf_ultralight_event->data->auth_context.skip_auth) {
            mf_ultralight_event->data->auth_context.password = instance->mf_ul_auth->password;

            if(data->type == MfUltralightTypeMfulC) {
                // Only set tdes_key for Manual/Reader auth types, not for dictionary attacks
                if(instance->mf_ul_auth->type == MfUltralightAuthTypeManual ||
                   instance->mf_ul_auth->type == MfUltralightAuthTypeReader) {
                    mf_ultralight_event->data->key_request_data.key =
                        instance->mf_ul_auth->tdes_key;
                    mf_ultralight_event->data->key_request_data.key_provided = true;
                } else {
                    mf_ultralight_event->data->key_request_data.key_provided = false;
                }
            }
        }
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeAuthSuccess) {
        instance->mf_ul_auth->pack = mf_ultralight_event->data->auth_context.pack;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_mf_ultralight(NfcApp* instance) {
    nfc_unlock_helper_setup_from_state(instance);
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_mf_ultralight, instance);
}

// UL-AES uses the AES dictionary-attack scene; other auth-capable UL types use the 3DES one.
static uint32_t nfc_mf_ultralight_dict_attack_scene(MfUltralightType type) {
    return (type == MfUltralightTypeUltralightAES) ? NfcSceneMfUltralightAesDictAttack :
                                                     NfcSceneMfUltralightCDictAttack;
}

// Show the UL-AES auth warning, continuing to next_scene if the user accepts
static void nfc_mf_ultralight_aes_warn(NfcApp* instance, uint32_t next_scene) {
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfUltralightAesDictAttackWarn, next_scene);
    scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightAesDictAttackWarn);
}

// Start a write. Writing to a protected UL-AES target dictionary-attacks it for the write key, which
// burns AUTHLIM attempts, so warn first; UL-C and the rest go straight to the write.
static void nfc_mf_ultralight_write_confirm(NfcApp* instance) {
    const MfUltralightData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
    if(data->type == MfUltralightTypeUltralightAES) {
        nfc_mf_ultralight_aes_warn(instance, NfcSceneWrite);
    } else {
        scene_manager_next_scene(instance->scene_manager, NfcSceneWrite);
    }
}

bool nfc_scene_read_on_event_mf_ultralight(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventPollerSuccess) {
            notification_message(instance->notifications, &sequence_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
            dolphin_deed(DolphinDeedNfcReadSuccess);
            return true;
        } else if(event.event == NfcCustomEventPollerIncomplete) {
            const MfUltralightData* data =
                nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
            if(instance->mf_ul_auth->type == MfUltralightAuthTypeNone &&
               data->type == MfUltralightTypeMfulC) {
                // UL-C only: it has no AUTHLIM, so auto attacking is safe. UL-AES is not
                // auto attacked, a failed run can lock the card - user starts it from the menu
                scene_manager_next_scene(
                    instance->scene_manager, nfc_mf_ultralight_dict_attack_scene(data->type));
            } else {
                if(data->pages_read == data->pages_total) {
                    notification_message(instance->notifications, &sequence_success);
                } else {
                    notification_message(instance->notifications, &sequence_semi_success);
                }
                scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
                dolphin_deed(DolphinDeedNfcReadSuccess);
            }
            return true;
        }
    }
    return false;
}

static void nfc_scene_read_and_saved_menu_on_enter_mf_ultralight(NfcApp* instance) {
    Submenu* submenu = instance->submenu;

    const MfUltralightData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
    bool is_locked = !mf_ultralight_is_all_data_read(data);

    if(is_locked ||
       (data->type != MfUltralightTypeNTAG213 && data->type != MfUltralightTypeNTAG215 &&
        data->type != MfUltralightTypeNTAG216 && data->type != MfUltralightTypeUL11 &&
        data->type != MfUltralightTypeUL21 && data->type != MfUltralightTypeOrigin &&
        data->type != MfUltralightTypeMfulC && data->type != MfUltralightTypeUltralightAES)) {
        submenu_remove_item(submenu, SubmenuIndexCommonWrite);
    } else if(data->type == MfUltralightTypeMfulC || data->type == MfUltralightTypeUltralightAES) {
        // Replace the generic Write item with two key options so the user can choose whether to
        // keep or overwrite the target card's auth key (3DES for UL-C, DataProtKey for UL-AES).
        submenu_remove_item(submenu, SubmenuIndexCommonWrite);
        submenu_add_item(
            submenu,
            "Write (Keep Key)",
            SubmenuIndexWriteKeepKey,
            nfc_protocol_support_common_submenu_callback,
            instance);
        submenu_add_item(
            submenu,
            "Write (Copy Key)",
            SubmenuIndexWriteCopyKey,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    if(is_locked) {
        // "Unlock" enters a key/password manually (AES key for UL-AES, 3DES for UL-C, password
        // otherwise — the SubmenuIndexUnlock handler routes by type). UL-C/UL-AES also get the
        // dictionary attack.
        submenu_add_item(
            submenu,
            "Unlock",
            SubmenuIndexUnlock,
            nfc_protocol_support_common_submenu_callback,
            instance);
        if(data->type == MfUltralightTypeMfulC || data->type == MfUltralightTypeUltralightAES) {
            submenu_add_item(
                submenu,
                "Unlock with Dictionary",
                SubmenuIndexDictAttack,
                nfc_protocol_support_common_submenu_callback,
                instance);
        }
    }

    // Random ID cards (4-byte UID starting 0x08) hide the real static UID; offer an explicit reveal
    // action here (see the AuthRequest handler for why it is not done automatically).
    if(data->type == MfUltralightTypeUltralightAES && data->iso14443_3a_data->uid_len == 4 &&
       data->iso14443_3a_data->uid[0] == 0x08) {
        submenu_add_item(
            submenu,
            "Reveal Real UID",
            SubmenuIndexRevealUid,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }
}

static void nfc_scene_read_success_on_enter_mf_ultralight(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfUltralightData* data = nfc_device_get_data(device, NfcProtocolMfUltralight);

    FuriString* temp_str = furi_string_alloc();

    bool unlocked =
        scene_manager_has_previous_scene(instance->scene_manager, NfcSceneMfUltralightUnlockWarn);
    if(unlocked) {
        nfc_render_mf_ultralight_pwd_pack(data, temp_str);
    } else {
        furi_string_cat_printf(
            temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));

        furi_string_replace(temp_str, "Mifare", "MIFARE");

        nfc_render_mf_ultralight_info(data, NfcProtocolFormatTypeShort, temp_str);

        // Show captured PWD/PACK on a plain read too, matching the Unlock screen.
        nfc_render_mf_ultralight_pwd_pack_if_read(data, temp_str);
    }

    mf_ultralight_auth_reset(instance->mf_ul_auth);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_emulate_on_enter_mf_ultralight(NfcApp* instance) {
    const MfUltralightData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
    // UL-AES now emulates via the Ultralight listener too (AES auth + AUTH0-gated reads).
    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolMfUltralight, data);
    nfc_listener_start(instance->listener, NULL, NULL);
}

static bool nfc_scene_read_and_saved_menu_on_event_mf_ultralight(
    NfcApp* instance,
    SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexUnlock) {
            const MfUltralightData* data =
                nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);

            // UL-C (3DES) and UL-AES both enter a 16-byte key via the shared DesAuth key input;
            // other types use the password-based unlock menu. UL-AES's brick warning lives on the
            // DesAuthUnlockWarn confirm that follows key entry (it shows the exact key), so no extra
            // pre-entry warning here.
            uint32_t next_scene = (data->type == MfUltralightTypeMfulC ||
                                   data->type == MfUltralightTypeUltralightAES) ?
                                      NfcSceneDesAuthKeyInput :
                                      NfcSceneMfUltralightUnlockMenu;
            scene_manager_next_scene(instance->scene_manager, next_scene);
            consumed = true;
        } else if(event.event == SubmenuIndexRevealUid) {
            // Set the reveal auth type, then confirm via the warn scene before re-reading.
            instance->mf_ul_auth->type = MfUltralightAuthTypeUidReveal;
            nfc_mf_ultralight_aes_warn(instance, NfcSceneRead);
            consumed = true;
        } else if(event.event == SubmenuIndexDictAttack) {
            const MfUltralightData* data =
                nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
            if(data->type == MfUltralightTypeUltralightAES) {
                // Confirm first, a failed run can lock the card
                nfc_mf_ultralight_aes_warn(instance, NfcSceneMfUltralightAesDictAttack);
            } else {
                uint32_t dict_scene = nfc_mf_ultralight_dict_attack_scene(data->type);
                if(!scene_manager_search_and_switch_to_previous_scene(
                       instance->scene_manager, dict_scene)) {
                    scene_manager_next_scene(instance->scene_manager, dict_scene);
                }
            }
            consumed = true;
        } else if(event.event == SubmenuIndexWriteKeepKey) {
            instance->mf_ultralight_c_write_context.copy_key = false;
            nfc_mf_ultralight_write_confirm(instance);
            consumed = true;
        } else if(event.event == SubmenuIndexWriteCopyKey) {
            instance->mf_ultralight_c_write_context.copy_key = true;
            nfc_mf_ultralight_write_confirm(instance);
            consumed = true;
        }
    }
    return consumed;
}

static NfcCommand
    nfc_scene_write_poller_callback_mf_ultralight(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfUltralight);

    NfcApp* instance = context;
    MfUltralightPollerEvent* mf_ultralight_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(mf_ultralight_event->type == MfUltralightPollerEventTypeRequestMode) {
        mf_ultralight_event->data->poller_mode = MfUltralightPollerModeWrite;
        furi_string_reset(instance->text_box_store);
        if(instance->mf_ultralight_c_dict_context.dict) {
            keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
        }
        instance->mf_ultralight_c_dict_context.dict = NULL;
        instance->mf_ultralight_c_write_context.dict_state = NfcMfUltralightCWriteDictIdle;
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeAuthRequest) {
        // Skip auth during the read phase of write - we'll authenticate
        // against the target card in RequestWriteData using source key or dict attack
        mf_ultralight_event->data->auth_context.skip_auth = true;
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeRequestKey) {
        // Write-phase target-auth key provider: user dict first, then system dict. Paths and key
        // field are chosen by the source card type (3DES for UL-C, AES for UL-AES).
        const MfUltralightData* wdata =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
        const bool is_aes = (wdata->type == MfUltralightTypeUltralightAES);
        const char* user_path = is_aes ? NFC_APP_MF_ULTRALIGHT_AES_DICT_USER_PATH :
                                         NFC_APP_MF_ULTRALIGHT_C_DICT_USER_PATH;
        const char* sys_path = is_aes ? NFC_APP_MF_ULTRALIGHT_AES_DICT_SYSTEM_PATH :
                                        NFC_APP_MF_ULTRALIGHT_C_DICT_SYSTEM_PATH;
        const size_t key_size = is_aes ? sizeof(MfUltralightAesKey) :
                                         sizeof(MfUltralightC3DesAuthKey);
        NfcMfUltralightCDictContext* dctx = &instance->mf_ultralight_c_dict_context;
        NfcMfUltralightCWriteContext* wctx = &instance->mf_ultralight_c_write_context;

        if(!dctx->dict && wctx->dict_state == NfcMfUltralightCWriteDictIdle) {
            if(keys_dict_check_presence(user_path)) {
                dctx->dict = keys_dict_alloc(user_path, KeysDictModeOpenExisting, key_size);
                wctx->dict_state = NfcMfUltralightCWriteDictUser;
            }
            if(!dctx->dict) {
                dctx->dict = keys_dict_alloc(sys_path, KeysDictModeOpenExisting, key_size);
                wctx->dict_state = NfcMfUltralightCWriteDictSystem;
            }
        }

        uint8_t key_buf[MF_ULTRALIGHT_AES_KEY_SIZE] = {0};
        bool got_key = false;
        if(dctx->dict) {
            got_key = keys_dict_get_next_key(dctx->dict, key_buf, key_size);
        }
        if(!got_key && wctx->dict_state == NfcMfUltralightCWriteDictUser) {
            // Exhausted user dict, switch to system dict
            if(dctx->dict) keys_dict_free(dctx->dict);
            dctx->dict = keys_dict_alloc(sys_path, KeysDictModeOpenExisting, key_size);
            wctx->dict_state = NfcMfUltralightCWriteDictSystem;
            if(dctx->dict) got_key = keys_dict_get_next_key(dctx->dict, key_buf, key_size);
        }
        if(got_key) {
            if(is_aes) {
                memcpy(
                    mf_ultralight_event->data->key_request_data.aes_key.data, key_buf, key_size);
            } else {
                memcpy(mf_ultralight_event->data->key_request_data.key.data, key_buf, key_size);
            }
            mf_ultralight_event->data->key_request_data.key_provided = true;
        } else {
            mf_ultralight_event->data->key_request_data.key_provided = false;
            if(dctx->dict) {
                keys_dict_free(dctx->dict);
                dctx->dict = NULL;
            }
            wctx->dict_state = NfcMfUltralightCWriteDictExhausted;
        }
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeRequestWriteData) {
        mf_ultralight_event->data->write_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
        // Reset dict context so RequestKey starts fresh for the write-phase auth
        if(instance->mf_ultralight_c_dict_context.dict) {
            keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
            instance->mf_ultralight_c_dict_context.dict = NULL;
        }
        instance->mf_ultralight_c_write_context.dict_state = NfcMfUltralightCWriteDictIdle;
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeWriteKeyRequest) {
        // Apply the user's key choice - read from static, not scene state (scene manager
        // resets state to 0 on scene entry, wiping any value set before next_scene).
        bool keep_key = !instance->mf_ultralight_c_write_context.copy_key;
        mf_ultralight_event->data->write_key_skip = keep_key;

        if(mf_ultralight_event->data->key_request_data.key_provided) {
            MfUltralightC3DesAuthKey found_key = mf_ultralight_event->data->key_request_data.key;
            FURI_LOG_D(
                "MfULC",
                "WriteKeyRequest: target key = "
                "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                found_key.data[0],
                found_key.data[1],
                found_key.data[2],
                found_key.data[3],
                found_key.data[4],
                found_key.data[5],
                found_key.data[6],
                found_key.data[7],
                found_key.data[8],
                found_key.data[9],
                found_key.data[10],
                found_key.data[11],
                found_key.data[12],
                found_key.data[13],
                found_key.data[14],
                found_key.data[15]);
        }
        FURI_LOG_D(
            "MfULC",
            "WriteKeyRequest: decision = %s (copy_key=%d)",
            keep_key ? "KEEP target key (pages 44-47 NOT written)" :
                       "OVERWRITE with source key (pages 44-47 WILL be written)",
            (int)instance->mf_ultralight_c_write_context.copy_key);
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeCardMismatch) {
        furi_string_set(instance->text_box_store, "Card of the same\ntype should be\n presented");
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
        command = NfcCommandStop;
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeCardLocked) {
        furi_string_set(
            instance->text_box_store, "Card protected by\npassword, AUTH0\nor lock bits");
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeWriteFail) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
    } else if(mf_ultralight_event->type == MfUltralightPollerEventTypeWriteSuccess) {
        furi_string_reset(instance->text_box_store);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    }

    return command;
}

static void nfc_scene_write_on_enter_mf_ultralight(NfcApp* instance) {
    // Free any dict the write callback opened (dict_state != Idle means we own it).
    // After a DictAttack scene, on_exit now NULLs the pointer so a simple NULL check
    // is safe here too — but the state enum is the authoritative ownership record.
    if(instance->mf_ultralight_c_write_context.dict_state != NfcMfUltralightCWriteDictIdle &&
       instance->mf_ultralight_c_dict_context.dict) {
        keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
    }
    instance->mf_ultralight_c_dict_context.dict = NULL;
    instance->mf_ultralight_c_write_context.dict_state = NfcMfUltralightCWriteDictIdle;
    furi_string_set(instance->text_box_store, "\nApply the\ntarget\ncard now");
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(instance->poller, nfc_scene_write_poller_callback_mf_ultralight, instance);
}

#define MF_ULTRALIGHT_DEFAULT_FEATURES \
    (NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo | NfcProtocolFeatureWrite)

const NfcProtocolSupportBase nfc_protocol_support_mf_ultralight = {
    .features = MF_ULTRALIGHT_DEFAULT_FEATURES,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_mf_ultralight,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_mf_ultralight,
            .on_event = nfc_scene_more_info_on_event_mf_ultralight,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_mf_ultralight,
            .on_event = nfc_scene_read_on_event_mf_ultralight,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_scene_read_and_saved_menu_on_enter_mf_ultralight,
            .on_event = nfc_scene_read_and_saved_menu_on_event_mf_ultralight,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_mf_ultralight,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_scene_read_and_saved_menu_on_enter_mf_ultralight,
            .on_event = nfc_scene_read_and_saved_menu_on_event_mf_ultralight,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_emulate =
        {
            .on_enter = nfc_scene_emulate_on_enter_mf_ultralight,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_scene_write_on_enter_mf_ultralight,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },

    .extra_scenes = mf_ultralight_extra_scenes,
    .extra_scenes_count = MfUltralightExtraSceneNum,
};

NFC_PROTOCOL_SUPPORT_PLUGIN(mf_ultralight, NfcProtocolMfUltralight);
