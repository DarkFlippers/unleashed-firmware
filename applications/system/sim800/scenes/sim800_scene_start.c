#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexBasic,
    SubmenuIndexSaved,
    SubmenuIndexAdvanced,
    SubmenuIndexAddNumber,
    SubmenuIndexPresets,
    SubmenuIndexSettings,
    SubmenuIndexHowto,
} SubmenuIndex;

void sim800_scene_start_on_enter(void* context) {
    Sim800App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "SIM800");
    submenu_add_item(app->submenu, "Basic", SubmenuIndexBasic, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Saved", SubmenuIndexSaved, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Advanced", SubmenuIndexAdvanced, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Add number", SubmenuIndexAddNumber, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "SMS presets", SubmenuIndexPresets, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Settings", SubmenuIndexSettings, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "How to use", SubmenuIndexHowto, sim800_submenu_callback, app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, Sim800SceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

bool sim800_scene_start_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    static const uint32_t next_scene[] = {
        [SubmenuIndexBasic] = Sim800SceneBasic,
        [SubmenuIndexSaved] = Sim800SceneSaved,
        [SubmenuIndexAdvanced] = Sim800SceneAdvanced,
        [SubmenuIndexAddNumber] = Sim800SceneNumberInput,
        [SubmenuIndexPresets] = Sim800ScenePresets,
        [SubmenuIndexSettings] = Sim800SceneSettings,
        [SubmenuIndexHowto] = Sim800SceneHowto,
    };

    if(event.event > SubmenuIndexHowto) return false;

    if(event.event == SubmenuIndexAddNumber) {
        app->mode = Sim800ModeNewEntry;
        furi_string_set_str(app->number, "+");
        furi_string_reset(app->entry_name);
    }

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneStart, event.event);
    scene_manager_next_scene(app->scene_manager, next_scene[event.event]);
    return true;
}

void sim800_scene_start_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
}
