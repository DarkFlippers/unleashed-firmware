#include "../nfc_i.h"

void nfc_scene_mf_ultralight_data_on_enter(void* context) {
    Nfc* nfc = context;
    MfUltralightData* data = &nfc->dev->dev_data.mf_ul_data;
    TextBox* text_box = nfc->text_box;

    text_box_set_font(text_box, TextBoxFontHex);
    for(uint16_t i = 0; i < data->data_size; i += 2) {
        if(!(i % 8) && i) {
            string_push_back(nfc->text_box_store, '\n');
        }
        string_cat_printf(nfc->text_box_store, "%02X%02X ", data->data[i], data->data[i + 1]);
    }
    text_box_set_text(text_box, string_get_cstr(nfc->text_box_store));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
}

bool nfc_scene_mf_ultralight_data_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_mf_ultralight_data_on_exit(void* context) {
    Nfc* nfc = context;

    // Clean view
    text_box_reset(nfc->text_box);
    string_reset(nfc->text_box_store);
}