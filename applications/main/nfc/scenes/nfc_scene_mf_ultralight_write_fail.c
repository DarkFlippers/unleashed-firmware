#include "../nfc_app_i.h"

void nfc_scene_mf_ultralight_write_fail_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_ultralight_write_fail_on_enter(void* context) {
    NfcApp* instance = context;
    Widget* widget = instance->widget;

    notification_message(instance->notifications, &sequence_error);

    widget_add_icon_element(widget, 83, 22, &I_WarningDolphinFlip_45x42);
    widget_add_string_element(
        widget, 7, 4, AlignLeft, AlignTop, FontPrimary, "Writing gone wrong!");
    widget_add_string_multiline_element(
        widget,
        7,
        17,
        AlignLeft,
        AlignTop,
        FontSecondary,
        "Card protected by\npassword, AUTH0\nor lock bits");

    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Finish",
        nfc_scene_mf_ultralight_write_fail_widget_callback,
        instance);

    // Setup and start worker
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

static bool nfc_scene_mf_ultralight_write_fail_move_to_back_scene(const NfcApp* const instance) {
    bool was_saved = scene_manager_has_previous_scene(instance->scene_manager, NfcSceneSavedMenu);
    uint32_t scene_id = was_saved ? NfcSceneSavedMenu : NfcSceneReadMenu;

    return scene_manager_search_and_switch_to_previous_scene(instance->scene_manager, scene_id);
}

bool nfc_scene_mf_ultralight_write_fail_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = nfc_scene_mf_ultralight_write_fail_move_to_back_scene(instance);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = nfc_scene_mf_ultralight_write_fail_move_to_back_scene(instance);
    }
    return consumed;
}

void nfc_scene_mf_ultralight_write_fail_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
