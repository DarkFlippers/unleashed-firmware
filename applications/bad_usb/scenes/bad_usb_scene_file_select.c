#include "../bad_usb_app_i.h"
#include "furi-hal-power.h"

static bool bad_usb_file_select(BadUsbApp* bad_usb) {
    furi_assert(bad_usb);

    // Input events and views are managed by file_select
    bool res = dialog_file_select_show(
        bad_usb->dialogs,
        BAD_USB_APP_PATH_FOLDER,
        BAD_USB_APP_EXTENSION,
        bad_usb->file_name,
        sizeof(bad_usb->file_name),
        NULL);
    return res;
}

void bad_usb_scene_file_select_on_enter(void* context) {
    BadUsbApp* bad_usb = context;

    if(bad_usb_file_select(bad_usb)) {
        scene_manager_next_scene(bad_usb->scene_manager, BadUsbAppViewWork);
    } else {
        //scene_manager_previous_scene(bad_usb->scene_manager);
        view_dispatcher_stop(bad_usb->view_dispatcher);
    }
}

bool bad_usb_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    // BadUsbApp* bad_usb = context;
    return false;
}

void bad_usb_scene_file_select_on_exit(void* context) {
    // BadUsbApp* bad_usb = context;
}
