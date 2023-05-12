#include "../bad_bt_app.h"
#include "furi_hal_power.h"
#include <storage/storage.h>

static bool bad_bt_layout_select(BadBtApp* bad_bt) {
    furi_assert(bad_bt);

    FuriString* predefined_path;
    predefined_path = furi_string_alloc();
    if(!furi_string_empty(bad_bt->keyboard_layout)) {
        furi_string_set(predefined_path, bad_bt->keyboard_layout);
    } else {
        furi_string_set(predefined_path, BAD_BT_APP_PATH_LAYOUT_FOLDER);
    }

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, BAD_BT_APP_LAYOUT_EXTENSION, &I_keyboard_10px);
    browser_options.base_path = BAD_BT_APP_PATH_LAYOUT_FOLDER;
    browser_options.skip_assets = false;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        bad_bt->dialogs, bad_bt->keyboard_layout, predefined_path, &browser_options);

    furi_string_free(predefined_path);
    return res;
}

void bad_bt_scene_config_layout_on_enter(void* context) {
    BadBtApp* bad_bt = context;

    if(bad_bt_layout_select(bad_bt)) {
        bad_bt_script_set_keyboard_layout(bad_bt->bad_bt_script, bad_bt->keyboard_layout);
    }
    scene_manager_previous_scene(bad_bt->scene_manager);
}

bool bad_bt_scene_config_layout_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void bad_bt_scene_config_layout_on_exit(void* context) {
    UNUSED(context);
}
