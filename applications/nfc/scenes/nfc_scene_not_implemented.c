#include "nfc_scene_not_implemented.h"
#include "../nfc_i.h"

#include <furi.h>
#include <gui/modules/dialog_ex.h>
#include <gui/view_dispatcher.h>

void nfc_scene_not_implemented_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, result);
}

const void nfc_scene_not_implemented_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // TODO Set data from worker
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Back");
    dialog_ex_set_header(dialog_ex, "Not implemented", 60, 24, AlignCenter, AlignCenter);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_not_implemented_dialog_callback);

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewDialogEx);
}

const bool nfc_scene_not_implemented_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == DialogExResultLeft) {
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventBack);
        return true;
    }
    return false;
}

const void nfc_scene_not_implemented_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);
}

AppScene* nfc_scene_not_implemented_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneReadCardSuccess;
    scene->on_enter = nfc_scene_not_implemented_on_enter;
    scene->on_event = nfc_scene_not_implemented_on_event;
    scene->on_exit = nfc_scene_not_implemented_on_exit;

    return scene;
}

void nfc_scene_not_implemented_free(AppScene* scene) {
    free(scene);
}
