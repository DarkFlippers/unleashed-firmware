#include <nfc/scenes/nfc_scene_set_atqa.h>

#include <furi.h>

#include "../nfc_i.h"

#include <gui/view_dispatcher.h>

#define SCENE_SET_ATQA_CUSTOM_EVENT (0UL)

void nfc_scene_set_atqa_byte_input_callback(void* context, uint8_t* bytes, uint8_t bytes_count) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(
        nfc->nfc_common.view_dispatcher, SCENE_SET_ATQA_CUSTOM_EVENT);
}

const void nfc_scene_set_atqa_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter atqa in hex");
    byte_input_set_result_callback(
        byte_input, nfc_scene_set_atqa_byte_input_callback, NULL, nfc, nfc->device.data.atqa, 2);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewByteInput);
}

const bool nfc_scene_set_atqa_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == SCENE_SET_ATQA_CUSTOM_EVENT) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_set_uid);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    }
    return false;
}

const void nfc_scene_set_atqa_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}

AppScene* nfc_scene_set_atqa_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneSetAtqa;
    scene->on_enter = nfc_scene_set_atqa_on_enter;
    scene->on_event = nfc_scene_set_atqa_on_event;
    scene->on_exit = nfc_scene_set_atqa_on_exit;

    return scene;
}

void nfc_scene_set_atqa_free(AppScene* scene) {
    free(scene);
}
