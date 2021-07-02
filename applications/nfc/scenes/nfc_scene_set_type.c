#include "nfc_scene_set_type.h"
#include "../nfc_i.h"

#include <furi.h>
#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>

enum SubmenuIndex {
    SubmenuIndexNFCA4,
    SubmenuIndexNFCA7,
};

void nfc_scene_set_type_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, index);
}

const void nfc_scene_set_type_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "NFC-A 7-bytes UID", SubmenuIndexNFCA7, nfc_scene_set_type_submenu_callback, nfc);
    submenu_add_item(
        submenu, "NFC-A 4-bytes UID", SubmenuIndexNFCA4, nfc_scene_set_type_submenu_callback, nfc);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_set_type_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == SubmenuIndexNFCA7) {
        nfc->device.data.uid_len = 7;
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_set_sak);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexNFCA4) {
        nfc->device.data.uid_len = 4;
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_set_sak);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    }
    return false;
}

const void nfc_scene_set_type_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}

AppScene* nfc_scene_set_type_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneSetType;
    scene->on_enter = nfc_scene_set_type_on_enter;
    scene->on_event = nfc_scene_set_type_on_event;
    scene->on_exit = nfc_scene_set_type_on_exit;

    return scene;
}

void nfc_scene_set_type_free(AppScene* scene) {
    free(scene);
}