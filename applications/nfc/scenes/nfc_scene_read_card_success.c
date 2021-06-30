#include "nfc_scene_read_card_success.h"
#include "../nfc_i.h"

#include <furi.h>
#include <gui/modules/dialog_ex.h>
#include <gui/view_dispatcher.h>

#define NFC_SCENE_READ_SUCCESS_SHIFT "              "

void nfc_scene_read_card_success_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, result);
}

const void nfc_scene_read_card_success_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Send notification
    notification_message(nfc->notifications, &sequence_success);

    // Setup view
    NfcDeviceData* data = (NfcDeviceData*)&nfc->nfc_common.worker_result;
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_header(dialog_ex, nfc_get_dev_type(data->device), 36, 8, AlignLeft, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 8, 13, I_Medium_chip_22x21);
    // Display UID
    if(data->uid_len == 4) {
        nfc_set_text_store(
            nfc,
            NFC_SCENE_READ_SUCCESS_SHIFT "%s\n" NFC_SCENE_READ_SUCCESS_SHIFT
                                         "ATQA: %02X%02X SAK: %02X\nUID: %02X %02X %02X %02X",
            nfc_get_protocol(data->protocol),
            data->atqa[0],
            data->atqa[1],
            data->sak,
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3]);
    } else if(data->uid_len == 7) {
        nfc_set_text_store(
            nfc,
            NFC_SCENE_READ_SUCCESS_SHIFT
            "%s\n" NFC_SCENE_READ_SUCCESS_SHIFT
            "ATQA: %02X%02X SAK: %02X\nUID: %02X %02X %02X %02X %02X %02X %02X",
            nfc_get_protocol(data->protocol),
            data->atqa[0],
            data->atqa[1],
            data->sak,
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3],
            data->uid[4],
            data->uid[5],
            data->uid[6]);
    }
    dialog_ex_set_text(dialog_ex, nfc->text_store, 8, 16, AlignLeft, AlignTop);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_read_card_success_dialog_callback);

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewDialogEx);
}

const bool nfc_scene_read_card_success_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == DialogExResultLeft) {
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventBack);
        return true;
    } else if(event == DialogExResultRight) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_card_menu);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    }
    return false;
}

const void nfc_scene_read_card_success_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_icon(dialog_ex, 0, 0, I_Empty_1x1);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_right_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);
}

AppScene* nfc_scene_read_card_success_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneReadCardSuccess;
    scene->on_enter = nfc_scene_read_card_success_on_enter;
    scene->on_event = nfc_scene_read_card_success_on_event;
    scene->on_exit = nfc_scene_read_card_success_on_exit;

    return scene;
}

void nfc_scene_read_card_success_free(AppScene* scene) {
    free(scene);
}
