#include "../storage_settings.h"

void storage_settings_scene_start_on_enter(void* context) {
    StorageSettings* app = context;

    FS_Error sd_status = storage_sd_status(app->fs_api);
    app->helper_descriptor->options[STORAGE_SETTINGS_MOUNT_INDEX].name =
        (sd_status != FSE_OK) ? "Mount SD Card" : "Unmount SD Card";
    submenu_settings_helpers_scene_enter(app->settings_helper);
}

bool storage_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    return submenu_settings_helpers_scene_event(app->settings_helper, event);
}

void storage_settings_scene_start_on_exit(void* context) {
    StorageSettings* app = context;
    submenu_settings_helpers_scene_exit(app->settings_helper);
}
