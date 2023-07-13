#include "../nfc_maker.h"

enum ByteInputResult {
    ByteInputResultOk,
};

static void nfc_maker_scene_bluetooth_byte_input_callback(void* context) {
    NfcMaker* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, ByteInputResultOk);
}

void nfc_maker_scene_bluetooth_on_enter(void* context) {
    NfcMaker* app = context;
    ByteInput* byte_input = app->byte_input;

    byte_input_set_header_text(byte_input, "Enter Bluetooth MAC:");

    for(size_t i = 0; i < MAC_INPUT_LEN; i++) {
        app->mac_buf[i] = 0x69;
    }

    byte_input_set_result_callback(
        byte_input,
        nfc_maker_scene_bluetooth_byte_input_callback,
        NULL,
        app,
        app->mac_buf,
        MAC_INPUT_LEN);

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcMakerViewByteInput);
}

bool nfc_maker_scene_bluetooth_on_event(void* context, SceneManagerEvent event) {
    NfcMaker* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case ByteInputResultOk:
            furi_hal_bt_reverse_mac_addr(app->mac_buf);
            scene_manager_next_scene(app->scene_manager, NfcMakerSceneSave);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void nfc_maker_scene_bluetooth_on_exit(void* context) {
    NfcMaker* app = context;
    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
