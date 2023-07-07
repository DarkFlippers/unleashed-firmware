#include "../wifi_marauder_app_i.h"

enum SubmenuIndex {
    SubmenuIndexBoot,
    SubmenuIndexPart,
    SubmenuIndexApp,
    SubmenuIndexFlash,
};

static void wifi_marauder_scene_flasher_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;

    scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneFlasher, index);

    // browse for files
    FuriString* predefined_filepath = furi_string_alloc_set_str(MARAUDER_APP_FOLDER);
    FuriString* selected_filepath = furi_string_alloc();
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".bin", &I_Text_10x10);

    // TODO refactor
    switch(index) {
    case SubmenuIndexBoot:
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_boot,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_boot));
        }
        break;
    case SubmenuIndexPart:
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_part,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_part));
        }
        break;
    case SubmenuIndexApp:
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_app,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_app));
        }
        break;
    case SubmenuIndexFlash:
        // TODO error checking
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        break;
    }

    furi_string_free(selected_filepath);
    furi_string_free(predefined_filepath);
}

void wifi_marauder_scene_flasher_on_enter(void* context) {
    WifiMarauderApp* app = context;

    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, "Browse for files to flash");
    submenu_add_item(
        submenu, "Bootloader", SubmenuIndexBoot, wifi_marauder_scene_flasher_callback, app);
    submenu_add_item(
        submenu, "Partition Table", SubmenuIndexPart, wifi_marauder_scene_flasher_callback, app);
    submenu_add_item(
        submenu, "Application", SubmenuIndexApp, wifi_marauder_scene_flasher_callback, app);
    submenu_add_item(
        submenu, "[>] FLASH", SubmenuIndexFlash, wifi_marauder_scene_flasher_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneFlasher));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

bool wifi_marauder_scene_flasher_on_event(void* context, SceneManagerEvent event) {
    //WifiMarauderApp* app = context;
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void wifi_marauder_scene_flasher_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
