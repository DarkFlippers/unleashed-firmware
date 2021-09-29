#include "../archive_i.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_browser.h"

#define SCENE_RENAME_CUSTOM_EVENT (0UL)
#define MAX_TEXT_INPUT_LEN 22

void archive_scene_rename_text_input_callback(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    view_dispatcher_send_custom_event(archive->view_dispatcher, SCENE_RENAME_CUSTOM_EVENT);
}

void archive_scene_rename_on_enter(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;

    TextInput* text_input = archive->text_input;
    ArchiveFile_t* current = archive_get_current_file(archive->browser);
    strlcpy(archive->text_store, string_get_cstr(current->name), MAX_NAME_LEN);

    archive_trim_file_path(archive->text_store, true);

    text_input_set_header_text(text_input, "Rename:");

    text_input_set_result_callback(
        text_input,
        archive_scene_rename_text_input_callback,
        archive,
        archive->text_store,
        MAX_TEXT_INPUT_LEN,
        false);

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewTextInput);
}

bool archive_scene_rename_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = (ArchiveApp*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_RENAME_CUSTOM_EVENT) {
            Storage* fs_api = furi_record_open("storage");

            string_t buffer_src;
            string_t buffer_dst;

            const char* path = archive_get_path(archive->browser);
            const char* name = archive_get_name(archive->browser);

            string_init_printf(buffer_src, "%s/%s", path, name);
            string_init_printf(buffer_dst, "%s/%s", path, archive->text_store);

            // append extension
            ArchiveFile_t* file = archive_get_current_file(archive->browser);

            string_cat(buffer_dst, known_ext[file->type]);
            storage_common_rename(
                fs_api, string_get_cstr(buffer_src), string_get_cstr(buffer_dst));
            furi_record_close("storage");

            if(file->fav) {
                archive_favorites_rename(path, name, string_get_cstr(buffer_dst));
            }

            string_clear(buffer_src);
            string_clear(buffer_dst);

            scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneBrowser);
            consumed = true;
        }
    }
    return consumed;
}

void archive_scene_rename_on_exit(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    // Clear view
    text_input_clean(archive->text_input);
}
