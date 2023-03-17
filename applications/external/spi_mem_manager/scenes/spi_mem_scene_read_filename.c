#include "../spi_mem_app_i.h"
#include "../spi_mem_files.h"

void spi_mem_scene_read_filename_view_result_callback(void* context) {
    SPIMemApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SPIMemCustomEventTextEditResult);
}

void spi_mem_scene_read_set_random_filename(SPIMemApp* app) {
    if(furi_string_end_with(app->file_path, SPI_MEM_FILE_EXTENSION)) {
        size_t filename_start = furi_string_search_rchar(app->file_path, '/');
        furi_string_left(app->file_path, filename_start);
    }
    set_random_name(app->text_buffer, SPI_MEM_TEXT_BUFFER_SIZE);
}

void spi_mem_scene_read_filename_on_enter(void* context) {
    SPIMemApp* app = context;
    spi_mem_scene_read_set_random_filename(app);
    text_input_set_header_text(app->text_input, "Name the dump");
    text_input_set_result_callback(
        app->text_input,
        spi_mem_scene_read_filename_view_result_callback,
        app,
        app->text_buffer,
        SPI_MEM_FILE_NAME_SIZE,
        true);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewTextInput);
}

bool spi_mem_scene_read_filename_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    UNUSED(app);
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == SPIMemCustomEventTextEditResult) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneRead);
        }
    }
    return success;
}
void spi_mem_scene_read_filename_on_exit(void* context) {
    SPIMemApp* app = context;
    text_input_reset(app->text_input);
}
