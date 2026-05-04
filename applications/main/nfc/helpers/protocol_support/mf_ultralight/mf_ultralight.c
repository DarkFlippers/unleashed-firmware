#include "mf_ultralight.h"
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
        if(instance->mf_ul_auth->type == MfUltralightAuthTypeXiaomi) {
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
            if(data->type == MfUltralightTypeMfulC &&
               instance->mf_ul_auth->type == MfUltralightAuthTypeNone) {
                // Start dict attack for MFUL C cards only if no specific auth was attempted
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightCDictAttack);
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
        data->type != MfUltralightTypeMfulC)) {
        submenu_remove_item(submenu, SubmenuIndexCommonWrite);
    } else if(data->type == MfUltralightTypeMfulC) {
        // Replace the generic Write item with two ULC-specific options so the user
        // can choose whether to keep or overwrite the target card's 3DES key.
        // This avoids any mid-write dialog/view-switching complexity entirely.
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
        submenu_add_item(
            submenu,
            "Unlock",
            SubmenuIndexUnlock,
            nfc_protocol_support_common_submenu_callback,
            instance);
        if(data->type == MfUltralightTypeMfulC) {
            submenu_add_item(
                submenu,
                "Unlock with Dictionary",
                SubmenuIndexDictAttack,
                nfc_protocol_support_common_submenu_callback,
                instance);
        }
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
    }

    mf_ultralight_auth_reset(instance->mf_ul_auth);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_emulate_on_enter_mf_ultralight(NfcApp* instance) {
    const MfUltralightData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
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

            uint32_t next_scene = (data->type == MfUltralightTypeMfulC) ?
                                      NfcSceneDesAuthKeyInput :
                                      NfcSceneMfUltralightUnlockMenu;
            scene_manager_next_scene(instance->scene_manager, next_scene);
            consumed = true;
        } else if(event.event == SubmenuIndexDictAttack) {
            if(!scene_manager_search_and_switch_to_previous_scene(
                   instance->scene_manager, NfcSceneMfUltralightCDictAttack)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightCDictAttack);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexWriteKeepKey) {
            instance->mf_ultralight_c_write_context.copy_key = false;
            scene_manager_next_scene(instance->scene_manager, NfcSceneWrite);
            consumed = true;
        } else if(event.event == SubmenuIndexWriteCopyKey) {
            instance->mf_ultralight_c_write_context.copy_key = true;
            scene_manager_next_scene(instance->scene_manager, NfcSceneWrite);
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
        // Dict attack key provider - user dict first, then system dict
        if(!instance->mf_ultralight_c_dict_context.dict &&
           instance->mf_ultralight_c_write_context.dict_state == NfcMfUltralightCWriteDictIdle) {
            if(keys_dict_check_presence(NFC_APP_MF_ULTRALIGHT_C_DICT_USER_PATH)) {
                instance->mf_ultralight_c_dict_context.dict = keys_dict_alloc(
                    NFC_APP_MF_ULTRALIGHT_C_DICT_USER_PATH,
                    KeysDictModeOpenExisting,
                    sizeof(MfUltralightC3DesAuthKey));
                instance->mf_ultralight_c_write_context.dict_state = NfcMfUltralightCWriteDictUser;
            }
            if(!instance->mf_ultralight_c_dict_context.dict) {
                instance->mf_ultralight_c_dict_context.dict = keys_dict_alloc(
                    NFC_APP_MF_ULTRALIGHT_C_DICT_SYSTEM_PATH,
                    KeysDictModeOpenExisting,
                    sizeof(MfUltralightC3DesAuthKey));
                instance->mf_ultralight_c_write_context.dict_state =
                    NfcMfUltralightCWriteDictSystem;
            }
        }
        MfUltralightC3DesAuthKey key = {};
        bool got_key = false;
        if(instance->mf_ultralight_c_dict_context.dict) {
            got_key = keys_dict_get_next_key(
                instance->mf_ultralight_c_dict_context.dict,
                key.data,
                sizeof(MfUltralightC3DesAuthKey));
        }
        if(!got_key &&
           instance->mf_ultralight_c_write_context.dict_state == NfcMfUltralightCWriteDictUser) {
            // Exhausted user dict, switch to system dict
            if(instance->mf_ultralight_c_dict_context.dict) {
                keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
            }
            instance->mf_ultralight_c_dict_context.dict = keys_dict_alloc(
                NFC_APP_MF_ULTRALIGHT_C_DICT_SYSTEM_PATH,
                KeysDictModeOpenExisting,
                sizeof(MfUltralightC3DesAuthKey));
            instance->mf_ultralight_c_write_context.dict_state = NfcMfUltralightCWriteDictSystem;
            if(instance->mf_ultralight_c_dict_context.dict) {
                got_key = keys_dict_get_next_key(
                    instance->mf_ultralight_c_dict_context.dict,
                    key.data,
                    sizeof(MfUltralightC3DesAuthKey));
            }
        }
        if(got_key) {
            mf_ultralight_event->data->key_request_data.key = key;
            mf_ultralight_event->data->key_request_data.key_provided = true;
            FURI_LOG_D(
                "MfULC",
                "Trying dict key: "
                "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                key.data[0],
                key.data[1],
                key.data[2],
                key.data[3],
                key.data[4],
                key.data[5],
                key.data[6],
                key.data[7],
                key.data[8],
                key.data[9],
                key.data[10],
                key.data[11],
                key.data[12],
                key.data[13],
                key.data[14],
                key.data[15]);
        } else {
            mf_ultralight_event->data->key_request_data.key_provided = false;
            FURI_LOG_D("MfULC", "Dict exhausted - no more keys");
            if(instance->mf_ultralight_c_dict_context.dict) {
                keys_dict_free(instance->mf_ultralight_c_dict_context.dict);
                instance->mf_ultralight_c_dict_context.dict = NULL;
            }
            instance->mf_ultralight_c_write_context.dict_state =
                NfcMfUltralightCWriteDictExhausted;
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

const NfcProtocolSupportBase nfc_protocol_support_mf_ultralight = {
    .features = NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo |
                NfcProtocolFeatureWrite,

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
};

NFC_PROTOCOL_SUPPORT_PLUGIN(mf_ultralight, NfcProtocolMfUltralight);
