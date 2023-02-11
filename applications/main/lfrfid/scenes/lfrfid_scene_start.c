#include "../lfrfid_i.h"
#include <dolphin/dolphin.h>

typedef enum {
    SubmenuIndexRead,
    SubmenuIndexSaved,
    SubmenuIndexAddManually,
    SubmenuIndexExtraActions,
} SubmenuIndex;

static void lfrfid_scene_start_submenu_callback(void* context, uint32_t index) {
    LfRfid* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void lfrfid_scene_start_on_enter(void* context) {
    LfRfid* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(submenu, "Read", SubmenuIndexRead, lfrfid_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "Saved", SubmenuIndexSaved, lfrfid_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "Add Manually", SubmenuIndexAddManually, lfrfid_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu,
        "Extra Actions",
        SubmenuIndexExtraActions,
        lfrfid_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, LfRfidSceneStart));

    // clear key
    furi_string_reset(app->file_name);
    app->protocol_id = PROTOCOL_NO;
    app->read_type = LFRFIDWorkerReadTypeAuto;

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewSubmenu);
}

bool lfrfid_scene_start_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexRead) {
            scene_manager_set_scene_state(app->scene_manager, LfRfidSceneStart, SubmenuIndexRead);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneRead);
            DOLPHIN_DEED(DolphinDeedRfidRead);
            consumed = true;
        } else if(event.event == SubmenuIndexSaved) {
            // Like in the other apps, explicitly save the scene state
            // in each branch in case the user cancels loading a file.
            scene_manager_set_scene_state(app->scene_manager, LfRfidSceneStart, SubmenuIndexSaved);
            furi_string_set(app->file_path, LFRFID_APP_FOLDER);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSelectKey);
            consumed = true;
        } else if(event.event == SubmenuIndexAddManually) {
            scene_manager_set_scene_state(
                app->scene_manager, LfRfidSceneStart, SubmenuIndexAddManually);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveType);
            consumed = true;
        } else if(event.event == SubmenuIndexExtraActions) {
            scene_manager_set_scene_state(
                app->scene_manager, LfRfidSceneStart, SubmenuIndexExtraActions);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneExtraActions);
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_scene_start_on_exit(void* context) {
    LfRfid* app = context;

    submenu_reset(app->submenu);
}
