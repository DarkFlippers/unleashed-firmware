#include "../sim800_app_i.h"

void sim800_scene_saved_on_enter(void* context) {
    Sim800App* app = context;

    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, SIM800_PHONE_EXTENSION, &I_phone_10px);
    options.base_path = SIM800_PHONEBOOK_FOLDER;
    options.hide_ext = true;

    if(furi_string_size(app->file_path) == 0) {
        furi_string_set_str(app->file_path, SIM800_PHONEBOOK_FOLDER);
    }

    if(dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &options) &&
       sim800_phonebook_load(furi_string_get_cstr(app->file_path), app->entry_name, app->number)) {
        app->number_saved = true;
        app->redial = false;
        scene_manager_next_scene(app->scene_manager, Sim800SceneSavedMenu);
        return;
    }

    furi_string_set_str(app->file_path, SIM800_PHONEBOOK_FOLDER);
    scene_manager_previous_scene(app->scene_manager);
}

bool sim800_scene_saved_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void sim800_scene_saved_on_exit(void* context) {
    UNUSED(context);
}
