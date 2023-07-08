#include "../nfc_rfid_detector_app_i.h"

typedef enum {
    SubmenuIndexNfcRfidDetectorFieldPresence,
    SubmenuIndexNfcRfidDetectorAbout,
} SubmenuIndex;

void nfc_rfid_detector_scene_start_submenu_callback(void* context, uint32_t index) {
    NfcRfidDetectorApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfc_rfid_detector_scene_start_on_enter(void* context) {
    UNUSED(context);
    NfcRfidDetectorApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Detect field type",
        SubmenuIndexNfcRfidDetectorFieldPresence,
        nfc_rfid_detector_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "About",
        SubmenuIndexNfcRfidDetectorAbout,
        nfc_rfid_detector_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, NfcRfidDetectorSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcRfidDetectorViewSubmenu);
}

bool nfc_rfid_detector_scene_start_on_event(void* context, SceneManagerEvent event) {
    NfcRfidDetectorApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexNfcRfidDetectorAbout) {
            scene_manager_next_scene(app->scene_manager, NfcRfidDetectorSceneAbout);
            consumed = true;
        } else if(event.event == SubmenuIndexNfcRfidDetectorFieldPresence) {
            scene_manager_next_scene(app->scene_manager, NfcRfidDetectorSceneFieldPresence);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, NfcRfidDetectorSceneStart, event.event);
    }

    return consumed;
}

void nfc_rfid_detector_scene_start_on_exit(void* context) {
    NfcRfidDetectorApp* app = context;
    submenu_reset(app->submenu);
}
