#include "../spi_mem_app_i.h"

typedef enum {
    SPIMemSceneSavedFileMenuSubmenuIndexWrite,
    SPIMemSceneSavedFileMenuSubmenuIndexCompare,
    SPIMemSceneSavedFileMenuSubmenuIndexInfo,
    SPIMemSceneSavedFileMenuSubmenuIndexDelete,
} SPIMemSceneSavedFileMenuSubmenuIndex;

static void spi_mem_scene_saved_file_menu_submenu_callback(void* context, uint32_t index) {
    SPIMemApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void spi_mem_scene_saved_file_menu_on_enter(void* context) {
    SPIMemApp* app = context;
    submenu_add_item(
        app->submenu,
        "Write",
        SPIMemSceneSavedFileMenuSubmenuIndexWrite,
        spi_mem_scene_saved_file_menu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Compare",
        SPIMemSceneSavedFileMenuSubmenuIndexCompare,
        spi_mem_scene_saved_file_menu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Info",
        SPIMemSceneSavedFileMenuSubmenuIndexInfo,
        spi_mem_scene_saved_file_menu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Delete",
        SPIMemSceneSavedFileMenuSubmenuIndexDelete,
        spi_mem_scene_saved_file_menu_submenu_callback,
        app);
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, SPIMemSceneSavedFileMenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewSubmenu);
}

bool spi_mem_scene_saved_file_menu_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, SPIMemSceneSavedFileMenu, event.event);
        if(event.event == SPIMemSceneSavedFileMenuSubmenuIndexWrite) {
            app->mode = SPIMemModeWrite;
            scene_manager_next_scene(app->scene_manager, SPIMemSceneChipDetect);
            success = true;
        }
        if(event.event == SPIMemSceneSavedFileMenuSubmenuIndexCompare) {
            app->mode = SPIMemModeCompare;
            scene_manager_next_scene(app->scene_manager, SPIMemSceneChipDetect);
            success = true;
        }
        if(event.event == SPIMemSceneSavedFileMenuSubmenuIndexDelete) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneDeleteConfirm);
            success = true;
        }
        if(event.event == SPIMemSceneSavedFileMenuSubmenuIndexInfo) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneFileInfo);
            success = true;
        }
    }
    return success;
}

void spi_mem_scene_saved_file_menu_on_exit(void* context) {
    SPIMemApp* app = context;
    submenu_reset(app->submenu);
}
