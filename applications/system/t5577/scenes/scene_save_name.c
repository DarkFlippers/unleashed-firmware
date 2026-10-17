#include "../t5577_app.h"
#include <toolbox/path.h>
#include <toolbox/name_generator.h>
#include <dolphin/dolphin.h>

static void t5577_app_scene_save_name_callback(void* context) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventConfirm);
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

    ValidatorIsFile* validator =
        validator_is_file_alloc_init(T5577_APP_FOLDER, T5577_FILE_EXTENSION, app->name);
    text_input_set_validator(app->text_input, validator_is_file_callback, validator);

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewTextInput);
}

static void t5577_app_scene_save_name_failed(T5577App* app) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Not saved", 64, 10, AlignCenter, AlignTop);
    dialog_message_set_text(
        message, "Could not write\nto the SD card", 64, 32, AlignCenter, AlignTop);
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

bool t5577_app_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event != T5577AppCustomEventConfirm) return false;

    FuriString* path =
        furi_string_alloc_printf("%s/%s%s", T5577_APP_FOLDER, app->name, T5577_FILE_EXTENSION);

    bool saved = t5577_file_save(&app->dump, furi_string_get_cstr(path));

    // The old file goes only once the new one is on the card, and only when it really is another
    // file: on /ext a name that differs in case alone is the same file, and deleting it would
    // take the rename with it.
    if(saved && app->renaming &&
       !storage_common_equivalent_path(
           app->storage, furi_string_get_cstr(app->path), furi_string_get_cstr(path))) {
        storage_simply_remove(app->storage, furi_string_get_cstr(app->path));
    }

    if(saved) {
        furi_string_set(app->path, path);
        notification_message(app->notifications, &sequence_success);
        // A rename is not a new dump, so it is not worth anything to the dolphin.
        if(!app->renaming) dolphin_deed(DolphinDeedRfidSave);
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
    } else {
        t5577_app_scene_save_name_failed(app);
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
