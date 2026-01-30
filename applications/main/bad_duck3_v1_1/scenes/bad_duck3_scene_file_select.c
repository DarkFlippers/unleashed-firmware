#include "../bad_duck3_app_i.h"
#include <dialogs/dialogs.h>

void bad_duck3_scene_file_select_on_enter(void* context) {
    BadDuck3App* app = context;

    if(dialog_file_browser_show(
           app->dialogs,
           app->file_path,
           app->file_path,
           &(DialogsFileBrowserOptions){
               .extension = BAD_DUCK3_APP_SCRIPT_EXTENSION,
               .base_path = BAD_DUCK3_APP_BASE_FOLDER,
               .skip_assets = true,
               .hide_ext = false,
               .item_loader_callback = NULL,
               .item_loader_context = NULL,
               .hide_dot_files = true,
           })) {
        scene_manager_set_scene_state(app->scene_manager, BadDuck3SceneWork, true);
        scene_manager_next_scene(app->scene_manager, BadDuck3SceneWork);
    } else {
        view_dispatcher_stop(app->view_dispatcher);
    }
}

bool bad_duck3_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void bad_duck3_scene_file_select_on_exit(void* context) {
    UNUSED(context);
}
