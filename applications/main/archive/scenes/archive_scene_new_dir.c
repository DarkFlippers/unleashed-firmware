#include "../archive_i.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_browser.h"
#include "archive/views/archive_browser_view.h"
#include "toolbox/path.h"
#include <dialogs/dialogs.h>

#define TAG "Archive"

#define SCENE_NEW_DIR_CUSTOM_EVENT (0UL)

void archive_scene_new_dir_text_input_callback(void* context) {
    furi_assert(context);
    ArchiveApp* archive = context;
    view_dispatcher_send_custom_event(archive->view_dispatcher, SCENE_NEW_DIR_CUSTOM_EVENT);
}

void archive_scene_new_dir_on_enter(void* context) {
    ArchiveApp* archive = context;

    TextInput* text_input = archive->text_input;

    archive->text_store[0] = '\0';
    text_input_set_header_text(text_input, "New directory:");

    text_input_set_result_callback(
        text_input,
        archive_scene_new_dir_text_input_callback,
        context,
        archive->text_store,
        MAX_NAME_LEN,
        false);

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewTextInput);
}

bool archive_scene_new_dir_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_NEW_DIR_CUSTOM_EVENT) {
            FuriString* path_dst = furi_string_alloc();

            path_concat(
                furi_string_get_cstr(archive->browser->path), archive->text_store, path_dst);

            view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewStack);
            archive_show_loading_popup(archive, true);
            FS_Error error;
            if(!path_contains_only_ascii(furi_string_get_cstr(path_dst))) {
                error = FSE_INVALID_NAME;
            } else {
                Storage* fs_api = furi_record_open(RECORD_STORAGE);
                error = storage_common_mkdir(fs_api, furi_string_get_cstr(path_dst));
                furi_record_close(RECORD_STORAGE);
            }
            archive_show_loading_popup(archive, false);

            if(error != FSE_OK) {
                FuriString* dialog_msg;
                dialog_msg = furi_string_alloc();
                furi_string_cat_printf(
                    dialog_msg, "Cannot mkdir:\n%s", storage_error_get_desc(error));
                dialog_message_show_storage_error(
                    archive->dialogs, furi_string_get_cstr(dialog_msg));
                furi_string_free(dialog_msg);
            } else {
                ArchiveFile_t* current = archive_get_current_file(archive->browser);
                if(current != NULL) furi_string_set(current->path, path_dst);
            }

            furi_string_free(path_dst);
            scene_manager_previous_scene(archive->scene_manager);
            consumed = true;
        }
    }
    return consumed;
}

void archive_scene_new_dir_on_exit(void* context) {
    ArchiveApp* archive = context;
    text_input_reset(archive->text_input);
}
