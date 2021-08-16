#include "../subghz_i.h"

const void subghz_scene_analyze_on_enter(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewAnalyze);
}

const bool subghz_scene_analyze_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeTick) {
        notification_message(subghz->notifications, &sequence_blink_blue_10);
        return true;
    }
    return false;
}

const void subghz_scene_analyze_on_exit(void* context) {
    // SubGhz* subghz = context;
}
