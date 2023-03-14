#include "../spi_mem_app_i.h"
#include "../spi_mem_files.h"

void spi_mem_scene_select_file_on_enter(void* context) {
    SPIMemApp* app = context;
    if(spi_mem_file_select(app)) {
        scene_manager_set_scene_state(app->scene_manager, SPIMemSceneSavedFileMenu, 0);
        scene_manager_next_scene(app->scene_manager, SPIMemSceneSavedFileMenu);
    } else {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SPIMemSceneStart);
    }
}

bool spi_mem_scene_select_file_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void spi_mem_scene_select_file_on_exit(void* context) {
    UNUSED(context);
}
