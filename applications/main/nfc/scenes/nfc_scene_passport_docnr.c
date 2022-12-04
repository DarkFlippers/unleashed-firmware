#include "../nfc_i.h"
#include "m-string.h"
#include <gui/modules/validators.h>

#define TAG "PassportDocnr"

void nfc_scene_passport_docnr_text_input_callback(void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventTextInputDone);
}

void nfc_scene_passport_docnr_on_enter(void* context) {
    Nfc* nfc = context;

    TextInput* text_input = nfc->text_input;
    text_input_set_header_text(text_input, "Document Nr.");

    char* docnr = nfc->dev->dev_data.mrtd_data.auth.doc_number;
    bool docnr_empty = false;

    if(*docnr) {
        nfc_text_store_set(nfc, docnr);
        docnr_empty = false;
    } else {
        nfc_text_store_set(nfc, "PA7HJ34M8");
        docnr_empty = true;
    }

    text_input_set_result_callback(
        text_input,
        nfc_scene_passport_docnr_text_input_callback,
        nfc,
        nfc->text_store,
        MRTD_DOCNR_MAX_LENGTH, // incl. '\x00'
        docnr_empty); // Use as template

    //TODO: add validator?

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextInput);
}

bool nfc_scene_passport_docnr_save(Nfc* nfc) {
    strncpy(nfc->dev->dev_data.mrtd_data.auth.doc_number, nfc->text_store, MRTD_DOCNR_MAX_LENGTH);
    return true;
}

bool nfc_scene_passport_docnr_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventTextInputDone) {
            nfc_scene_passport_docnr_save(nfc);

            consumed = scene_manager_search_and_switch_to_previous_scene(
                nfc->scene_manager, NfcScenePassportAuth);
        }
    }
    return consumed;
}

void nfc_scene_passport_docnr_on_exit(void* context) {
    Nfc* nfc = context;

    text_input_reset(nfc->text_input);
}
