#include "nfc_scene_debug_menu.h"
#include "../nfc_i.h"

#include <furi.h>
#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>

typedef enum {
    SubmenuIndexDetect,
    SubmenuIndexEmulate,
    SubmenuIndexReadEmv,
    SubmenuIndexReadMifareUl,
} SubmenuIndex;

void nfc_scene_debug_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, index);
}

const void nfc_scene_debug_menu_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "Detect", SubmenuIndexDetect, nfc_scene_debug_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Emulate", SubmenuIndexEmulate, nfc_scene_debug_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Read EMV", SubmenuIndexReadEmv, nfc_scene_debug_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu,
        "Read Mifare Ultralight",
        SubmenuIndexReadMifareUl,
        nfc_scene_debug_menu_submenu_callback,
        nfc);

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_debug_menu_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == SubmenuIndexDetect) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_debug_detect);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexEmulate) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_debug_emulate);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexReadEmv) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_debug_read_emv);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexReadMifareUl) {
        view_dispatcher_add_scene(
            nfc->nfc_common.view_dispatcher, nfc->scene_debug_read_mifare_ul);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    }

    return false;
}

const void nfc_scene_debug_menu_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}

AppScene* nfc_scene_debug_menu_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneDebugMenu;
    scene->on_enter = nfc_scene_debug_menu_on_enter;
    scene->on_event = nfc_scene_debug_menu_on_event;
    scene->on_exit = nfc_scene_debug_menu_on_exit;

    return scene;
}

void nfc_scene_debug_menu_free(AppScene* scene) {
    free(scene);
}