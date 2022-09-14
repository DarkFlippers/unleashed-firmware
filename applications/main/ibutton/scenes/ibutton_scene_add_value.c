#include "../ibutton_i.h"

#include <dolphin/dolphin.h>

void ibutton_scene_add_type_byte_input_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventByteEditResult);
}

void ibutton_scene_add_value_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    uint8_t* new_key_data = malloc(IBUTTON_KEY_DATA_SIZE);

    scene_manager_set_scene_state(
        ibutton->scene_manager, iButtonSceneAddValue, (uint32_t)new_key_data);
    memcpy(new_key_data, ibutton_key_get_data_p(key), ibutton_key_get_data_size(key));

    byte_input_set_result_callback(
        ibutton->byte_input,
        ibutton_scene_add_type_byte_input_callback,
        NULL,
        ibutton,
        new_key_data,
        ibutton_key_get_data_size(key));

    byte_input_set_header_text(ibutton->byte_input, "Enter the key");
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewByteInput);
}

bool ibutton_scene_add_value_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    uint8_t* new_key_data =
        (uint8_t*)scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneAddValue);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventByteEditResult) {
            ibutton_key_set_data(ibutton->key, new_key_data, IBUTTON_KEY_DATA_SIZE);
            DOLPHIN_DEED(DolphinDeedIbuttonAdd);
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneSaveName);
        }
    }

    return consumed;
}

void ibutton_scene_add_value_on_exit(void* context) {
    iButton* ibutton = context;
    uint8_t* new_key_data =
        (uint8_t*)scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneAddValue);

    byte_input_set_result_callback(ibutton->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(ibutton->byte_input, NULL);
    free(new_key_data);
}
