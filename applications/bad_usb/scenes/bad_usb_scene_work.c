#include "../bad_usb_script.h"
#include "../bad_usb_app_i.h"
#include "../views/bad_usb_view.h"
#include "furi_hal.h"
#include "m-string.h"
#include "toolbox/path.h"

void bad_usb_scene_work_ok_callback(InputType type, void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, type);
}

bool bad_usb_scene_work_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        bad_usb_script_toggle(app->bad_usb_script);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        bad_usb_set_state(app->bad_usb_view, bad_usb_script_get_state(app->bad_usb_script));
    }
    return consumed;
}

void bad_usb_scene_work_on_enter(void* context) {
    BadUsbApp* app = context;

    string_t file_name;
    string_init(file_name);

    path_extract_filename(app->file_path, file_name, true);
    bad_usb_set_file_name(app->bad_usb_view, string_get_cstr(file_name));
    app->bad_usb_script = bad_usb_script_open(app->file_path);

    string_clear(file_name);

    bad_usb_set_state(app->bad_usb_view, bad_usb_script_get_state(app->bad_usb_script));

    bad_usb_set_ok_callback(app->bad_usb_view, bad_usb_scene_work_ok_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, BadUsbAppViewWork);
}

void bad_usb_scene_work_on_exit(void* context) {
    BadUsbApp* app = context;
    bad_usb_script_close(app->bad_usb_script);
}
