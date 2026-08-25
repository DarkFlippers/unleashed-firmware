#include "slix_extra_scenes.h"

#include "nfc/nfc_app_i.h"
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/slix/slix_poller.h>

// ---- unlock_menu -------------------------------------------------------
enum SubmenuIndex {
    SubmenuIndexSlixUnlockMenuManual,
    SubmenuIndexSlixUnlockMenuTonieBox,
};

static void slix_scene_unlock_menu_submenu_callback(void* context, uint32_t index) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, index);
}

static void slix_scene_unlock_menu_on_enter(NfcApp* instance) {
    Submenu* submenu = instance->submenu;

    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneSlixUnlockMenu);
    submenu_add_item(
        submenu,
        "Enter Password Manually",
        SubmenuIndexSlixUnlockMenuManual,
        slix_scene_unlock_menu_submenu_callback,
        instance);
    submenu_add_item(
        submenu,
        "Auth As TommyBox",
        SubmenuIndexSlixUnlockMenuTonieBox,
        slix_scene_unlock_menu_submenu_callback,
        instance);
    submenu_set_selected_item(submenu, state);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

static bool slix_scene_unlock_menu_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSlixUnlockMenuManual) {
            slix_unlock_set_method(instance->slix_unlock, SlixUnlockMethodManual);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSlixKeyInput);
            consumed = true;
        } else if(event.event == SubmenuIndexSlixUnlockMenuTonieBox) {
            slix_unlock_set_method(instance->slix_unlock, SlixUnlockMethodTonieBox);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSlixUnlock);
            consumed = true;
        }
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneSlixUnlockMenu, event.event);
    }
    return consumed;
}

static void slix_scene_unlock_menu_on_exit(NfcApp* instance) {
    submenu_reset(instance->submenu);
}

// ---- key_input ---------------------------------------------------------
static void slix_scene_key_input_byte_input_callback(void* context) {
    NfcApp* instance = context;

    SlixPassword password =
        bit_lib_bytes_to_num_be(instance->byte_input_store, sizeof(SlixPassword));
    slix_unlock_set_password(instance->slix_unlock, password);
    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventByteInputDone);
}

static void slix_scene_key_input_on_enter(NfcApp* instance) {
    // Setup view
    ByteInput* byte_input = instance->byte_input;
    byte_input_set_header_text(byte_input, "Enter the password in hex");
    byte_input_set_result_callback(
        byte_input,
        slix_scene_key_input_byte_input_callback,
        NULL,
        instance,
        instance->byte_input_store,
        sizeof(SlixPassword));
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewByteInput);
}

static bool slix_scene_key_input_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventByteInputDone) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneSlixUnlock);
            consumed = true;
        }
    }
    return consumed;
}

static void slix_scene_key_input_on_exit(NfcApp* instance) {
    // Clear view
    byte_input_set_result_callback(instance->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(instance->byte_input, "");
}

// ---- unlock ------------------------------------------------------------
static NfcCommand slix_scene_unlock_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolSlix);

    NfcCommand command = NfcCommandContinue;

    NfcApp* instance = context;
    SlixPollerEvent* slix_event = event.event_data;
    if(slix_event->type == SlixPollerEventTypePrivacyUnlockRequest) {
        SlixPassword pwd = 0;
        bool get_password_success = slix_unlock_get_next_password(instance->slix_unlock, &pwd);
        slix_event->data->privacy_password.password = pwd;
        slix_event->data->privacy_password.password_set = get_password_success;
    } else if(slix_event->type == SlixPollerEventTypeError) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
    } else if(slix_event->type == SlixPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolSlix, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    }

    return command;
}

static void slix_scene_unlock_on_enter(NfcApp* instance) {
    popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    popup_set_header(instance->popup, "Unlocking", 97, 15, AlignCenter, AlignTop);
    popup_set_text(
        instance->popup, "Hold card next\nto Flipper's back", 94, 27, AlignCenter, AlignTop);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    notification_message(instance->notifications, &sequence_blink_start_yellow);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolSlix);
    nfc_poller_start(instance->poller, slix_scene_unlock_worker_callback, instance);
}

static bool slix_scene_unlock_on_event(NfcApp* instance, SceneManagerEvent event) {
    UNUSED(instance);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventPollerFailure) {
            consumed = true;
        } else if(event.event == NfcCustomEventPollerSuccess) {
            notification_message(instance->notifications, &sequence_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSlixUnlockSuccess);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, NfcSceneSlixUnlockMenu);
    }

    return consumed;
}

static void slix_scene_unlock_on_exit(NfcApp* instance) {
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);
    instance->poller = NULL;

    popup_reset(instance->popup);

    // Stop notifications
    nfc_blink_stop(instance);
}

// ---- unlock_success ----------------------------------------------------
static void
    slix_scene_unlock_success_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcApp* instance = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

static void slix_scene_unlock_success_on_enter(NfcApp* instance) {
    Widget* widget = instance->widget;
    widget_add_string_element(widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "SLIX Unlocked!");

    FuriString* temp_str = furi_string_alloc_set_str("UID:");
    size_t uid_len = 0;
    const uint8_t* uid = nfc_device_get_uid(instance->nfc_device, &uid_len);
    for(size_t i = 0; i < uid_len; i++) {
        furi_string_cat_printf(temp_str, " %02X", uid[i]);
    }
    furi_string_cat_printf(temp_str, "\nPrivacy Mode: Disabled");
    widget_add_string_multiline_element(
        widget, 0, 12, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", slix_scene_unlock_success_widget_callback, instance);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", slix_scene_unlock_success_widget_callback, instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool slix_scene_unlock_success_on_event(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneReadMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }

    return consumed;
}

static void slix_scene_unlock_success_on_exit(NfcApp* instance) {
    widget_reset(instance->widget);
}

const NfcProtocolSupportExtraScene slix_extra_scenes[SlixExtraSceneNum] = {
    [SlixExtraSceneUnlockMenu] =
        {
            .on_enter = slix_scene_unlock_menu_on_enter,
            .on_event = slix_scene_unlock_menu_on_event,
            .on_exit = slix_scene_unlock_menu_on_exit,
        },
    [SlixExtraSceneKeyInput] =
        {
            .on_enter = slix_scene_key_input_on_enter,
            .on_event = slix_scene_key_input_on_event,
            .on_exit = slix_scene_key_input_on_exit,
        },
    [SlixExtraSceneUnlock] =
        {
            .on_enter = slix_scene_unlock_on_enter,
            .on_event = slix_scene_unlock_on_event,
            .on_exit = slix_scene_unlock_on_exit,
        },
    [SlixExtraSceneUnlockSuccess] =
        {
            .on_enter = slix_scene_unlock_success_on_enter,
            .on_event = slix_scene_unlock_success_on_event,
            .on_exit = slix_scene_unlock_success_on_exit,
        },
};
