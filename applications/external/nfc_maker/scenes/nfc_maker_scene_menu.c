#include "../nfc_maker.h"

void nfc_maker_scene_menu_submenu_callback(void* context, uint32_t index) {
    NfcMaker* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfc_maker_scene_menu_on_enter(void* context) {
    NfcMaker* app = context;
    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, "NFC Tag Maker:");

    submenu_add_item(
        submenu,
        "Bluetooth MAC",
        NfcMakerSceneBluetooth,
        nfc_maker_scene_menu_submenu_callback,
        app);

    submenu_add_item(
        submenu, "HTTPS Link", NfcMakerSceneHttps, nfc_maker_scene_menu_submenu_callback, app);

    submenu_add_item(
        submenu, "Email Address", NfcMakerSceneMail, nfc_maker_scene_menu_submenu_callback, app);

    submenu_add_item(
        submenu, "Phone Number", NfcMakerScenePhone, nfc_maker_scene_menu_submenu_callback, app);

    submenu_add_item(
        submenu, "Text Note", NfcMakerSceneText, nfc_maker_scene_menu_submenu_callback, app);

    submenu_add_item(
        submenu, "Plain URL", NfcMakerSceneUrl, nfc_maker_scene_menu_submenu_callback, app);

    submenu_add_item(
        submenu, "WiFi Login", NfcMakerSceneWifi, nfc_maker_scene_menu_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, NfcMakerSceneMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMakerViewSubmenu);
}

bool nfc_maker_scene_menu_on_event(void* context, SceneManagerEvent event) {
    NfcMaker* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, NfcMakerSceneMenu, event.event);
        consumed = true;
        scene_manager_next_scene(app->scene_manager, event.event);
    }

    return consumed;
}

void nfc_maker_scene_menu_on_exit(void* context) {
    NfcMaker* app = context;
    submenu_reset(app->submenu);
}
