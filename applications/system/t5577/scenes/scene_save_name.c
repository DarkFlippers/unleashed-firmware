#include "../t5577_app.h"
#include <toolbox/path.h>
#include <toolbox/name_generator.h>
#include <dolphin/dolphin.h>

static void t5577_app_scene_save_name_callback(void* context) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventConfirm);
}

// Where a save puts the file. A new dump goes to the app folder, a rename stays in the folder
// the file it renames is in: a dump can be opened from anywhere now - a subfolder, or the
// archive - and building the destination from the app folder would move it without saying so.
static FuriString* t5577_app_scene_save_name_folder(const T5577App* app) {
    FuriString* folder = furi_string_alloc();

    if(app->renaming) {
        path_extract_dirname(furi_string_get_cstr(app->path), folder);
    }

    if(furi_string_empty(folder)) {
        furi_string_set(folder, T5577_APP_FOLDER);
    }

    return folder;
}

void t5577_app_scene_save_name_on_enter(void* context) {
    T5577App* app = context;

    if(app->renaming) {
        // Start from the name being changed, which is the whole point of a rename.
        FuriString* name = furi_string_alloc();
        path_extract_filename(app->path, name, true);
        strlcpy(app->name, furi_string_get_cstr(name), sizeof(app->name));
        furi_string_free(name);
    } else {
        name_generator_make_auto(app->name, sizeof(app->name), "T5577");
    }

    text_input_set_header_text(app->text_input, "Name the dump");
    text_input_set_result_callback(
        app->text_input,
        t5577_app_scene_save_name_callback,
        app,
        app->name,
        sizeof(app->name),
        false);

    FuriString* folder = t5577_app_scene_save_name_folder(app);
    ValidatorIsFile* validator = validator_is_file_alloc_init(
        furi_string_get_cstr(folder), T5577_FILE_EXTENSION, app->name);
    text_input_set_validator(app->text_input, validator_is_file_callback, validator);
    furi_string_free(folder);

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewTextInput);
}

bool t5577_app_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventConfirm) return false;

    FuriString* folder = t5577_app_scene_save_name_folder(app);
    FuriString* path = furi_string_alloc_printf(
        "%s/%s%s", furi_string_get_cstr(folder), app->name, T5577_FILE_EXTENSION);
    furi_string_free(folder);

    bool saved = t5577_file_save(&app->dump, furi_string_get_cstr(path));

    // The old file goes only once the new one is on the card, and only when it really is another
    // file: on /ext a name that differs in case alone is the same file, and deleting it would
    // take the rename with it.
    bool old_left_behind = false;
    if(saved && app->renaming &&
       !storage_common_equivalent_path(
           app->storage, furi_string_get_cstr(app->path), furi_string_get_cstr(path))) {
        old_left_behind = !storage_simply_remove(app->storage, furi_string_get_cstr(app->path));
    }

    if(!saved) {
        dialog_message_show_storage_error(app->dialogs, "Could not write\nthe dump");
    } else {
        furi_string_set(app->path, path);

        if(old_left_behind) {
            // Both copies are on the card, and the one the user thinks is gone holds the tag's
            // password. The new name is kept, since it is the copy they asked for.
            dialog_message_show_storage_error(
                app->dialogs, "Renamed, but the old\nname is still there");
        } else {
            notification_message(app->notifications, &sequence_success);
            // A rename is not a new dump, so it is not worth anything to the dolphin.
            if(!app->renaming) dolphin_deed(DolphinDeedRfidSave);
            // A dump handed over by another app stays open under its new name: the start menu
            // is not part of that visit.
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager,
                app->renaming && app->file_only ? T5577AppSceneFileMenu : T5577AppSceneStart);
        }
    }

    furi_string_free(path);

    return true;
}

void t5577_app_scene_save_name_on_exit(void* context) {
    T5577App* app = context;

    void* validator = text_input_get_validator_callback_context(app->text_input);
    text_input_set_validator(app->text_input, NULL, NULL);
    if(validator) validator_is_file_free(validator);

    text_input_reset(app->text_input);
    app->renaming = false;
}
