#include "../archive_i.h"
#include "../helpers/archive_browser.h"

#define TAG "Archive"

void archive_scene_info_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void archive_scene_info_on_enter(void* context) {
    furi_assert(context);
    ArchiveApp* instance = context;

    widget_add_button_element(
        instance->widget, GuiButtonTypeLeft, "Back", archive_scene_info_widget_callback, instance);

    FuriString* filename;
    FuriString* dirname;
    FuriString* str_size;
    filename = furi_string_alloc();
    dirname = furi_string_alloc();
    str_size = furi_string_alloc();

    ArchiveFile_t* current = archive_get_current_file(instance->browser);
    char file_info_message[128];
    Storage* fs_api = furi_record_open(RECORD_STORAGE);

    // Filename
    path_extract_filename(current->path, filename, false);
    snprintf(
        file_info_message, sizeof(file_info_message), "\e#%s\e#", furi_string_get_cstr(filename));
    widget_add_text_box_element(
        instance->widget, 0, 0, 128, 25, AlignLeft, AlignCenter, file_info_message, false);

    // Directory path
    path_extract_dirname(furi_string_get_cstr(current->path), dirname);
    if(strcmp(furi_string_get_cstr(dirname), "/any") == 0) {
        furi_string_replace(dirname, STORAGE_ANY_PATH_PREFIX, "/");
    } else {
        furi_string_replace(dirname, STORAGE_ANY_PATH_PREFIX, "");
    }

    // File size
    FileInfo fileinfo;
    storage_common_stat(fs_api, furi_string_get_cstr(current->path), &fileinfo);
    if(fileinfo.size <= 1024) {
        furi_string_printf(str_size, "%d", fileinfo.size);
        snprintf(
            file_info_message,
            sizeof(file_info_message),
            "Size: \e#%s\e# bytes\n%s",
            furi_string_get_cstr(str_size),
            furi_string_get_cstr(dirname));
    } else {
        furi_string_printf(str_size, "%d", fileinfo.size / 1024);
        snprintf(
            file_info_message,
            sizeof(file_info_message),
            "Size: \e#%s\e# Kb.\n%s",
            furi_string_get_cstr(str_size),
            furi_string_get_cstr(dirname));
    }
    widget_add_text_box_element(
        instance->widget, 0, 25, 128, 25, AlignLeft, AlignCenter, file_info_message, true);

    // This one to return and cursor select this file
    path_extract_filename_no_ext(furi_string_get_cstr(current->path), filename);
    strlcpy(instance->text_store, furi_string_get_cstr(filename), MAX_NAME_LEN);

    furi_string_free(filename);
    furi_string_free(dirname);
    furi_string_free(str_size);

    view_dispatcher_switch_to_view(instance->view_dispatcher, ArchiveViewWidget);
}

bool archive_scene_info_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_next_scene(app->scene_manager, ArchiveAppSceneBrowser);
        return true;
    }
    return false;
}

void archive_scene_info_on_exit(void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    widget_reset(app->widget);
}
