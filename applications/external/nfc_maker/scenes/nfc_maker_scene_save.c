#include "../nfc_maker.h"

enum TextInputResult {
    TextInputResultOk,
};

static void nfc_maker_scene_save_text_input_callback(void* context) {
    NfcMaker* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, TextInputResultOk);
}

void nfc_maker_scene_save_on_enter(void* context) {
    NfcMaker* app = context;
    NFCMaker_TextInput* text_input = app->text_input;

    nfc_maker_text_input_set_header_text(text_input, "Save the NFC tag:");

    set_random_name(app->save_buf, BIG_INPUT_LEN);

    nfc_maker_text_input_set_result_callback(
        text_input,
        nfc_maker_scene_save_text_input_callback,
        app,
        app->save_buf,
        BIG_INPUT_LEN,
        true);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(NFC_APP_FOLDER, NFC_APP_EXTENSION, NULL);
    nfc_maker_text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMakerViewTextInput);
}

bool nfc_maker_scene_save_on_event(void* context, SceneManagerEvent event) {
    NfcMaker* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case TextInputResultOk:
            scene_manager_next_scene(app->scene_manager, NfcMakerSceneResult);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void nfc_maker_scene_save_on_exit(void* context) {
    NfcMaker* app = context;
    nfc_maker_text_input_reset(app->text_input);
}
