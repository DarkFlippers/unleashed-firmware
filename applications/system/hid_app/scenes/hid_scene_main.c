#include "../hid.h"
#include "../views.h"

void hid_scene_main_on_enter(void* context) {
    Hid* app = context;
    view_dispatcher_switch_to_view(
        app->view_dispatcher, scene_manager_get_scene_state(app->scene_manager, HidSceneMain));
}

bool hid_scene_main_on_event(void* context, SceneManagerEvent event) {
    Hid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom && event.event == HidCustomEventUnpair) {
        scene_manager_next_scene(app->scene_manager, HidSceneUnpair);
        consumed = true;
    }

    return consumed;
}

void hid_scene_main_on_exit(void* context) {
    Hid* app = context;
    UNUSED(app);
}
