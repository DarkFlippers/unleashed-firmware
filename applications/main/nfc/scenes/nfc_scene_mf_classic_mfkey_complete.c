#include "../nfc_app_i.h"
#include "loader/loader.h"

typedef enum {
    NfcSceneMfClassicMfKeyCompleteStateAppMissing,
    NfcSceneMfClassicMfKeyCompleteStateAppPresent,
} NfcSceneMfClassicMfKeyCompleteState;

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

    NfcSceneMfClassicMfKeyCompleteState scene_state =
        storage_common_exists(instance->storage, NFC_MFKEY32_APP_PATH) ?
            NfcSceneMfClassicMfKeyCompleteStateAppPresent :
            NfcSceneMfClassicMfKeyCompleteStateAppMissing;
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfClassicMfkeyComplete, scene_state);

    if(scene_state == NfcSceneMfClassicMfKeyCompleteStateAppMissing) {
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
    } else {
        widget_add_string_multiline_element(
            instance->widget,
            60,
            16,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "Now run Mfkey32\n to extract \nkeys");
        widget_add_icon_element(instance->widget, 5, 18, &I_WarningDolphin_45x42);
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "Run",
            nfc_scene_mf_classic_mfkey_complete_callback,
            instance);
    }
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_mfkey_complete_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            NfcSceneMfClassicMfKeyCompleteState scene_state = scene_manager_get_scene_state(
                instance->scene_manager, NfcSceneMfClassicMfkeyComplete);
            if(scene_state == NfcSceneMfClassicMfKeyCompleteStateAppMissing) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcSceneStart);
            } else {
                nfc_app_run_external(instance, NFC_MFKEY32_APP_PATH);
            }
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
