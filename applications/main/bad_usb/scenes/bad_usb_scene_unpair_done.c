#include "../bad_usb_app_i.h"

static void bad_usb_scene_unpair_done_popup_callback(void* context) {
    BadUsbApp* bad_usb = context;
    scene_manager_search_and_switch_to_previous_scene(bad_usb->scene_manager, BadUsbSceneConfig);
}

void bad_usb_scene_unpair_done_on_enter(void* context) {
    BadUsbApp* bad_usb = context;
    Popup* popup = bad_usb->popup;

    bad_usb_hid_ble_remove_pairing();

    popup_set_icon(popup, 48, 4, &I_DolphinDone_80x58);
    popup_set_header(popup, "Done", 20, 19, AlignLeft, AlignBottom);
    popup_set_callback(popup, bad_usb_scene_unpair_done_popup_callback);
    popup_set_context(popup, bad_usb);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(bad_usb->view_dispatcher, BadUsbAppViewPopup);
}

bool bad_usb_scene_unpair_done_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* bad_usb = context;
    UNUSED(bad_usb);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void bad_usb_scene_unpair_done_on_exit(void* context) {
    BadUsbApp* bad_usb = context;
    Popup* popup = bad_usb->popup;
    UNUSED(popup);

    popup_reset(popup);
}
