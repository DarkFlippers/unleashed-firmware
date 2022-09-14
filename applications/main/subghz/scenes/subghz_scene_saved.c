#include "../subghz_i.h"

void subghz_scene_saved_on_enter(void* context) {
    SubGhz* subghz = context;

    if(subghz_load_protocol_from_file(subghz)) {
        if((!strcmp(subghz->txrx->decoder_result->protocol->name, "RAW"))) {
            subghz->txrx->rx_key_state = SubGhzRxKeyStateRAWLoad;
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReadRAW);
        } else {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSavedMenu);
        }
    } else {
        scene_manager_search_and_switch_to_previous_scene(subghz->scene_manager, SubGhzSceneStart);
    }
}

bool subghz_scene_saved_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void subghz_scene_saved_on_exit(void* context) {
    UNUSED(context);
}
