#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexChangeNumber,
    SubmenuIndexRename,
    SubmenuIndexDelete,
} SubmenuIndex;

static bool sim800_entry_edit_confirm_delete(Sim800App* app) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Delete entry?", 64, 4, AlignCenter, AlignTop);
    dialog_message_set_text(
        message, furi_string_get_cstr(app->entry_name), 64, 28, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, "Cancel", NULL, "Delete");

    DialogMessageButton answer = dialog_message_show(app->dialogs, message);
    dialog_message_free(message);

    return answer == DialogMessageButtonRight;
}

void sim800_scene_entry_edit_on_enter(void* context) {
    Sim800App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, furi_string_get_cstr(app->entry_name));
    submenu_add_item(
        app->submenu, "Change number", SubmenuIndexChangeNumber, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Rename", SubmenuIndexRename, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Delete", SubmenuIndexDelete, sim800_submenu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

bool sim800_scene_entry_edit_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case SubmenuIndexChangeNumber:
        app->mode = Sim800ModeEditNumber;
        scene_manager_next_scene(app->scene_manager, Sim800SceneNumberInput);
        return true;

    case SubmenuIndexRename:
        app->save_target = Sim800SaveTargetRenameEntry;
        scene_manager_next_scene(app->scene_manager, Sim800SceneSaveName);
        return true;

    case SubmenuIndexDelete:
        if(sim800_entry_edit_confirm_delete(app)) {
            sim800_notify_result(
                app, sim800_phonebook_delete(furi_string_get_cstr(app->file_path)));
            furi_string_set_str(app->file_path, SIM800_PHONEBOOK_FOLDER);
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, Sim800SceneStart);
        } else {
            view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
        }
        return true;

    default:
        return false;
    }
}

void sim800_scene_entry_edit_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
}
