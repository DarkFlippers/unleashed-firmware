#include "../nfc_maker.h"

void nfc_maker_scene_wifi_auth_submenu_callback(void* context, uint32_t index) {
    NfcMaker* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfc_maker_scene_wifi_auth_on_enter(void* context) {
    NfcMaker* app = context;
    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, "Authentication Type:");

    submenu_add_item(
        submenu, "Open", WifiAuthenticationOpen, nfc_maker_scene_wifi_auth_submenu_callback, app);

    submenu_add_item(
        submenu,
        "WPA 2 Personal",
        WifiAuthenticationWpa2Personal,
        nfc_maker_scene_wifi_auth_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "WPA 2 Enterprise",
        WifiAuthenticationWpa2Enterprise,
        nfc_maker_scene_wifi_auth_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "WPA Personal",
        WifiAuthenticationWpaPersonal,
        nfc_maker_scene_wifi_auth_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "WPA Enterprise",
        WifiAuthenticationWpaEnterprise,
        nfc_maker_scene_wifi_auth_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "Shared",
        WifiAuthenticationShared,
        nfc_maker_scene_wifi_auth_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneWifiAuth));

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMakerViewSubmenu);
}

bool nfc_maker_scene_wifi_auth_on_event(void* context, SceneManagerEvent event) {
    NfcMaker* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, NfcMakerSceneWifiAuth, event.event);
        consumed = true;
        if(event.event == WifiAuthenticationOpen) {
            scene_manager_set_scene_state(
                app->scene_manager, NfcMakerSceneWifiEncr, WifiEncryptionNone);
            strcpy(app->small_buf2, "");
            scene_manager_next_scene(app->scene_manager, NfcMakerSceneSave);
        } else {
            scene_manager_set_scene_state(
                app->scene_manager, NfcMakerSceneWifiEncr, WifiEncryptionAes);
            scene_manager_next_scene(app->scene_manager, NfcMakerSceneWifiEncr);
        }
    }

    return consumed;
}

void nfc_maker_scene_wifi_auth_on_exit(void* context) {
    NfcMaker* app = context;
    submenu_reset(app->submenu);
}
