#include "../nfc_i.h"
#include "../helpers/nfc_generators.h"

void nfc_scene_generate_info_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_generate_info_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup dialog view
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_right_button_text(dialog_ex, "More");

    // Create info text
    string_t info_str;
    string_init_printf(
        info_str, "%s\n%s\nUID:", nfc->generator->name, nfc_get_dev_type(data->type));
    // Append UID
    for(int i = 0; i < data->uid_len; ++i) {
        string_cat_printf(info_str, " %02X", data->uid[i]);
    }
    nfc_text_store_set(nfc, string_get_cstr(info_str));
    string_clear(info_str);

    dialog_ex_set_text(dialog_ex, nfc->text_store, 0, 0, AlignLeft, AlignTop);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_generate_info_dialog_callback);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_generate_info_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultRight) {
            scene_manager_next_scene(nfc->scene_manager, nfc->generator->next_scene);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_generate_info_on_exit(void* context) {
    Nfc* nfc = context;

    // Clean views
    dialog_ex_reset(nfc->dialog_ex);
}
