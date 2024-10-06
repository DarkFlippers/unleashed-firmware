#include "../bad_ble_app_i.h"

static void bad_ble_scene_unpair_done_popup_callback(void* context) {
    BadBleApp* bad_ble = context;
    scene_manager_search_and_switch_to_previous_scene(bad_ble->scene_manager, BadBleSceneConfig);
}

void bad_ble_scene_unpair_done_on_enter(void* context) {
    BadBleApp* bad_ble = context;
    Popup* popup = bad_ble->popup;

    popup_set_icon(popup, 48, 4, &I_DolphinDone_80x58);
    popup_set_header(popup, "Done", 20, 19, AlignLeft, AlignBottom);
    popup_set_callback(popup, bad_ble_scene_unpair_done_popup_callback);
    popup_set_context(popup, bad_ble);
    popup_set_timeout(popup, 1000);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(bad_ble->view_dispatcher, BadBleAppViewPopup);
}

bool bad_ble_scene_unpair_done_on_event(void* context, SceneManagerEvent event) {
    BadBleApp* bad_ble = context;
    UNUSED(bad_ble);
    UNUSED(event);

    bool consumed = false;

    return consumed;
}

void bad_ble_scene_unpair_done_on_exit(void* context) {
    BadBleApp* bad_ble = context;
    Popup* popup = bad_ble->popup;

    popup_reset(popup);
}
