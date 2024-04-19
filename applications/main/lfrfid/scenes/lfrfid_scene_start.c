#include "../lfrfid_i.h"
#include <dolphin/dolphin.h>

static void lfrfid_scene_start_submenu_callback(void* context, uint32_t index) {
    LfRfid* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void lfrfid_scene_start_on_enter(void* context) {
    LfRfid* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu, "Read", LfRfidMenuIndexRead, lfrfid_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "Saved", LfRfidMenuIndexSaved, lfrfid_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu,
        "Add Manually",
        LfRfidMenuIndexAddManually,
        lfrfid_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Extra Actions",
        LfRfidMenuIndexExtraActions,
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
        if(event.event == LfRfidMenuIndexRead) {
            scene_manager_set_scene_state(
                app->scene_manager, LfRfidSceneStart, LfRfidMenuIndexRead);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneRead);
            dolphin_deed(DolphinDeedRfidRead);
            consumed = true;
        } else if(event.event == LfRfidMenuIndexSaved) {
            // Like in the other apps, explicitly save the scene state
            // in each branch in case the user cancels loading a file.
            scene_manager_set_scene_state(
                app->scene_manager, LfRfidSceneStart, LfRfidMenuIndexSaved);
            furi_string_set(app->file_path, LFRFID_APP_FOLDER);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSelectKey);
            consumed = true;
        } else if(event.event == LfRfidMenuIndexAddManually) {
            scene_manager_set_scene_state(
                app->scene_manager, LfRfidSceneStart, LfRfidMenuIndexAddManually);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveType);
            consumed = true;
        } else if(event.event == LfRfidMenuIndexExtraActions) {
            scene_manager_set_scene_state(
                app->scene_manager, LfRfidSceneStart, LfRfidMenuIndexExtraActions);
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
