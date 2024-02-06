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
        instance->protocols_detected_num = NfcProtocolNum;
        for(uint32_t i = 0; i < NfcProtocolNum; i++) {
            instance->protocols_detected[i] = i;
        }
    } else {
        prefix = "Read as";
        submenu_set_header(submenu, "Multi-protocol card");
    }

    for(uint32_t i = 0; i < instance->protocols_detected_num; i++) {
        furi_string_printf(
            temp_str,
            "%s %s",
            prefix,
            nfc_device_get_protocol_name(instance->protocols_detected[i]));

        furi_string_replace_str(temp_str, "Mifare", "MIFARE");
        submenu_add_item(
            submenu,
            furi_string_get_cstr(temp_str),
            i,
            nfc_scene_select_protocol_submenu_callback,
            instance);
    }
    furi_string_free(temp_str);

    const uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneSelectProtocol);
    submenu_set_selected_item(submenu, state);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_select_protocol_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        instance->protocols_detected_selected_idx = event.event;
        scene_manager_next_scene(instance->scene_manager, NfcSceneRead);
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneSelectProtocol, event.event);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_select_protocol_on_exit(void* context) {
    NfcApp* nfc = context;

    submenu_reset(nfc->submenu);
}
