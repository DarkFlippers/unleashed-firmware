#include "../nfc_i.h"
#include <dolphin/dolphin.h>

#define NFC_SCENE_READ_SUCCESS_SHIFT "              "

enum {
    ReadMifareDesfireSuccessStateShowUID,
    ReadMifareDesfireSuccessStateShowData,
};

void nfc_scene_read_mifare_desfire_success_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_read_mifare_desfire_success_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    MifareDesfireData* data = &nfc->dev->dev_data.mf_df_data;
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Back");
    dialog_ex_set_center_button_text(dialog_ex, "Data");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_icon(dialog_ex, 8, 16, &I_Medium_chip_22x21);

    uint16_t n_apps = 0;
    uint16_t n_files = 0;

    for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
        n_apps++;
        for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
            n_files++;
        }
    }

    nfc_text_store_set(
        nfc,
        "UID: %02X %02X %02X %02X %02X %02X %02X\n" NFC_SCENE_READ_SUCCESS_SHIFT
        "%d%s bytes\n" NFC_SCENE_READ_SUCCESS_SHIFT "%d bytes free\n"
        "%d application%s, %d file%s",
        data->version.uid[0],
        data->version.uid[1],
        data->version.uid[2],
        data->version.uid[3],
        data->version.uid[4],
        data->version.uid[5],
        data->version.uid[6],
        1 << (data->version.sw_storage >> 1),
        (data->version.sw_storage & 1) ? "+" : "",
        data->free_memory ? data->free_memory->bytes : 0,
        n_apps,
        n_apps == 1 ? "" : "s",
        n_files,
        n_files == 1 ? "" : "s");
    dialog_ex_set_text(dialog_ex, nfc->text_store, 8, 6, AlignLeft, AlignTop);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(
        dialog_ex, nfc_scene_read_mifare_desfire_success_dialog_callback);

    scene_manager_set_scene_state(
        nfc->scene_manager,
        NfcSceneReadMifareDesfireSuccess,
        ReadMifareDesfireSuccessStateShowUID);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_read_mifare_desfire_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadMifareDesfireSuccess);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(state == ReadMifareDesfireSuccessStateShowUID && event.event == DialogExResultLeft) {
            scene_manager_previous_scene(nfc->scene_manager);
            consumed = true;
        } else if(
            state == ReadMifareDesfireSuccessStateShowUID && event.event == DialogExResultCenter) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMifareDesfireData);
            consumed = true;
        } else if(state == ReadMifareDesfireSuccessStateShowUID && event.event == DialogExResultRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMifareDesfireMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state == ReadMifareDesfireSuccessStateShowData) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneReadMifareDesfireSuccess,
                ReadMifareDesfireSuccessStateShowUID);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_read_mifare_desfire_success_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clean dialog
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_reset(dialog_ex);
}
