#include "../nfc_i.h"
#include <lib/nfc/helpers/mfkey32.h>

void nfc_scene_mfkey_nonces_info_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mfkey_nonces_info_on_enter(void* context) {
    Nfc* nfc = context;

    FuriString* temp_str;
    temp_str = furi_string_alloc();

    uint16_t nonces_saved = mfkey32_get_auth_sectors(temp_str);
    widget_add_text_scroll_element(nfc->widget, 0, 22, 128, 42, furi_string_get_cstr(temp_str));
    furi_string_printf(temp_str, "Nonce pairs saved: %d", nonces_saved);
    widget_add_string_element(
        nfc->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, furi_string_get_cstr(temp_str));
    widget_add_string_element(
        nfc->widget, 0, 12, AlignLeft, AlignTop, FontSecondary, "Authenticated sectors:");

    widget_add_button_element(
        nfc->widget, GuiButtonTypeCenter, "OK", nfc_scene_mfkey_nonces_info_callback, nfc);

    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mfkey_nonces_info_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfkeyComplete);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed =
            scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneStart);
    }

    return consumed;
}

void nfc_scene_mfkey_nonces_info_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    widget_reset(nfc->widget);
}
