#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexCall,
    SubmenuIndexSms,
    SubmenuIndexStatus,
} SubmenuIndex;

void sim800_scene_basic_on_enter(void* context) {
    Sim800App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Basic");
    submenu_add_item(app->submenu, "Call", SubmenuIndexCall, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "SMS", SubmenuIndexSms, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Status", SubmenuIndexStatus, sim800_submenu_callback, app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, Sim800SceneBasic));

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

bool sim800_scene_basic_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event > SubmenuIndexStatus) return false;

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneBasic, event.event);

    if(event.event == SubmenuIndexStatus) {
        scene_manager_next_scene(app->scene_manager, Sim800SceneStatus);
        return true;
    }

    app->mode = (event.event == SubmenuIndexCall) ? Sim800ModeCall : Sim800ModeSms;
    furi_string_reset(app->sms_text);
    sim800_number_ask(app);
    return true;
}

void sim800_scene_basic_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
}
