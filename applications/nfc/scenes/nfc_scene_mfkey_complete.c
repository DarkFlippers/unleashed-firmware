#include "../nfc_i.h"

void nfc_scene_mfkey_complete_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mfkey_complete_on_enter(void* context) {
    Nfc* nfc = context;

    widget_add_string_element(nfc->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Complete!");
    widget_add_string_multiline_element(
        nfc->widget,
        64,
        32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "Now use mfkey32v2\nto extract keys");
    widget_add_button_element(
        nfc->widget, GuiButtonTypeCenter, "OK", nfc_scene_mfkey_complete_callback, nfc);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mfkey_complete_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                nfc->scene_manager, NfcSceneStart);
        }
    } else if(event.event == SceneManagerEventTypeBack) {
        consumed =
            scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneStart);
    }

    return consumed;
}

void nfc_scene_mfkey_complete_on_exit(void* context) {
    Nfc* nfc = context;

    widget_reset(nfc->widget);
}