#include "../storage_settings.h"

enum StorageSettingsStartSubmenuIndex {
    StorageSettingsStartSubmenuIndexInternalInfo,
    StorageSettingsStartSubmenuIndexSDInfo,
    StorageSettingsStartSubmenuIndexUnmount,
    StorageSettingsStartSubmenuIndexFormat,
    StorageSettingsStartSubmenuIndexBenchy,
    StorageSettingsStartSubmenuIndexFactoryReset
};

static void storage_settings_scene_start_submenu_callback(void* context, uint32_t index) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void storage_settings_scene_start_on_enter(void* context) {
    StorageSettings* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "About Internal Storage",
        StorageSettingsStartSubmenuIndexInternalInfo,
        storage_settings_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "About SD Card",
        StorageSettingsStartSubmenuIndexSDInfo,
        storage_settings_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Unmount SD Card",
        StorageSettingsStartSubmenuIndexUnmount,
        storage_settings_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Format SD Card",
        StorageSettingsStartSubmenuIndexFormat,
        storage_settings_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Benchmark SD Card",
        StorageSettingsStartSubmenuIndexBenchy,
        storage_settings_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Factory Reset",
        StorageSettingsStartSubmenuIndexFactoryReset,
        storage_settings_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, StorageSettingsStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewSubmenu);
}

bool storage_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case StorageSettingsStartSubmenuIndexSDInfo:
            scene_manager_set_scene_state(
                app->scene_manager, StorageSettingsStart, StorageSettingsStartSubmenuIndexSDInfo);
            scene_manager_next_scene(app->scene_manager, StorageSettingsSDInfo);
            consumed = true;
            break;
        case StorageSettingsStartSubmenuIndexInternalInfo:
            scene_manager_set_scene_state(
                app->scene_manager,
                StorageSettingsStart,
                StorageSettingsStartSubmenuIndexInternalInfo);
            scene_manager_next_scene(app->scene_manager, StorageSettingsInternalInfo);
            consumed = true;
            break;
        case StorageSettingsStartSubmenuIndexUnmount:
            scene_manager_set_scene_state(
                app->scene_manager, StorageSettingsStart, StorageSettingsStartSubmenuIndexUnmount);
            scene_manager_next_scene(app->scene_manager, StorageSettingsUnmountConfirm);
            consumed = true;
            break;
        case StorageSettingsStartSubmenuIndexFormat:
            scene_manager_set_scene_state(
                app->scene_manager, StorageSettingsStart, StorageSettingsStartSubmenuIndexFormat);
            scene_manager_next_scene(app->scene_manager, StorageSettingsFormatConfirm);
            consumed = true;
            break;
        case StorageSettingsStartSubmenuIndexBenchy:
            scene_manager_set_scene_state(
                app->scene_manager, StorageSettingsStart, StorageSettingsStartSubmenuIndexBenchy);
            scene_manager_next_scene(app->scene_manager, StorageSettingsBenchmark);
            consumed = true;
            break;
        case StorageSettingsStartSubmenuIndexFactoryReset:
            scene_manager_set_scene_state(
                app->scene_manager,
                StorageSettingsStart,
                StorageSettingsStartSubmenuIndexFactoryReset);
            scene_manager_next_scene(app->scene_manager, StorageSettingsFactoryReset);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void storage_settings_scene_start_on_exit(void* context) {
    StorageSettings* app = context;
    submenu_reset(app->submenu);
}
