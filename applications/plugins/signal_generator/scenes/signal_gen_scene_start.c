#include "../signal_gen_app_i.h"

typedef enum {
    SubmenuIndexPwm,
    SubmenuIndexClockOutput,
} SubmenuIndex;

void signal_gen_scene_start_submenu_callback(void* context, uint32_t index) {
    SignalGenApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void signal_gen_scene_start_on_enter(void* context) {
    SignalGenApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu, "PWM", SubmenuIndexPwm, signal_gen_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu,
        "Clock Output",
        SubmenuIndexClockOutput,
        signal_gen_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, SignalGenSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, SignalGenViewSubmenu);
}

bool signal_gen_scene_start_on_event(void* context, SceneManagerEvent event) {
    SignalGenApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexPwm) {
            scene_manager_next_scene(app->scene_manager, SignalGenScenePwm);
            consumed = true;
        } else if(event.event == SubmenuIndexClockOutput) {
            scene_manager_next_scene(app->scene_manager, SignalGenSceneMco);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, SignalGenSceneStart, event.event);
    }

    return consumed;
}

void signal_gen_scene_start_on_exit(void* context) {
    SignalGenApp* app = context;

    submenu_reset(app->submenu);
}
