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

    FuriString* path_name = furi_string_alloc();
    FuriString* path_folder = furi_string_alloc();

    if(current->type == ArchiveFileTypeFolder) {
        // Set file ext to empty since we need to see folder name here
        strcpy(archive->file_extension, "");
        // Extract folder name and copy into text_store
        path_extract_basename(furi_string_get_cstr(current->path), path_name);
        strlcpy(archive->text_store, furi_string_get_cstr(path_name), MAX_NAME_LEN);
        text_input_set_header_text(text_input, "Rename directory:");
    } else /*if(current->type != ArchiveFileTypeUnknown) */ {
        // Extract file name and copy into text_store
        path_extract_filename(current->path, path_name, true);
        strlcpy(archive->text_store, furi_string_get_cstr(path_name), MAX_NAME_LEN);
        // Extract file extension for validator and rename func
        path_extract_extension(current->path, archive->file_extension, MAX_EXT_LEN);
        text_input_set_header_text(text_input, "Rename file:");
    } /*else {
        path_extract_filename(current->path, path_name, false);
        strlcpy(archive->text_store, furi_string_get_cstr(path_name), MAX_NAME_LEN);
        text_input_set_header_text(text_input, "Rename unknown file:");
    }*/

    // Get current folder (for file) or previous folder (for folder) for validator
    path_extract_dirname(furi_string_get_cstr(current->path), path_folder);

    text_input_set_result_callback(
        text_input,
        archive_scene_rename_text_input_callback,
        context,
        archive->text_store,
        MAX_TEXT_INPUT_LEN,
        false);

    // Init validator to show message to user that name already exist
    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        furi_string_get_cstr(path_folder), archive->file_extension, archive->text_store);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    furi_string_free(path_name);
    furi_string_free(path_folder);

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewTextInput);
}

bool archive_scene_rename_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_RENAME_CUSTOM_EVENT) {
            const char* path_src = archive_get_name(archive->browser);
            ArchiveFile_t* file = archive_get_current_file(archive->browser);

            FuriString* path_dst;

            path_dst = furi_string_alloc();

            if(file->type == ArchiveFileTypeFolder) {
                // Rename folder/dir
                path_extract_dirname(path_src, path_dst);
                furi_string_cat_printf(path_dst, "/%s", archive->text_store);
            } else if(file->type != ArchiveFileTypeUnknown) {
                // Rename known type
                path_extract_dirname(path_src, path_dst);
                furi_string_cat_printf(
                    path_dst, "/%s%s", archive->text_store, known_ext[file->type]);
            } else {
                // Rename unknown type
                path_extract_dirname(path_src, path_dst);
                furi_string_cat_printf(
                    path_dst, "/%s%s", archive->text_store, archive->file_extension);
            }
            // Long time process if this is directory
            view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewStack);
            archive_show_loading_popup(archive, true);
            FS_Error error = archive_rename_copy_file_or_dir(
                archive->browser, path_src, furi_string_get_cstr(path_dst), false);
            archive_show_loading_popup(archive, false);
            archive_show_file_menu(archive->browser, false);

            furi_string_free(path_dst);

            if(error == FSE_OK || error == FSE_EXIST) {
                scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneBrowser);
            } else {
                FuriString* dialog_msg;
                dialog_msg = furi_string_alloc();
                furi_string_cat_printf(dialog_msg, "Cannot rename\nCode: %d", error);
                dialog_message_show_storage_error(
                    archive->dialogs, furi_string_get_cstr(dialog_msg));
                furi_string_free(dialog_msg);
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
