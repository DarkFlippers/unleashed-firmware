#include "../bad_ble_app_i.h"
#include <furi_hal_power.h>
#include <storage/storage.h>

static bool bad_ble_file_select(BadBleApp* bad_ble) {
    furi_assert(bad_ble);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, BAD_BLE_APP_SCRIPT_EXTENSION, &I_badusb_10px);
    browser_options.base_path = BAD_BLE_APP_BASE_FOLDER;
    browser_options.skip_assets = true;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        bad_ble->dialogs, bad_ble->file_path, bad_ble->file_path, &browser_options);

    return res;
}

void bad_ble_scene_file_select_on_enter(void* context) {
    BadBleApp* bad_ble = context;

    if(bad_ble->bad_ble_script) {
        bad_ble_script_close(bad_ble->bad_ble_script);
        bad_ble->bad_ble_script = NULL;
    }

    if(bad_ble_file_select(bad_ble)) {
        scene_manager_next_scene(bad_ble->scene_manager, BadBleSceneWork);
    } else {
        view_dispatcher_stop(bad_ble->view_dispatcher);
    }
}

bool bad_ble_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // BadBleApp* bad_ble = context;
    return false;
}

void bad_ble_scene_file_select_on_exit(void* context) {
    UNUSED(context);
    // BadBleApp* bad_ble = context;
}
