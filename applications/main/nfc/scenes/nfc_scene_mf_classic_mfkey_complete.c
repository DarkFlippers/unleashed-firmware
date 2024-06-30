#include "../nfc_app_i.h"

void nfc_scene_mf_classic_mfkey_complete_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_mfkey_complete_on_enter(void* context) {
    NfcApp* instance = context;

    widget_add_string_element(
        instance->widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Completed!");
    widget_add_string_multiline_element(
        instance->widget,
        64,
        13,
        AlignCenter,
        AlignTop,
        FontSecondary,
        "Now use Mfkey32 to extract \nkeys: r.flipper.net/nfc-tools");
    widget_add_icon_element(instance->widget, 50, 39, &I_MFKey_qr_25x25);
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeRight,
        "Finish",
        nfc_scene_mf_classic_mfkey_complete_callback,
        instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_mfkey_complete_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneStart);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        const uint32_t prev_scenes[] = {NfcSceneSavedMenu, NfcSceneStart};
        consumed = scene_manager_search_and_switch_to_previous_scene_one_of(
            instance->scene_manager, prev_scenes, COUNT_OF(prev_scenes));
    }

    return consumed;
}

void nfc_scene_mf_classic_mfkey_complete_on_exit(void* context) {
    NfcApp* instance = context;

    widget_reset(instance->widget);
}
