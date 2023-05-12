#include "../bad_bt_app.h"
#include <furi_hal_power.h>
#include <storage/storage.h>

static bool bad_bt_file_select(BadBtApp* bad_bt) {
    furi_assert(bad_bt);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, BAD_BT_APP_SCRIPT_EXTENSION, &I_badbt_10px);
    browser_options.base_path = BAD_BT_APP_BASE_FOLDER;
    browser_options.skip_assets = true;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        bad_bt->dialogs, bad_bt->file_path, bad_bt->file_path, &browser_options);

    return res;
}

void bad_bt_scene_file_select_on_enter(void* context) {
    BadBtApp* bad_bt = context;

    if(bad_bt->bad_bt_script) {
        bad_bt_script_close(bad_bt->bad_bt_script);
        bad_bt->bad_bt_script = NULL;
    }

    if(bad_bt_file_select(bad_bt)) {
        bad_bt->bad_bt_script = bad_bt_script_open(bad_bt->file_path, bad_bt->bt, bad_bt);
        bad_bt_script_set_keyboard_layout(bad_bt->bad_bt_script, bad_bt->keyboard_layout);

        scene_manager_next_scene(bad_bt->scene_manager, BadBtSceneWork);
    } else {
        view_dispatcher_stop(bad_bt->view_dispatcher);
    }
}

bool bad_bt_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // BadBtApp* bad_bt = context;
    return false;
}

void bad_bt_scene_file_select_on_exit(void* context) {
    UNUSED(context);
    // BadBtApp* bad_bt = context;
}
