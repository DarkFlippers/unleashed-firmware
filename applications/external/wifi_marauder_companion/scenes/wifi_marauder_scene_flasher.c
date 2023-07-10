#include "../wifi_marauder_app_i.h"
#include "../wifi_marauder_flasher.h"

enum SubmenuIndex {
    SubmenuIndexS3Mode,
    SubmenuIndexBoot,
    SubmenuIndexPart,
    SubmenuIndexNvs,
    SubmenuIndexBootApp0,
    SubmenuIndexApp,
    SubmenuIndexCustom,
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
    case SubmenuIndexS3Mode:
        // toggle S3 mode
        app->selected_flash_options[SelectedFlashS3Mode] =
            !app->selected_flash_options[SelectedFlashS3Mode];
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshSubmenu);
        break;
    case SubmenuIndexBoot:
        app->selected_flash_options[SelectedFlashBoot] =
            !app->selected_flash_options[SelectedFlashBoot];
        if(app->selected_flash_options[SelectedFlashBoot]) {
            if(dialog_file_browser_show(
                   app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
                strncpy(
                    app->bin_file_path_boot,
                    furi_string_get_cstr(selected_filepath),
                    sizeof(app->bin_file_path_boot));
            }
        }
        if(app->bin_file_path_boot[0] == '\0') {
            // if user didn't select a file, leave unselected
            app->selected_flash_options[SelectedFlashBoot] = false;
        }
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshSubmenu);
        break;
    case SubmenuIndexPart:
        app->selected_flash_options[SelectedFlashPart] =
            !app->selected_flash_options[SelectedFlashPart];
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_part,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_part));
        }
        if(app->bin_file_path_part[0] == '\0') {
            // if user didn't select a file, leave unselected
            app->selected_flash_options[SelectedFlashPart] = false;
        }
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshSubmenu);
        break;
    case SubmenuIndexNvs:
        app->selected_flash_options[SelectedFlashNvs] =
            !app->selected_flash_options[SelectedFlashNvs];
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_nvs,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_nvs));
        }
        if(app->bin_file_path_nvs[0] == '\0') {
            // if user didn't select a file, leave unselected
            app->selected_flash_options[SelectedFlashNvs] = false;
        }
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshSubmenu);
        break;
    case SubmenuIndexBootApp0:
        app->selected_flash_options[SelectedFlashBootApp0] =
            !app->selected_flash_options[SelectedFlashBootApp0];
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_boot_app0,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_boot_app0));
        }
        if(app->bin_file_path_boot_app0[0] == '\0') {
            // if user didn't select a file, leave unselected
            app->selected_flash_options[SelectedFlashBootApp0] = false;
        }
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshSubmenu);
        break;
    case SubmenuIndexApp:
        app->selected_flash_options[SelectedFlashApp] =
            !app->selected_flash_options[SelectedFlashApp];
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_app,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_app));
        }
        if(app->bin_file_path_app[0] == '\0') {
            // if user didn't select a file, leave unselected
            app->selected_flash_options[SelectedFlashApp] = false;
        }
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshSubmenu);
        break;
    case SubmenuIndexCustom:
        app->selected_flash_options[SelectedFlashCustom] =
            !app->selected_flash_options[SelectedFlashCustom];
        if(dialog_file_browser_show(
               app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
            strncpy(
                app->bin_file_path_custom,
                furi_string_get_cstr(selected_filepath),
                sizeof(app->bin_file_path_custom));
        }
        if(app->bin_file_path_custom[0] == '\0') {
            // if user didn't select a file, leave unselected
            app->selected_flash_options[SelectedFlashCustom] = false;
        }
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshSubmenu);
        break;
    case SubmenuIndexFlash:
        // count how many options are selected
        app->num_selected_flash_options = 0;
        for(bool* option = &app->selected_flash_options[SelectedFlashBoot];
            option < &app->selected_flash_options[NUM_FLASH_OPTIONS];
            ++option) {
            if(*option) {
                ++app->num_selected_flash_options;
            }
        }
        if(app->num_selected_flash_options) {
            // only start next scene if at least one option is selected
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        }
        break;
    }

    furi_string_free(selected_filepath);
    furi_string_free(predefined_filepath);
}

