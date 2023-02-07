#include "../bad_usb_app_i.h"
#include <furi_hal_power.h>
#include <furi_hal_usb.h>
#include <storage/storage.h>

static bool bad_usb_file_select(BadUsbApp* bad_usb) {
    furi_assert(bad_usb);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, BAD_USB_APP_EXTENSION, &I_badusb_10px);
    browser_options.base_path = BAD_USB_APP_PATH_FOLDER;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        bad_usb->dialogs, bad_usb->file_path, bad_usb->file_path, &browser_options);

    return res;
}

void bad_usb_scene_file_select_on_enter(void* context) {
    BadUsbApp* bad_usb = context;

    furi_hal_usb_disable();

    if(bad_usb_file_select(bad_usb)) {
        scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneWork);
    } else {
        furi_hal_usb_enable();
        //scene_manager_previous_scene(bad_usb->scene_manager);
        view_dispatcher_stop(bad_usb->view_dispatcher);
    }
}

bool bad_usb_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // BadUsbApp* bad_usb = context;
    return false;
}

void bad_usb_scene_file_select_on_exit(void* context) {
    UNUSED(context);
    // BadUsbApp* bad_usb = context;
}
