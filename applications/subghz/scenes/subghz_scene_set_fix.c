#include "../subghz_i.h"

void subghz_scene_set_fix_byte_input_callback(void* context) {
    SubGhz* subghz = (SubGhz*)context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_set_fix_on_enter(void* context) {
    SubGhz* subghz = (SubGhz*)context;

    // Setup view
    ByteInput* byte_input = subghz->byte_input;
    byte_input_set_header_text(byte_input, "Enter FIX in hex");
    byte_input_set_result_callback(
        byte_input,
        subghz_scene_set_fix_byte_input_callback,
        NULL,
        subghz,
        subghz->txrx->secure_data->fix,
        4);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
}

bool subghz_scene_set_fix_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = (SubGhz*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetCnt);
            return true;
        }
    }
    return false;
}

void subghz_scene_set_fix_on_exit(void* context) {
    SubGhz* subghz = (SubGhz*)context;

    // Clear view
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
}