#define STR_SELECT "[x]"
#define STR_UNSELECT "[ ]"
#define STR_BOOT "Bootloader (" TOSTRING(ESP_ADDR_BOOT) ")"
#define STR_BOOT_S3 "Bootloader (" TOSTRING(ESP_ADDR_BOOT_S3) ")"
#define STR_PART "Part Table (" TOSTRING(ESP_ADDR_PART) ")"
#define STR_NVS "NVS (" TOSTRING(ESP_ADDR_NVS) ")"
#define STR_BOOT_APP0 "boot_app0 (" TOSTRING(ESP_ADDR_BOOT_APP0) ")"
#define STR_APP "Firmware (" TOSTRING(ESP_ADDR_APP) ")"
#define STR_CUSTOM "Custom"
#define STR_FLASH_S3 "[>] FLASH (ESP32-S3)"
#define STR_FLASH "[>] FLASH"
static void _refresh_submenu(WifiMarauderApp* app) {
    Submenu* submenu = app->submenu;

    submenu_reset(app->submenu);

    submenu_set_header(submenu, "Browse for files to flash");
    submenu_add_item(
        submenu,
        app->selected_flash_options[SelectedFlashS3Mode] ? "[x] Using ESP32-S3" :
                                                           "[ ] Check if using S3",
        SubmenuIndexS3Mode,
        wifi_marauder_scene_flasher_callback,
        app);
    const char* strSelectBootloader = STR_UNSELECT " " STR_BOOT;
    if(app->selected_flash_options[SelectedFlashS3Mode]) {
        if(app->selected_flash_options[SelectedFlashBoot]) {
            strSelectBootloader = STR_SELECT " " STR_BOOT_S3;
        } else {
            strSelectBootloader = STR_UNSELECT " " STR_BOOT_S3;
        }
    } else {
        if(app->selected_flash_options[SelectedFlashBoot]) {
            strSelectBootloader = STR_SELECT " " STR_BOOT;
        } else {
            strSelectBootloader = STR_UNSELECT " " STR_BOOT;
        }
    }
    submenu_add_item(
        submenu, strSelectBootloader, SubmenuIndexBoot, wifi_marauder_scene_flasher_callback, app);
    submenu_add_item(
        submenu,
        app->selected_flash_options[SelectedFlashPart] ? STR_SELECT " " STR_PART :
                                                         STR_UNSELECT " " STR_PART,
        SubmenuIndexPart,
        wifi_marauder_scene_flasher_callback,
        app);
    submenu_add_item(
        submenu,
        app->selected_flash_options[SelectedFlashNvs] ? STR_SELECT " " STR_NVS :
                                                        STR_UNSELECT " " STR_NVS,
        SubmenuIndexNvs,
        wifi_marauder_scene_flasher_callback,
        app);
    submenu_add_item(
        submenu,
        app->selected_flash_options[SelectedFlashBootApp0] ? STR_SELECT " " STR_BOOT_APP0 :
                                                             STR_UNSELECT " " STR_BOOT_APP0,
        SubmenuIndexBootApp0,
        wifi_marauder_scene_flasher_callback,
        app);
    submenu_add_item(
        submenu,
        app->selected_flash_options[SelectedFlashApp] ? STR_SELECT " " STR_APP :
                                                        STR_UNSELECT " " STR_APP,
        SubmenuIndexApp,
        wifi_marauder_scene_flasher_callback,
        app);
    // TODO: custom addr
    //submenu_add_item(
    //    submenu, app->selected_flash_options[SelectedFlashCustom] ? STR_SELECT " " STR_CUSTOM : STR_UNSELECT " " STR_CUSTOM, SubmenuIndexCustom, wifi_marauder_scene_flasher_callback, app);
    submenu_add_item(
        submenu,
        app->selected_flash_options[SelectedFlashS3Mode] ? STR_FLASH_S3 : STR_FLASH,
        SubmenuIndexFlash,
        wifi_marauder_scene_flasher_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneFlasher));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewSubmenu);
}

void wifi_marauder_scene_flasher_on_enter(void* context) {
    WifiMarauderApp* app = context;

    memset(app->selected_flash_options, 0, sizeof(app->selected_flash_options));
    app->bin_file_path_boot[0] = '\0';
    app->bin_file_path_part[0] = '\0';
    app->bin_file_path_nvs[0] = '\0';
    app->bin_file_path_boot_app0[0] = '\0';
    app->bin_file_path_app[0] = '\0';
    app->bin_file_path_custom[0] = '\0';

    _refresh_submenu(app);
}

bool wifi_marauder_scene_flasher_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == WifiMarauderEventRefreshSubmenu) {
            _refresh_submenu(app);
            consumed = true;
        }
    }

    return consumed;
}

void wifi_marauder_scene_flasher_on_exit(void* context) {
    WifiMarauderApp* app = context;
    submenu_reset(app->submenu);
}
