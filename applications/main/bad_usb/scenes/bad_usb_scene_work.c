#include "../helpers/ducky_script.h"
#include "../bad_usb_app_i.h"
#include "../views/bad_usb_view.h"
#include <furi_hal.h>
#include "toolbox/path.h"

void bad_usb_scene_work_button_callback(InputKey key, void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, key);
}

bool bad_usb_scene_work_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InputKeyLeft) {
            if(bad_usb_view_is_idle_state(app->bad_usb_view)) {
                bad_usb_script_close(app->bad_usb_script);
                app->bad_usb_script = NULL;

                scene_manager_set_scene_state(app->scene_manager, BadUsbSceneConfig, 0);
                scene_manager_next_scene(app->scene_manager, BadUsbSceneConfig);
            }
            consumed = true;
        } else if(event.event == InputKeyOk) {
            bad_usb_script_start_stop(app->bad_usb_script);
            consumed = true;
        } else if(event.event == InputKeyRight) {
            if(bad_usb_view_is_idle_state(app->bad_usb_view)) {
                bad_usb_set_interface(
                    app,
                    app->interface == BadUsbHidInterfaceBle ? BadUsbHidInterfaceUsb :
                                                              BadUsbHidInterfaceBle);
                bad_usb_script_close(app->bad_usb_script);
                app->bad_usb_script = bad_usb_script_open(
                    app->file_path, &app->interface, &app->script_hid_cfg, false);
                bad_usb_script_set_keyboard_layout(app->bad_usb_script, app->keyboard_layout);
            } else {
                bad_usb_script_pause_resume(app->bad_usb_script);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        bad_usb_view_set_state(app->bad_usb_view, bad_usb_script_get_state(app->bad_usb_script));
        bad_usb_view_set_interface(app->bad_usb_view, app->interface);
    }
    return consumed;
}

void bad_usb_scene_work_on_enter(void* context) {
    BadUsbApp* app = context;

    bad_usb_view_set_interface(app->bad_usb_view, app->interface);

    // Opening script the first time:
    // - copy user settings as base config
    // - load ID/BLE_ID/BT_ID config if present
    // Then disable this until next script selected so user can customize options
    bool first_script_load = scene_manager_get_scene_state(app->scene_manager, BadUsbSceneWork);
    if(first_script_load) {
        memcpy(&app->script_hid_cfg, &app->user_hid_cfg, sizeof(app->script_hid_cfg));
        scene_manager_set_scene_state(app->scene_manager, BadUsbSceneWork, false);
    }
    // Interface and config are passed as pointers as ID/BLE_ID/BT_ID config can modify them
    app->bad_usb_script = bad_usb_script_open(
        app->file_path, &app->interface, &app->script_hid_cfg, first_script_load);
    bad_usb_script_set_keyboard_layout(app->bad_usb_script, app->keyboard_layout);

    FuriString* file_name;
    file_name = furi_string_alloc();
    path_extract_filename(app->file_path, file_name, true);
    bad_usb_view_set_file_name(app->bad_usb_view, furi_string_get_cstr(file_name));
    furi_string_free(file_name);

    FuriString* layout;
    layout = furi_string_alloc();
    path_extract_filename(app->keyboard_layout, layout, true);
    bad_usb_view_set_layout(app->bad_usb_view, furi_string_get_cstr(layout));
    furi_string_free(layout);

    bad_usb_view_set_state(app->bad_usb_view, bad_usb_script_get_state(app->bad_usb_script));

    bad_usb_view_set_button_callback(app->bad_usb_view, bad_usb_scene_work_button_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, BadUsbAppViewWork);
}

void bad_usb_scene_work_on_exit(void* context) {
    UNUSED(context);
}
