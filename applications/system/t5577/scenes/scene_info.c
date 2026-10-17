#include "../t5577_app.h"

void t5577_app_scene_info_on_enter(void* context) {
    T5577App* app = context;

    t5577_dump_view_set_dump(app->dump_view, &app->dump);

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewDump);
}

bool t5577_app_scene_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void t5577_app_scene_info_on_exit(void* context) {
    UNUSED(context);
}
