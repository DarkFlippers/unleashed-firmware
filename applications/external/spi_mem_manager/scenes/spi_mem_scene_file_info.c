#include "../spi_mem_app_i.h"
#include "../spi_mem_files.h"

void spi_mem_scene_file_info_on_enter(void* context) {
    SPIMemApp* app = context;
    FuriString* str = furi_string_alloc();
    furi_string_printf(str, "Size: %zu KB", spi_mem_file_get_size(app) / 1024);
    widget_add_string_element(
        app->widget, 64, 9, AlignCenter, AlignBottom, FontPrimary, "File info");
    widget_add_string_element(
        app->widget, 64, 20, AlignCenter, AlignBottom, FontSecondary, furi_string_get_cstr(str));
    furi_string_free(str);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewWidget);
}

bool spi_mem_scene_file_info_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, SPIMemSceneSavedFileMenu);
    }
    return success;
}
void spi_mem_scene_file_info_on_exit(void* context) {
    SPIMemApp* app = context;
    widget_reset(app->widget);
}
