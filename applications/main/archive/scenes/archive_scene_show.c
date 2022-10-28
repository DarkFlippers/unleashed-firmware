#include "../archive_i.h"
#include "../helpers/archive_browser.h"
#include <storage/storage.h>

#define TAG "Archive"

#define SHOW_MAX_FILE_SIZE 5000

void archive_scene_show_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

static bool text_show_read_lines(File* file, FuriString* str_result) {
    //furi_string_reset(str_result);
    uint8_t buffer[SHOW_MAX_FILE_SIZE];

    uint16_t read_count = storage_file_read(file, buffer, SHOW_MAX_FILE_SIZE);
    if(storage_file_get_error(file) != FSE_OK) {
        return false;
    }

    for(uint16_t i = 0; i < read_count; i++) {
        furi_string_push_back(str_result, buffer[i]);
    }

    return true;
}

void archive_scene_show_on_enter(void* context) {
    furi_assert(context);
    ArchiveApp* instance = context;

    FuriString* filename;
    filename = furi_string_alloc();

    FuriString* buffer;
    buffer = furi_string_alloc();

    ArchiveFile_t* current = archive_get_current_file(instance->browser);
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    FileInfo fileinfo;
    FS_Error error = storage_common_stat(fs_api, furi_string_get_cstr(current->path), &fileinfo);
    if(error == FSE_OK) {
        if((fileinfo.size < SHOW_MAX_FILE_SIZE) && (fileinfo.size > 2)) {
            bool ok = storage_file_open(
                file, furi_string_get_cstr(current->path), FSAM_READ, FSOM_OPEN_EXISTING);
            if(ok) {
                if(!text_show_read_lines(file, buffer)) {
                    goto text_file_read_err;
                }
                if(!furi_string_size(buffer)) {
                    goto text_file_read_err;
                }

                storage_file_seek(file, 0, true);

                widget_add_text_scroll_element(
                    instance->widget, 0, 0, 128, 64, furi_string_get_cstr(buffer));

            } else {
            text_file_read_err:
                widget_add_text_box_element(
                    instance->widget,
                    0,
                    0,
                    128,
                    64,
                    AlignLeft,
                    AlignCenter,
                    "\e#Error:\nStorage file open error\e#",
                    false);
            }
            storage_file_close(file);
        } else if(fileinfo.size < 2) {
            widget_add_text_box_element(
                instance->widget,
                0,
                0,
                128,
                64,
                AlignLeft,
                AlignCenter,
                "\e#Error:\nFile is too small\e#",
                false);
        } else {
            widget_add_text_box_element(
                instance->widget,
                0,
                0,
                128,
                64,
                AlignLeft,
                AlignCenter,
                "\e#Error:\nFile is too large to show\e#",
                false);
        }
    } else {
        widget_add_text_box_element(
            instance->widget,
            0,
            0,
            128,
            64,
            AlignLeft,
            AlignCenter,
            "\e#Error:\nFile system error\e#",
            false);
    }
    path_extract_filename(current->path, filename, false);

    // This one to return and cursor select this file
    path_extract_filename_no_ext(furi_string_get_cstr(current->path), filename);
    strlcpy(instance->text_store, furi_string_get_cstr(filename), MAX_NAME_LEN);

    furi_string_free(buffer);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    furi_string_free(filename);

    view_dispatcher_switch_to_view(instance->view_dispatcher, ArchiveViewWidget);
}

bool archive_scene_show_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_next_scene(app->scene_manager, ArchiveAppSceneBrowser);
        return true;
    }
    return false;
}

void archive_scene_show_on_exit(void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    widget_reset(app->widget);
}
