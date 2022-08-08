#include "../subghz_i.h"

#define TAG "SubGhzSetCntBft"

void subghz_scene_set_cnt_bft_byte_input_callback(void* context) {
    SubGhz* subghz = context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_set_cnt_bft_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    ByteInput* byte_input = subghz->byte_input;
    byte_input_set_header_text(byte_input, "Enter COUNTER in hex");
    byte_input_set_result_callback(
        byte_input,
        subghz_scene_set_cnt_bft_byte_input_callback,
        NULL,
        subghz,
        subghz->txrx->secure_data->cnt,
        2);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
}

bool subghz_scene_set_cnt_bft_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetSeedBft);
            consumed = true;
        }
    }
    return consumed;
}

void subghz_scene_set_cnt_bft_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
}
