#include "../infrared_app_i.h"

#include "common/infrared_scene_universal_common.h"

typedef enum {
    SubmenuIndexSaveAsNewRemote,
    SubmenuIndexAddToExistingRemote,
} SubmenuIndex;

static void infrared_scene_universal_save_submenu_callback(void* context, uint32_t index) {
    InfraredApp* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, index);
}

static void infrared_scene_universal_save_add_to_existing(InfraredApp* infrared) {
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, INFRARED_APP_EXTENSION, &I_ir_10px);
    browser_options.base_path = INFRARED_APP_FOLDER;

    // The browser opens at whatever the path holds, and an empty one lands at /ext.
    if(furi_string_empty(infrared->file_path)) {
        furi_string_set(infrared->file_path, INFRARED_APP_FOLDER);
    }

    // Backing out of the browser just puts the user back on this menu.
    if(!dialog_file_browser_show(
           infrared->dialogs, infrared->file_path, infrared->file_path, &browser_options)) {
        return;
    }

    InfraredErrorCode error =
        infrared_remote_load(infrared->remote, furi_string_get_cstr(infrared->file_path));

    if(!INFRARED_ERROR_PRESENT(error)) {
        error = infrared_remote_append_signal(
            infrared->remote, infrared->current_signal, infrared->text_store[1]);
    }

    if(INFRARED_ERROR_PRESENT(error)) {
        infrared_show_error_message(
            infrared, "Failed to add to\n\"%s\"", furi_string_get_cstr(infrared->file_path));
    } else {
        scene_manager_next_scene(infrared->scene_manager, InfraredSceneUniversalSaveDone);
    }
}

void infrared_scene_universal_save_on_enter(void* context) {
    InfraredApp* infrared = context;
    Submenu* submenu = infrared->submenu;

    // The header is the name of the button being brute forced, which is also the
    // name the signal will be saved under.
    submenu_set_header(submenu, infrared->text_store[1]);
    submenu_add_item(
        submenu,
        "Save as New Remote",
        SubmenuIndexSaveAsNewRemote,
        infrared_scene_universal_save_submenu_callback,
        context);
    submenu_add_item(
        submenu,
        "Add to Existing Remote",
        SubmenuIndexAddToExistingRemote,
        infrared_scene_universal_save_submenu_callback,
        context);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewSubmenu);
}

bool infrared_scene_universal_save_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSaveAsNewRemote) {
            scene_manager_next_scene(infrared->scene_manager, InfraredSceneUniversalSaveName);
        } else if(event.event == SubmenuIndexAddToExistingRemote) {
            infrared_scene_universal_save_add_to_existing(infrared);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        infrared_scene_universal_common_return(infrared);
        consumed = true;
    }

    return consumed;
}

void infrared_scene_universal_save_on_exit(void* context) {
    InfraredApp* infrared = context;
    submenu_reset(infrared->submenu);
}
