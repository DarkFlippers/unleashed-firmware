#include "spi_mem_app_i.h"

void spi_mem_file_create_folder(SPIMemApp* app) {
    if(!storage_simply_mkdir(app->storage, SPI_MEM_FILE_FOLDER)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot create\napp folder");
    }
}

bool spi_mem_file_delete(SPIMemApp* app) {
    return (storage_simply_remove(app->storage, furi_string_get_cstr(app->file_path)));
}

bool spi_mem_file_select(SPIMemApp* app) {
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SPI_MEM_FILE_EXTENSION, &I_Dip8_10px);
    browser_options.base_path = SPI_MEM_FILE_FOLDER;
    bool success =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options);
    return success;
}

bool spi_mem_file_create_open(SPIMemApp* app) {
    bool success = false;
    app->file = storage_file_alloc(app->storage);
    do {
        if(furi_string_end_with(app->file_path, SPI_MEM_FILE_EXTENSION)) {
            if(!spi_mem_file_delete(app)) break;
            size_t filename_start = furi_string_search_rchar(app->file_path, '/');
            furi_string_left(app->file_path, filename_start);
        }
        furi_string_cat_printf(app->file_path, "/%s%s", app->text_buffer, SPI_MEM_FILE_EXTENSION);
        if(!storage_file_open(
               app->file, furi_string_get_cstr(app->file_path), FSAM_WRITE, FSOM_CREATE_NEW))
            break;
        success = true;
    } while(0);
    if(!success) { //-V547
        dialog_message_show_storage_error(app->dialogs, "Cannot save\nfile");
    }
    return success;
}

bool spi_mem_file_open(SPIMemApp* app) {
    app->file = storage_file_alloc(app->storage);
    if(!storage_file_open(
           app->file, furi_string_get_cstr(app->file_path), FSAM_READ_WRITE, FSOM_OPEN_EXISTING)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot save\nfile");
        return false;
    }
    return true;
}

bool spi_mem_file_write_block(SPIMemApp* app, uint8_t* data, size_t size) {
    if(storage_file_write(app->file, data, size) != size) return false;
    return true;
}

bool spi_mem_file_read_block(SPIMemApp* app, uint8_t* data, size_t size) {
    if(storage_file_read(app->file, data, size) != size) return false;
    return true;
}

void spi_mem_file_close(SPIMemApp* app) {
    storage_file_close(app->file);
    storage_file_free(app->file);
}

size_t spi_mem_file_get_size(SPIMemApp* app) {
    FileInfo file_info;
    if(storage_common_stat(app->storage, furi_string_get_cstr(app->file_path), &file_info) !=
       FSE_OK)
        return 0;
    return file_info.size;
}
