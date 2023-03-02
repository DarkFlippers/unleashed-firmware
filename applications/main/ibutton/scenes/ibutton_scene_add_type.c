#include "../ibutton_i.h"

void ibutton_scene_add_type_on_enter(void* context) {
    iButton* ibutton = context;
    Submenu* submenu = ibutton->submenu;

    FuriString* tmp = furi_string_alloc();

    for(uint32_t protocol_id = 0; protocol_id < ibutton_protocols_get_protocol_count();
        ++protocol_id) {
        furi_string_printf(
            tmp,
            "%s %s",
            ibutton_protocols_get_manufacturer(ibutton->protocols, protocol_id),
            ibutton_protocols_get_name(ibutton->protocols, protocol_id));

        submenu_add_item(
            submenu, furi_string_get_cstr(tmp), protocol_id, ibutton_submenu_callback, context);
    }

    const uint32_t prev_protocol_id =
        scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneAddType);
    submenu_set_selected_item(submenu, prev_protocol_id);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewSubmenu);
    furi_string_free(tmp);
}

bool ibutton_scene_add_type_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const iButtonProtocolId protocol_id = event.event;

        ibutton_key_reset(key);
        ibutton_key_set_protocol_id(key, protocol_id);
        ibutton_protocols_apply_edits(ibutton->protocols, key);

        scene_manager_set_scene_state(ibutton->scene_manager, iButtonSceneAddType, protocol_id);
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneAddValue);

        consumed = true;
    }

    return consumed;
}

void ibutton_scene_add_type_on_exit(void* context) {
    iButton* ibutton = context;
    submenu_reset(ibutton->submenu);
}
