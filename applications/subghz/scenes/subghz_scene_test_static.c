#include "../subghz_i.h"

const void subghz_scene_test_static_on_enter(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewStatic);
}

const bool subghz_scene_test_static_on_event(void* context, SceneManagerEvent event) {
    // SubGhz* subghz = context;
    return false;
}

const void subghz_scene_test_static_on_exit(void* context) {
    // SubGhz* subghz = context;
}
