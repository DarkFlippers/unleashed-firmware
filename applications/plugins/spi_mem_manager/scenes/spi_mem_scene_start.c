#include "../spi_mem_app_i.h"

typedef enum {
    SPIMemSceneStartSubmenuIndexRead,
    SPIMemSceneStartSubmenuIndexSaved,
    SPIMemSceneStartSubmenuIndexErase,
    SPIMemSceneStartSubmenuIndexWiring,
    SPIMemSceneStartSubmenuIndexAbout
} SPIMemSceneStartSubmenuIndex;

static void spi_mem_scene_start_submenu_callback(void* context, uint32_t index) {
    SPIMemApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void spi_mem_scene_start_on_enter(void* context) {
    SPIMemApp* app = context;
    submenu_add_item(
        app->submenu,
        "Read",
        SPIMemSceneStartSubmenuIndexRead,
        spi_mem_scene_start_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Saved",
        SPIMemSceneStartSubmenuIndexSaved,
        spi_mem_scene_start_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Erase",
        SPIMemSceneStartSubmenuIndexErase,
        spi_mem_scene_start_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Wiring",
        SPIMemSceneStartSubmenuIndexWiring,
        spi_mem_scene_start_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "About",
        SPIMemSceneStartSubmenuIndexAbout,
        spi_mem_scene_start_submenu_callback,
        app);
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, SPIMemSceneStart));
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewSubmenu);
}

bool spi_mem_scene_start_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, SPIMemSceneStart, event.event);
        if(event.event == SPIMemSceneStartSubmenuIndexRead) {
            app->mode = SPIMemModeRead;
            scene_manager_next_scene(app->scene_manager, SPIMemSceneChipDetect);
            success = true;
        } else if(event.event == SPIMemSceneStartSubmenuIndexSaved) {
            furi_string_set(app->file_path, SPI_MEM_FILE_FOLDER);
            scene_manager_next_scene(app->scene_manager, SPIMemSceneSelectFile);
            success = true;
        } else if(event.event == SPIMemSceneStartSubmenuIndexErase) {
            app->mode = SPIMemModeErase;
            scene_manager_next_scene(app->scene_manager, SPIMemSceneChipDetect);
            success = true;
        } else if(event.event == SPIMemSceneStartSubmenuIndexWiring) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneWiring);
            success = true;
        } else if(event.event == SPIMemSceneStartSubmenuIndexAbout) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneAbout);
            success = true;
        }
    }
    return success;
}

void spi_mem_scene_start_on_exit(void* context) {
    SPIMemApp* app = context;
    submenu_reset(app->submenu);
}
