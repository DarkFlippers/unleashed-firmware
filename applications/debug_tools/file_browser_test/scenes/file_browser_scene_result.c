#include "../file_browser_app_i.h"
#include "furi_hal.h"
#include "m-string.h"

void file_browser_scene_result_ok_callback(InputType type, void* context) {
    furi_assert(context);
    FileBrowserApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, type);
}

bool file_browser_scene_result_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    //FileBrowserApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
    }
    return consumed;
}

void file_browser_scene_result_on_enter(void* context) {
    FileBrowserApp* app = context;

    widget_add_string_multiline_element(
        app->widget, 64, 10, AlignCenter, AlignTop, FontSecondary, string_get_cstr(app->file_path));

    view_dispatcher_switch_to_view(app->view_dispatcher, FileBrowserAppViewResult);
}

void file_browser_scene_result_on_exit(void* context) {
    UNUSED(context);
    FileBrowserApp* app = context;
    widget_reset(app->widget);
}
