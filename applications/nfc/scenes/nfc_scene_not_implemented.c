#include "../nfc_i.h"

void nfc_scene_not_implemented_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

const void nfc_scene_not_implemented_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // TODO Set data from worker
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Back");
    dialog_ex_set_header(dialog_ex, "Not implemented", 60, 24, AlignCenter, AlignCenter);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_not_implemented_dialog_callback);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

const bool nfc_scene_not_implemented_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            return scene_manager_previous_scene(nfc->scene_manager);
        }
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
