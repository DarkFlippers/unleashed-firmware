#include "../wifi_marauder_app_i.h"

enum SubmenuIndex {
    SubmenuIndexRun,
    SubmenuIndexSettings,
    SubmenuIndexEditStages,
    SubmenuIndexSave,
    SubmenuIndexDelete
};

void wifi_marauder_scene_script_options_save_script(WifiMarauderApp* app) {
    char script_path[256];
    snprintf(
        script_path,
        sizeof(script_path),
        "%s/%s.json",
        MARAUDER_APP_FOLDER_SCRIPTS,
        app->script->name);
    wifi_marauder_script_save_json(app->storage, script_path, app->script);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_text(message, "Saved!", 88, 32, AlignCenter, AlignCenter);
    dialog_message_set_icon(message, &I_DolphinCommon_56x48, 5, 6);
    dialog_message_set_buttons(message, NULL, "Ok", NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

static void wifi_marauder_scene_script_options_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;

    switch(index) {
    case SubmenuIndexRun:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        break;
    case SubmenuIndexSettings:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptSettings);
        break;
    case SubmenuIndexEditStages:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptEdit);
        break;
    case SubmenuIndexSave:
        wifi_marauder_scene_script_options_save_script(app);
        break;
    case SubmenuIndexDelete:
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptConfirmDelete);
        break;
    }
}

void wifi_marauder_scene_script_options_on_enter(void* context) {
    WifiMarauderApp* app = context;

    // If returning after confirming script deletion
    if(app->script == NULL) {
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, app->script->name);
    submenu_add_item(
        submenu, "[>] RUN", SubmenuIndexRun, wifi_marauder_scene_script_options_callback, app);
    submenu_add_item(
        submenu,
        "[S] SETTINGS",
        SubmenuIndexSettings,
        wifi_marauder_scene_script_options_callback,
        app);
    submenu_add_item(
        submenu,
        "[+] EDIT STAGES",
        SubmenuIndexEditStages,
        wifi_marauder_scene_script_options_callback,
        app);
    submenu_add_item(
        submenu, "[*] SAVE", SubmenuIndexSave, wifi_marauder_scene_script_options_callback, app);
    submenu_add_item(
        submenu,
        "[X] DELETE",
        SubmenuIndexDelete,
        wifi_marauder_scene_script_options_callback,
        app);

    submenu_set_selected_item(
        submenu,
        scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptOptions));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_script_options_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        wifi_marauder_script_free(app->script);
        app->script = NULL;
    }

    return consumed;
}

void wifi_marauder_scene_script_options_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
