#include "nfc_scene_start.h"
#include "../nfc_i.h"

#include <furi.h>
#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>

typedef enum {
    SubmenuIndexRead,
    SubmenuIndexRunScript,
    SubmenuIndexSaved,
    SubmenuIndexAddManualy,
    SubmenuIndexDebug,
} SubmenuIndex;

void nfc_scene_start_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, index);
}

const void nfc_scene_start_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "Read card", SubmenuIndexRead, nfc_scene_start_submenu_callback, nfc);
    submenu_add_item(
        submenu,
        "Run special action",
        SubmenuIndexRunScript,
        nfc_scene_start_submenu_callback,
        nfc);
    submenu_add_item(
        submenu, "Saved cards", SubmenuIndexSaved, nfc_scene_start_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Add manualy", SubmenuIndexAddManualy, nfc_scene_start_submenu_callback, nfc);
    submenu_add_item(submenu, "Debug", SubmenuIndexDebug, nfc_scene_start_submenu_callback, nfc);

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_start_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == SubmenuIndexRead) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_read_card);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexRunScript) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_not_implemented);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexSaved) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_not_implemented);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexAddManualy) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_not_implemented);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    } else if(event == SubmenuIndexDebug) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_debug_menu);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
    }
    return false;
}

const void nfc_scene_start_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}

AppScene* nfc_scene_start_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneStart;
    scene->on_enter = nfc_scene_start_on_enter;
    scene->on_event = nfc_scene_start_on_event;
    scene->on_exit = nfc_scene_start_on_exit;

    return scene;
}

void nfc_scene_start_free(AppScene* scene) {
    free(scene);
}