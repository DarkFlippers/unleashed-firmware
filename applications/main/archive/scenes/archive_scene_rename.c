#include "../archive_i.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_browser.h"
#include "archive/views/archive_browser_view.h"
#include "toolbox/path.h"

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

    string_t filename;
    string_init(filename);
    path_extract_filename(current->path, filename, true);
    strlcpy(archive->text_store, string_get_cstr(filename), MAX_NAME_LEN);

    path_extract_extension(current->path, archive->file_extension, MAX_EXT_LEN);

    text_input_set_header_text(text_input, "Rename:");

    text_input_set_result_callback(
        text_input,
        archive_scene_rename_text_input_callback,
        archive,
        archive->text_store,
        MAX_TEXT_INPUT_LEN,
        false);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        string_get_cstr(archive->browser->path), archive->file_extension, "");
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    string_clear(filename);

    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewTextInput);
}

bool archive_scene_rename_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = (ArchiveApp*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_RENAME_CUSTOM_EVENT) {
            Storage* fs_api = furi_record_open(RECORD_STORAGE);

            const char* path_src = archive_get_name(archive->browser);
            ArchiveFile_t* file = archive_get_current_file(archive->browser);

            string_t path_dst;
            string_init(path_dst);
            path_extract_dirname(path_src, path_dst);
            string_cat_printf(path_dst, "/%s%s", archive->text_store, known_ext[file->type]);

            storage_common_rename(fs_api, path_src, string_get_cstr(path_dst));
            furi_record_close(RECORD_STORAGE);

            if(file->fav) {
                archive_favorites_rename(path_src, string_get_cstr(path_dst));
            }

            string_clear(path_dst);

            scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneBrowser);
            consumed = true;
        }
    }
    return consumed;
}

void archive_scene_rename_on_exit(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;

    // Clear view
    void* validator_context = text_input_get_validator_callback_context(archive->text_input);
    text_input_set_validator(archive->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    text_input_reset(archive->text_input);
}
