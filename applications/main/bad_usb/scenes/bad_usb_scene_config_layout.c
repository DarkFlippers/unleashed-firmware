#include "../bad_usb_app_i.h"
#include "furi_hal_power.h"
#include "furi_hal_usb.h"
#include <storage/storage.h>

static bool bad_usb_layout_select(BadUsbApp* bad_usb) {
    furi_assert(bad_usb);

    FuriString* predefined_path;
    predefined_path = furi_string_alloc();
    if(!furi_string_empty(bad_usb->keyboard_layout)) {
        furi_string_set(predefined_path, bad_usb->keyboard_layout);
    } else {
        furi_string_set(predefined_path, BAD_USB_APP_PATH_LAYOUT_FOLDER);
    }

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, BAD_USB_APP_LAYOUT_EXTENSION, &I_keyboard_10px);
    browser_options.base_path = BAD_USB_APP_PATH_LAYOUT_FOLDER;
    browser_options.skip_assets = false;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        bad_usb->dialogs, bad_usb->keyboard_layout, predefined_path, &browser_options);

    furi_string_free(predefined_path);
    return res;
}

void bad_usb_scene_config_layout_on_enter(void* context) {
    BadUsbApp* bad_usb = context;

    if(bad_usb_layout_select(bad_usb)) {
        bad_usb_script_set_keyboard_layout(bad_usb->bad_usb_script, bad_usb->keyboard_layout);
    }
    scene_manager_previous_scene(bad_usb->scene_manager);
}

bool bad_usb_scene_config_layout_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // BadUsbApp* bad_usb = context;
    return false;
}

void bad_usb_scene_config_layout_on_exit(void* context) {
    UNUSED(context);
    // BadUsbApp* bad_usb = context;
}
