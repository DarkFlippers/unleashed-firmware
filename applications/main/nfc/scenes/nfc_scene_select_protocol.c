#include "../nfc_app_i.h"

void nfc_scene_select_protocol_submenu_callback(void* context, uint32_t index) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, index);
}

void nfc_scene_select_protocol_on_enter(void* context) {
    NfcApp* instance = context;
    Submenu* submenu = instance->submenu;

    FuriString* temp_str = furi_string_alloc();
    const char* prefix;
    if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneExtraActions)) {
        prefix = "Read";
        nfc_detected_protocols_fill_all_protocols(instance->detected_protocols);
    } else {
        prefix = "Read as";
        submenu_set_header(submenu, "Multi-protocol card");
    }

    for(uint32_t i = 0; i < nfc_detected_protocols_get_num(instance->detected_protocols); i++) {
        furi_string_printf(
            temp_str,
            "%s %s",
            prefix,
            nfc_device_get_protocol_name(
                nfc_detected_protocols_get_protocol(instance->detected_protocols, i)));

        furi_string_replace_str(temp_str, "Mifare", "MIFARE");
        submenu_add_item(
            submenu,
            furi_string_get_cstr(temp_str),
            i,
            nfc_scene_select_protocol_submenu_callback,
            instance);
    }
    furi_string_free(temp_str);

    submenu_set_selected_item(
        submenu, nfc_detected_protocols_get_selected_idx(instance->detected_protocols));

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_select_protocol_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        nfc_detected_protocols_select(instance->detected_protocols, event.event);
        scene_manager_next_scene(instance->scene_manager, NfcSceneRead);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneDetect)) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneStart);
        }
    }
    return consumed;
}

void nfc_scene_select_protocol_on_exit(void* context) {
    NfcApp* nfc = context;

    submenu_reset(nfc->submenu);
}
