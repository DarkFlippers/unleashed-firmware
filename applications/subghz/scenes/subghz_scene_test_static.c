#include "../subghz_i.h"
#include "../views/subghz_test_static.h"

void subghz_scene_test_static_callback(SubGhzTestStaticEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_test_static_on_enter(void* context) {
    SubGhz* subghz = context;
    subghz_test_static_set_callback(
        subghz->subghz_test_static, subghz_scene_test_static_callback, subghz);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdStatic);
}

bool subghz_scene_test_static_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzTestStaticEventOnlyRx) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
            return true;
        }
    }
    return false;
}

void subghz_scene_test_static_on_exit(void* context) {
    UNUSED(context);
}
