#include "../sim800_app_i.h"

#define PRESET_INDEX_NEW 0xFEUL

void sim800_scene_presets_on_enter(void* context) {
    Sim800App* app = context;

    app->preset_count = sim800_preset_list(app->preset_names, SIM800_PRESET_MAX);

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "SMS presets");
    submenu_add_item(app->submenu, "New preset", PRESET_INDEX_NEW, sim800_submenu_callback, app);

    for(size_t i = 0; i < app->preset_count; i++) {
        submenu_add_item(
            app->submenu,
            furi_string_get_cstr(app->preset_names[i]),
            i,
            sim800_submenu_callback,
            app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

bool sim800_scene_presets_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == PRESET_INDEX_NEW) {
        app->preset_is_new = true;
        furi_string_reset(app->sms_text);
        furi_string_reset(app->entry_name);
        scene_manager_next_scene(app->scene_manager, Sim800ScenePresetText);
        return true;
    }

    if(event.event >= app->preset_count) return true;

    app->preset_is_new = false;
    furi_string_set(app->entry_name, app->preset_names[event.event]);
    sim800_preset_path(furi_string_get_cstr(app->entry_name), app->file_path);
    scene_manager_next_scene(app->scene_manager, Sim800ScenePresetMenu);
    return true;
}

void sim800_scene_presets_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
}
