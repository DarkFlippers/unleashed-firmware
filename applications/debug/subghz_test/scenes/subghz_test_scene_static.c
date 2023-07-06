#include "../subghz_test_app_i.h"

void subghz_test_scene_static_callback(SubGhzTestStaticEvent event, void* context) {
    furi_assert(context);
    SubGhzTestApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void subghz_test_scene_static_on_enter(void* context) {
    SubGhzTestApp* app = context;
    subghz_test_static_set_callback(
        app->subghz_test_static, subghz_test_scene_static_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzTestViewStatic);
}

bool subghz_test_scene_static_on_event(void* context, SceneManagerEvent event) {
    SubGhzTestApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzTestStaticEventOnlyRx) {
            scene_manager_next_scene(app->scene_manager, SubGhzTestSceneShowOnlyRx);
            return true;
        }
    }
    return false;
}

void subghz_test_scene_static_on_exit(void* context) {
    UNUSED(context);
}
