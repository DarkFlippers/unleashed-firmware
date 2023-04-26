#include "../wifi_marauder_app_i.h"

static void wifi_marauder_scene_script_select_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;

    char script_path[256];
    snprintf(
        script_path,
        sizeof(script_path),
        "%s/%s.json",
        MARAUDER_APP_FOLDER_SCRIPTS,
        furi_string_get_cstr(app->script_list[index]));

    app->script = wifi_marauder_script_parse_json(app->storage, script_path);
    if(app->script) {
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptSelect, index);
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptOptions);
    }
}

static void wifi_marauder_scene_script_select_add_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;
    scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptSelect, index);

    app->user_input_type = WifiMarauderUserInputTypeFileName;
    app->user_input_file_dir = strdup(MARAUDER_APP_FOLDER_SCRIPTS);
    app->user_input_file_extension = strdup("json");
    scene_manager_next_scene(app->scene_manager, WifiMarauderSceneUserInput);
}

void wifi_marauder_scene_script_select_on_enter(void* context) {
    WifiMarauderApp* app = context;
    Submenu* submenu = app->submenu;

    File* dir_scripts = storage_file_alloc(app->storage);
    if(storage_dir_open(dir_scripts, MARAUDER_APP_FOLDER_SCRIPTS)) {
        FileInfo file_info;
        char file_path[255];
        app->script_list_count = 0;
        // Goes through the files in the folder counting the ones that end with the json extension
        while(storage_dir_read(dir_scripts, &file_info, file_path, 255)) {
            app->script_list_count++;
        }
        if(app->script_list_count > 0) {
            submenu_set_header(submenu, "Select a script:");
            app->script_list = malloc(app->script_list_count * sizeof(FuriString*));
            storage_dir_close(dir_scripts);
            storage_dir_open(dir_scripts, MARAUDER_APP_FOLDER_SCRIPTS);
            // Read the files again from the beginning, adding the scripts in the list
            int script_index = 0;
            while(storage_dir_read(dir_scripts, &file_info, file_path, 255)) {
                app->script_list[script_index] = furi_string_alloc();
                path_extract_filename_no_ext(file_path, app->script_list[script_index]);
                submenu_add_item(
                    submenu,
                    furi_string_get_cstr(app->script_list[script_index]),
                    script_index,
                    wifi_marauder_scene_script_select_callback,
                    app);
                script_index++;
            }
        } else {
            submenu_set_header(submenu, "No script found");
        }
        submenu_add_item(
            submenu, "[+] ADD SCRIPT", 99, wifi_marauder_scene_script_select_add_callback, app);
        storage_dir_close(dir_scripts);
    }
    storage_file_free(dir_scripts);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptSelect));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_script_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void wifi_marauder_scene_script_select_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);

    for(int i = 0; i < app->script_list_count; i++) {
        furi_string_free(app->script_list[i]);
    }
    free(app->script_list);
}
