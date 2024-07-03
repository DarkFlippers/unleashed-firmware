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
    SubmenuIndexWrite,
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
            mf_ultralight_event->data->auth_context.tdes_key = instance->mf_ul_auth->tdes_key;
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
        if(event.event == NfcCustomEventCardDetected) {
            nfc_unlock_helper_card_detected_handler(instance);
        } else if(event.event == NfcCustomEventPollerIncomplete) {
            notification_message(instance->notifications, &sequence_semi_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
            dolphin_deed(DolphinDeedNfcReadSuccess);
        }
    }
    return true;
}

static void nfc_scene_read_and_saved_menu_on_enter_mf_ultralight(NfcApp* instance) {
    Submenu* submenu = instance->submenu;

    const MfUltralightData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);

    if(!mf_ultralight_is_all_data_read(data)) {
        submenu_add_item(
            submenu,
            "Unlock",
            SubmenuIndexUnlock,
            nfc_protocol_support_common_submenu_callback,
            instance);
    } else if(
        data->type == MfUltralightTypeNTAG213 || data->type == MfUltralightTypeNTAG215 ||
        data->type == MfUltralightTypeNTAG216 || data->type == MfUltralightTypeUL11 ||
        data->type == MfUltralightTypeUL21 || data->type == MfUltralightTypeOrigin) {
        submenu_add_item(
            submenu,
            "Write",
            SubmenuIndexWrite,
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
        } else if(event.event == SubmenuIndexWrite) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightWrite);
            consumed = true;
        } else if(event.event == SubmenuIndexCommonEdit) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneSetUid);
            consumed = true;
        }
    }
    return consumed;
}

const NfcProtocolSupportBase nfc_protocol_support_mf_ultralight = {
    .features = NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo,

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
};
