#include "../archive_i.h"
#include "../helpers/archive_browser.h"
#include <storage/storage.h>

#define TAG "Archive"

void archive_scene_show_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void archive_scene_show_on_enter(void* context) {
    furi_assert(context);
    ArchiveApp* instance = context;

    FuriString* filename;
    FuriString* str_size;
    filename = furi_string_alloc();
    str_size = furi_string_alloc();

    ArchiveFile_t* current = archive_get_current_file(instance->browser);
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);
    uint32_t bytes_count;

    FileInfo fileinfo;
    storage_common_stat(fs_api, furi_string_get_cstr(current->path), &fileinfo);

    storage_file_open(file, furi_string_get_cstr(current->path), FSAM_READ, FSOM_OPEN_EXISTING);
    char* content = malloc(fileinfo.size + 1);

    bytes_count = storage_file_read(file, content, fileinfo.size);
    content[bytes_count + 1] = 0;

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, content);

    path_extract_filename(current->path, filename, false);

    // This one to return and cursor select this file
    path_extract_filename_no_ext(furi_string_get_cstr(current->path), filename);
    strlcpy(instance->text_store, furi_string_get_cstr(filename), MAX_NAME_LEN);

    free(content);
    storage_file_close(file);
    storage_file_free(file);

    furi_string_free(filename);
    furi_string_free(str_size);

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
