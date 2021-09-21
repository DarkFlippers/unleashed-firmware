#include "../subghz_i.h"

void subghz_scene_saved_on_enter(void* context) {
    SubGhz* subghz = context;

    if(subghz_load_protocol_from_file(subghz)) {
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTransmitter);
    } else {
        scene_manager_search_and_switch_to_previous_scene(subghz->scene_manager, SubGhzSceneStart);
    }
}

bool subghz_scene_saved_on_event(void* context, SceneManagerEvent event) {
    // SubGhz* subghz = context;
    return false;
}

void subghz_scene_saved_on_exit(void* context) {
    // SubGhz* subghz = context;
}
