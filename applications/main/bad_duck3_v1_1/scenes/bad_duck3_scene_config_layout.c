#include "../bad_duck3_app_i.h"
#include <dialogs/dialogs.h>

void bad_duck3_scene_config_layout_on_enter(void* context) {
    BadDuck3App* app = context;

    FuriString* layout_path = furi_string_alloc_set(BAD_DUCK3_APP_PATH_LAYOUT_FOLDER);

    if(dialog_file_browser_show(
           app->dialogs,
           layout_path,
           layout_path,
           &(DialogsFileBrowserOptions){
               .extension = BAD_DUCK3_APP_LAYOUT_EXTENSION,
               .base_path = BAD_DUCK3_APP_PATH_LAYOUT_FOLDER,
               .skip_assets = false,
               .hide_ext = true,
               .item_loader_callback = NULL,
               .item_loader_context = NULL,
               .hide_dot_files = true,
           })) {
        furi_string_set(app->keyboard_layout, layout_path);
    }

    furi_string_free(layout_path);

    scene_manager_previous_scene(app->scene_manager);
}

bool bad_duck3_scene_config_layout_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void bad_duck3_scene_config_layout_on_exit(void* context) {
    UNUSED(context);
}
