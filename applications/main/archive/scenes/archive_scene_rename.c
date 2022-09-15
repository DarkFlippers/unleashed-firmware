#include "../archive_i.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_browser.h"
#include "archive/views/archive_browser_view.h"
#include "toolbox/path.h"
#include <dialogs/dialogs.h>

#define TAG "Archive"

#define SCENE_RENAME_CUSTOM_EVENT (0UL)
#define MAX_TEXT_INPUT_LEN 22

void archive_scene_rename_text_input_callback(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    view_dispatcher_send_custom_event(archive->view_dispatcher, SCENE_RENAME_CUSTOM_EVENT);
}

void archive_scene_rename_on_enter(void* context) {
    ArchiveApp* archive = context;

    TextInput* text_input = archive->text_input;
    ArchiveFile_t* current = archive_get_current_file(archive->browser);

    string_t path_name;
    string_init(path_name);

    if(current->type == ArchiveFileTypeFolder) {
        path_extract_basename(string_get_cstr(current->path), path_name);
        archive_text_store_set(archive, string_get_cstr(path_name));
        text_input_set_header_text(text_input, "Rename directory:");
    } else /*if(current->type != ArchiveFileTypeUnknown) */ {
        path_extract_filename(current->path, path_name, true);
        archive_text_store_set(archive, string_get_cstr(path_name));

        path_extract_extension(current->path, archive->file_extension, MAX_EXT_LEN);
        text_input_set_header_text(text_input, "Rename file:");
    } /*else {
        path_extract_filename(current->path, path_name, false);
        strlcpy(archive->text_store, string_get_cstr(path_name), MAX_NAME_LEN);
        text_input_set_header_text(text_input, "Rename unknown file:");
    }*/

    text_input_set_result_callback(
        text_input,
        archive_scene_rename_text_input_callback,
        context,
        archive->text_store,
        MAX_TEXT_INPUT_LEN,
        false);

    string_clear(path_name);

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewTextInput);
}

bool archive_scene_rename_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_RENAME_CUSTOM_EVENT) {
            const char* path_src = archive_get_name(archive->browser);
            ArchiveFile_t* file = archive_get_current_file(archive->browser);

            string_t path_dst;
            string_init(path_dst);

            if(file->type == ArchiveFileTypeFolder) {
                // Rename folder/dir
                path_extract_dirname(path_src, path_dst);
                string_cat_printf(path_dst, "/%s", archive->text_store);
            } else if(file->type != ArchiveFileTypeUnknown) {
                // Rename known type
                path_extract_dirname(path_src, path_dst);
                string_cat_printf(path_dst, "/%s%s", archive->text_store, known_ext[file->type]);
            } else {
                // Rename unknown type
                path_extract_dirname(path_src, path_dst);
                string_cat_printf(path_dst, "/%s%s", archive->text_store, archive->file_extension);
            }
            // Long time process if this is directory
            view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewStack);
            archive_show_loading_popup(archive, true);
            FS_Error error =
                archive_rename_file_or_dir(archive->browser, path_src, string_get_cstr(path_dst));
            archive_show_loading_popup(archive, false);
            archive_show_file_menu(archive->browser, false);

            string_clear(path_dst);

            if(error == FSE_OK || error == FSE_EXIST) {
                scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneBrowser);
            } else {
                string_t dialog_msg;
                string_init(dialog_msg);
                string_cat_printf(dialog_msg, "Cannot rename\nCode: %d", error);
                dialog_message_show_storage_error(archive->dialogs, string_get_cstr(dialog_msg));
                string_clear(dialog_msg);
            }
            consumed = true;
        }
    }
    return consumed;
}

void archive_scene_rename_on_exit(void* context) {
    ArchiveApp* archive = context;
    text_input_reset(archive->text_input);
}
