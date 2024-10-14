#include "../bad_ble_app_i.h"
#include <storage/storage.h>

static bool bad_ble_layout_select(BadBleApp* bad_ble) {
    furi_assert(bad_ble);

    FuriString* predefined_path;
    predefined_path = furi_string_alloc();
    if(!furi_string_empty(bad_ble->keyboard_layout)) {
        furi_string_set(predefined_path, bad_ble->keyboard_layout);
    } else {
        furi_string_set(predefined_path, BAD_BLE_APP_PATH_LAYOUT_FOLDER);
    }

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, BAD_BLE_APP_LAYOUT_EXTENSION, &I_keyboard_10px);
    browser_options.base_path = BAD_BLE_APP_PATH_LAYOUT_FOLDER;
    browser_options.skip_assets = false;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        bad_ble->dialogs, bad_ble->keyboard_layout, predefined_path, &browser_options);

    furi_string_free(predefined_path);
    return res;
}

void bad_ble_scene_config_layout_on_enter(void* context) {
    BadBleApp* bad_ble = context;

    if(bad_ble_layout_select(bad_ble)) {
        scene_manager_search_and_switch_to_previous_scene(bad_ble->scene_manager, BadBleSceneWork);
    } else {
        scene_manager_previous_scene(bad_ble->scene_manager);
    }
}

bool bad_ble_scene_config_layout_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // BadBleApp* bad_ble = context;
    return false;
}

void bad_ble_scene_config_layout_on_exit(void* context) {
    UNUSED(context);
    // BadBleApp* bad_ble = context;
}
