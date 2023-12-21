#include "nfc/nfc_app_i.h"

#include "nfc/helpers/protocol_support/nfc_protocol_support_gui_common.h"

void nfc_scene_supported_card_on_enter(void* context) {
    NfcApp* instance = context;

    FuriString* temp_str = furi_string_alloc();

    if(nfc_supported_cards_parse(instance->nfc_supported_cards, instance->nfc_device, temp_str)) {
        widget_add_text_scroll_element(
            instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
        widget_add_button_element(
            instance->widget,
            GuiButtonTypeRight,
            "More",
            nfc_protocol_support_common_widget_callback,
            instance);

        scene_manager_set_scene_state(instance->scene_manager, NfcSceneSupportedCard, true);
        view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);

    } else {
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneSupportedCard, false);
        scene_manager_next_scene(instance->scene_manager, NfcSceneInfo);
    }

    furi_string_free(temp_str);
}

bool nfc_scene_supported_card_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneInfo);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_supported_card_on_exit(void* context) {
    NfcApp* instance = context;
    widget_reset(instance->widget);
}
