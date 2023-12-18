#include "../nfc_app_i.h"

void nfc_scene_mf_classic_mfkey_nonces_info_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcApp* instance = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_mfkey_nonces_info_on_enter(void* context) {
    NfcApp* instance = context;

    FuriString* temp_str = furi_string_alloc();

    size_t mfkey_params_saved = mfkey32_logger_get_params_num(instance->mfkey32_logger);
    furi_string_printf(temp_str, "Nonce pairs saved: %zu\n", mfkey_params_saved);
    widget_add_string_element(
        instance->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(temp_str));
    widget_add_string_element(
        instance->widget, 0, 12, AlignLeft, AlignTop, FontSecondary, "Authenticated sectors:");

    mfkey32_logger_get_params_data(instance->mfkey32_logger, temp_str);
    widget_add_text_scroll_element(
        instance->widget, 0, 22, 128, 42, furi_string_get_cstr(temp_str));
    widget_add_button_element(
        instance->widget,
        GuiButtonTypeCenter,
        "OK",
        nfc_scene_mf_classic_mfkey_nonces_info_callback,
        instance);

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_mfkey_nonces_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            if(mfkey32_logger_save_params(
                   instance->mfkey32_logger, NFC_APP_MFKEY32_LOGS_FILE_PATH)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicMfkeyComplete);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, NfcSceneStart);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        const uint32_t prev_scenes[] = {NfcSceneSavedMenu, NfcSceneStart};
        consumed = scene_manager_search_and_switch_to_previous_scene_one_of(
            instance->scene_manager, prev_scenes, COUNT_OF(prev_scenes));
    }

    return consumed;
}

void nfc_scene_mf_classic_mfkey_nonces_info_on_exit(void* context) {
    NfcApp* instance = context;

    mfkey32_logger_free(instance->mfkey32_logger);

    // Clear view
    widget_reset(instance->widget);
}
