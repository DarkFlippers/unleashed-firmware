#include "../helpers/ducky_script.h"
#include "../bad_bt_app.h"
#include "../views/bad_bt_view.h"
#include <furi_hal.h>
#include "toolbox/path.h"

void bad_bt_scene_work_button_callback(InputKey key, void* context) {
    furi_assert(context);
    BadBtApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, key);
}

bool bad_bt_scene_work_on_event(void* context, SceneManagerEvent event) {
    BadBtApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InputKeyLeft) {
            if(bad_bt_is_idle_state(app->bad_bt_view)) {
                scene_manager_next_scene(app->scene_manager, BadBtSceneConfig);
            }
            consumed = true;
        } else if(event.event == InputKeyOk) {
            bad_bt_script_toggle(app->bad_bt_script);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        bad_bt_set_state(app->bad_bt_view, bad_bt_script_get_state(app->bad_bt_script));
    }
    return consumed;
}

void bad_bt_scene_work_on_enter(void* context) {
    BadBtApp* app = context;

    FuriString* file_name;
    file_name = furi_string_alloc();
    path_extract_filename(app->file_path, file_name, true);
    bad_bt_set_file_name(app->bad_bt_view, furi_string_get_cstr(file_name));
    furi_string_free(file_name);

    FuriString* layout;
    layout = furi_string_alloc();
    path_extract_filename(app->keyboard_layout, layout, true);
    bad_bt_set_layout(app->bad_bt_view, furi_string_get_cstr(layout));
    furi_string_free(layout);

    bad_bt_set_state(app->bad_bt_view, bad_bt_script_get_state(app->bad_bt_script));

    bad_bt_set_button_callback(app->bad_bt_view, bad_bt_scene_work_button_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, BadBtAppViewWork);
}

void bad_bt_scene_work_on_exit(void* context) {
    UNUSED(context);
}
