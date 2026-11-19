#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexType,
    SubmenuIndexPreset,
} SubmenuIndex;

#define SCENE_STATE_MENU 0
#define SCENE_STATE_LIST 1

// Preset rows are numbered from here, clear of the two menu entries above
#define PRESET_INDEX_BASE 0x10UL

static void sim800_sms_source_show_menu(Sim800App* app) {
    app->preset_count = sim800_preset_list(app->preset_names, SIM800_PRESET_MAX);

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Message");
    submenu_add_item(
        app->submenu, "Type new text", SubmenuIndexType, sim800_submenu_callback, app);
    if(app->preset_count > 0) {
        submenu_add_item(
            app->submenu, "Use a preset", SubmenuIndexPreset, sim800_submenu_callback, app);
    }

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneSmsSource, SCENE_STATE_MENU);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

static void sim800_sms_source_show_presets(Sim800App* app) {
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "SMS presets");

    for(size_t i = 0; i < app->preset_count; i++) {
        submenu_add_item(
            app->submenu,
            furi_string_get_cstr(app->preset_names[i]),
            PRESET_INDEX_BASE + i,
            sim800_submenu_callback,
            app);
    }

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneSmsSource, SCENE_STATE_LIST);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

void sim800_scene_sms_source_on_enter(void* context) {
    sim800_sms_source_show_menu(context);
}

bool sim800_scene_sms_source_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeBack) {
        // Back out of the preset list to the two choices, not out of the message entirely
        if(scene_manager_get_scene_state(app->scene_manager, Sim800SceneSmsSource) ==
           SCENE_STATE_LIST) {
            sim800_sms_source_show_menu(app);
            return true;
        }
        return false;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == SubmenuIndexType) {
        furi_string_reset(app->sms_text);
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsText);
        return true;
    }

    if(event.event == SubmenuIndexPreset) {
        sim800_sms_source_show_presets(app);
        return true;
    }

    if(event.event >= PRESET_INDEX_BASE) {
        size_t index = (size_t)(event.event - PRESET_INDEX_BASE);
        if(index >= app->preset_count) return true;

        sim800_preset_path(furi_string_get_cstr(app->preset_names[index]), app->file_path);
        if(!sim800_preset_load(furi_string_get_cstr(app->file_path), app->sms_text)) {
            sim800_notify_result(app, false);
            return true;
        }

        // A preset is a finished message: picking one is the send
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneSmsResult, 0);
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsResult);
        return true;
    }

    return false;
}

void sim800_scene_sms_source_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
}
