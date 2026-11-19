#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexType,
    SubmenuIndexSaved,
} SubmenuIndex;

/** Browse the phonebook and load whatever is picked into the app's number. */
static bool sim800_number_source_pick(Sim800App* app) {
    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, SIM800_PHONE_EXTENSION, &I_phone_10px);
    options.base_path = SIM800_PHONEBOOK_FOLDER;
    options.hide_ext = true;

    // Its own string: app->file_path may still be holding the preset we are sending
    FuriString* picked = furi_string_alloc_set_str(SIM800_PHONEBOOK_FOLDER);
    bool chosen =
        dialog_file_browser_show(app->dialogs, picked, picked, &options) &&
        sim800_phonebook_load(furi_string_get_cstr(picked), app->entry_name, app->number);

    if(chosen) furi_string_set(app->file_path, picked);
    furi_string_free(picked);
    return chosen;
}

void sim800_scene_number_source_on_enter(void* context) {
    Sim800App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Number");
    submenu_add_item(
        app->submenu, "Type a number", SubmenuIndexType, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Saved numbers", SubmenuIndexSaved, sim800_submenu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

bool sim800_scene_number_source_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == SubmenuIndexType) {
        scene_manager_next_scene(app->scene_manager, Sim800SceneNumberInput);
        return true;
    }

    if(event.event == SubmenuIndexSaved) {
        if(sim800_number_source_pick(app)) {
            app->number_saved = true;
            app->redial = false;
            sim800_number_chosen(app);
        }
        return true;
    }

    return false;
}

void sim800_scene_number_source_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
}
