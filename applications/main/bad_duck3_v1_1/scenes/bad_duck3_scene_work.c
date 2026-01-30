#include "../helpers/duck3_script.h"
#include "../bad_duck3_app_i.h"
#include "../views/bad_duck3_view.h"
#include <furi_hal.h>
#include "toolbox/path.h"

void bad_duck3_scene_work_button_callback(InputKey key, void* context) {
    furi_assert(context);
    BadDuck3App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, key);
}

bool bad_duck3_scene_work_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InputKeyLeft) {
            if(bad_duck3_view_is_idle_state(app->bad_duck3_view)) {
                duck3_script_close(app->duck3_script);
                app->duck3_script = NULL;

                scene_manager_set_scene_state(app->scene_manager, BadDuck3SceneConfig, 0);
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfig);
            }
            consumed = true;
        } else if(event.event == InputKeyOk) {
            duck3_script_start_stop(app->duck3_script);
            consumed = true;
        } else if(event.event == InputKeyRight) {
            if(bad_duck3_view_is_idle_state(app->bad_duck3_view)) {
                bad_duck3_set_interface(
                    app,
                    app->interface == Duck3HidInterfaceBle ? Duck3HidInterfaceUsb :
                                                             Duck3HidInterfaceBle);
                duck3_script_close(app->duck3_script);
                app->duck3_script = duck3_script_open(
                    app->file_path, &app->interface, &app->script_hid_cfg, false);
                duck3_script_set_keyboard_layout(app->duck3_script, app->keyboard_layout);
            } else {
                duck3_script_pause_resume(app->duck3_script);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        bad_duck3_view_set_state(app->bad_duck3_view, duck3_script_get_state(app->duck3_script));
        bad_duck3_view_set_interface(app->bad_duck3_view, app->interface);
    }
    return consumed;
}

void bad_duck3_scene_work_on_enter(void* context) {
    BadDuck3App* app = context;

    bad_duck3_view_set_interface(app->bad_duck3_view, app->interface);

    bool first_script_load = scene_manager_get_scene_state(app->scene_manager, BadDuck3SceneWork);
    if(first_script_load) {
        memcpy(&app->script_hid_cfg, &app->user_hid_cfg, sizeof(app->script_hid_cfg));
        scene_manager_set_scene_state(app->scene_manager, BadDuck3SceneWork, false);
    }

    app->duck3_script = duck3_script_open(
        app->file_path, &app->interface, &app->script_hid_cfg, first_script_load);
    duck3_script_set_keyboard_layout(app->duck3_script, app->keyboard_layout);

    FuriString* file_name;
    file_name = furi_string_alloc();
    path_extract_filename(app->file_path, file_name, true);
    bad_duck3_view_set_file_name(app->bad_duck3_view, furi_string_get_cstr(file_name));
    furi_string_free(file_name);

    FuriString* layout;
    layout = furi_string_alloc();
    path_extract_filename(app->keyboard_layout, layout, true);
    bad_duck3_view_set_layout(app->bad_duck3_view, furi_string_get_cstr(layout));
    furi_string_free(layout);

    bad_duck3_view_set_state(app->bad_duck3_view, duck3_script_get_state(app->duck3_script));

    bad_duck3_view_set_button_callback(app->bad_duck3_view, bad_duck3_scene_work_button_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewWork);
}

void bad_duck3_scene_work_on_exit(void* context) {
    UNUSED(context);
}
