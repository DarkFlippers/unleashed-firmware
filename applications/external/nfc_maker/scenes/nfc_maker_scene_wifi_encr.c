#include "../nfc_maker.h"

void nfc_maker_scene_wifi_encr_submenu_callback(void* context, uint32_t index) {
    NfcMaker* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfc_maker_scene_wifi_encr_on_enter(void* context) {
    NfcMaker* app = context;
    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, "Encryption Type:");

    submenu_add_item(
        submenu, "AES", WifiEncryptionAes, nfc_maker_scene_wifi_encr_submenu_callback, app);

    submenu_add_item(
        submenu, "WEP", WifiEncryptionWep, nfc_maker_scene_wifi_encr_submenu_callback, app);

    submenu_add_item(
        submenu, "TKIP", WifiEncryptionTkip, nfc_maker_scene_wifi_encr_submenu_callback, app);

    submenu_add_item(
        submenu, "None", WifiEncryptionNone, nfc_maker_scene_wifi_encr_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiEncr));

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMakerViewSubmenu);
}

bool nfc_maker_scene_wifi_encr_on_event(void* context, SceneManagerEvent event) {
    NfcMaker* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, NfcMakerSceneWifiEncr, event.event);
        consumed = true;
        scene_manager_next_scene(app->scene_manager, NfcMakerSceneWifiPass);
    }

    return consumed;
}

void nfc_maker_scene_wifi_encr_on_exit(void* context) {
    NfcMaker* app = context;
    submenu_reset(app->submenu);
}
