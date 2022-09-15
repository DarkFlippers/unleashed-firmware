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
    ArchiveApp* app = (ArchiveApp*)context;

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Back", archive_scene_info_widget_callback, app);

    string_t filename;
    string_t dirname;
    string_t str_size;
    string_init(filename);
    string_init(dirname);
    string_init(str_size);

    ArchiveFile_t* current = archive_get_current_file(app->browser);
    char file_info_message[128];
    Storage* fs_api = furi_record_open(RECORD_STORAGE);

    // Filename
    path_extract_filename(current->path, filename, false);
    snprintf(file_info_message, sizeof(file_info_message), "\e#%s\e#", string_get_cstr(filename));
    widget_add_text_box_element(
        app->widget, 0, 0, 128, 20, AlignLeft, AlignCenter, file_info_message, false);

    // File size
    FileInfo fileinfo;
    storage_common_stat(fs_api, string_get_cstr(current->path), &fileinfo);
    string_printf(str_size, "%d", fileinfo.size / 1024);
    snprintf(
        file_info_message,
        sizeof(file_info_message),
        "Size: \e#%s\e# Kb.",
        string_get_cstr(str_size));
    widget_add_text_box_element(
        app->widget, 0, 23, 128, 20, AlignLeft, AlignCenter, file_info_message, false);

    // Directory path
    path_extract_dirname(string_get_cstr(current->path), dirname);
    string_replace_str(dirname, STORAGE_ANY_PATH_PREFIX, "");
    widget_add_text_box_element(
        app->widget, 0, 23, 128, 20, AlignLeft, AlignCenter, string_get_cstr(dirname), false);

    string_clear(filename);
    string_clear(dirname);
    string_clear(str_size);

    view_dispatcher_switch_to_view(app->view_dispatcher, ArchiveViewWidget);
}

bool archive_scene_info_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        return scene_manager_previous_scene(app->scene_manager);
    }
    return false;
}

void archive_scene_info_on_exit(void* context) {
    furi_assert(context);
    ArchiveApp* app = (ArchiveApp*)context;

    widget_reset(app->widget);
}
