#include "../avr_isp_app_i.h"

void avr_isp_scene_load_on_enter(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    if(avr_isp_load_from_file(app)) {
        scene_manager_next_scene(app->scene_manager, AvrIspSceneWriter);
    } else {
        scene_manager_previous_scene(app->scene_manager);
    }
}

bool avr_isp_scene_load_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void avr_isp_scene_load_on_exit(void* context) {
    UNUSED(context);
}
