#include "../t5577_app.h"
#include <t5577_icons.h>

void t5577_app_scene_select_dump_on_enter(void* context) {
    T5577App* app = context;

    // Where the browser opens: the dump this run last opened or saved, which is what makes
    // leaving the file menu come back to the row it was opened from, and the app folder before
    // there is one. A path that has since gone - deleted here, or renamed by the app that was
    // handed the dump - is the browser's own business; it walks up to a folder that exists.
    if(furi_string_empty(app->path)) {
        furi_string_set(app->path, T5577_APP_FOLDER);
    }

    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, T5577_FILE_EXTENSION, &I_t5577_10px);
    options.base_path = T5577_APP_FOLDER;

    // Nothing is counted first. A folder with no dumps in it draws "<Empty>" by itself, and
    // looking would mean reading past every key in the lfrfid folder before the browser - which
    // reads the same folder again - can put anything on screen. The dumps earlier versions
    // saved are in a subfolder of this one, and the browser lists it like any other.
    if(dialog_file_browser_show(app->dialogs, app->path, app->path, &options) &&
       t5577_app_open_dump(app)) {
        scene_manager_next_scene(app->scene_manager, T5577AppSceneFileMenu);
        return;
    }

    // Nothing was opened, so there is nothing for Back to come back to: this scene is the
    // browser, and re-entering it would open it again on the file the user just declined.
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
}

bool t5577_app_scene_select_dump_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void t5577_app_scene_select_dump_on_exit(void* context) {
    UNUSED(context);
}
