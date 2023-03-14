#include "../spi_mem_app_i.h"
#include "../spi_mem_files.h"

static void spi_mem_scene_delete_confirm_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    SPIMemApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void spi_mem_scene_delete_confirm_on_enter(void* context) {
    SPIMemApp* app = context;
    FuriString* file_name = furi_string_alloc();
    FuriString* message = furi_string_alloc();
    path_extract_filename(app->file_path, file_name, true);
    furi_string_printf(message, "\e#Delete %s?\e#", furi_string_get_cstr(file_name));
    widget_add_text_box_element(
        app->widget, 0, 0, 128, 27, AlignCenter, AlignCenter, furi_string_get_cstr(message), true);
    widget_add_button_element(
        app->widget,
        GuiButtonTypeLeft,
        "Cancel",
        spi_mem_scene_delete_confirm_widget_callback,
        app);
    widget_add_button_element(
        app->widget,
        GuiButtonTypeRight,
        "Delete",
        spi_mem_scene_delete_confirm_widget_callback,
        app);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewWidget);
    furi_string_free(file_name);
    furi_string_free(message);
}

bool spi_mem_scene_delete_confirm_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == GuiButtonTypeRight) {
            app->mode = SPIMemModeDelete;
            if(spi_mem_file_delete(app)) {
                scene_manager_next_scene(app->scene_manager, SPIMemSceneSuccess);
            } else {
                scene_manager_next_scene(app->scene_manager, SPIMemSceneStorageError);
            }
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, SPIMemSceneSavedFileMenu);
        }
    }
    return success;
}

void spi_mem_scene_delete_confirm_on_exit(void* context) {
    SPIMemApp* app = context;
    widget_reset(app->widget);
}
