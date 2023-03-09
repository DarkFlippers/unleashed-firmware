#include "../ibutton_i.h"

static void ibutton_scene_add_type_byte_input_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventByteEditResult);
}

static void ibutton_scene_add_type_byte_changed_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventByteEditChanged);
}

void ibutton_scene_add_value_on_enter(void* context) {
    iButton* ibutton = context;
    byte_input_set_header_text(ibutton->byte_input, "Enter the key");

    iButtonEditableData editable_data;
    ibutton_protocols_get_editable_data(ibutton->protocols, ibutton->key, &editable_data);

    byte_input_set_result_callback(
        ibutton->byte_input,
        ibutton_scene_add_type_byte_input_callback,
        ibutton_scene_add_type_byte_changed_callback,
        context,
        editable_data.ptr,
        editable_data.size);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewByteInput);
}

bool ibutton_scene_add_value_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventByteEditResult) {
            scene_manager_next_scene(scene_manager, iButtonSceneSaveName);
        } else if(event.event == iButtonCustomEventByteEditChanged) {
            ibutton_protocols_apply_edits(ibutton->protocols, ibutton->key);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        // User cancelled editing, reload the key from storage
        if(scene_manager_has_previous_scene(scene_manager, iButtonSceneSavedKeyMenu)) {
            if(!ibutton_load_key(ibutton)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, iButtonSceneStart);
            }
        }
    }

    return consumed;
}

void ibutton_scene_add_value_on_exit(void* context) {
    iButton* ibutton = context;

    byte_input_set_result_callback(ibutton->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(ibutton->byte_input, NULL);
}
