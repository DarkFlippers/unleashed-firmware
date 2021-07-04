#include <nfc/scenes/nfc_scene_set_uid.h>

#include <furi.h>

#include "../nfc_i.h"

#include <gui/view_dispatcher.h>

#define SCENE_SET_UID_CUSTOM_EVENT (0UL)

void nfc_scene_set_uid_byte_input_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, SCENE_SET_UID_CUSTOM_EVENT);
}

const void nfc_scene_set_uid_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    ByteInput* byte_input = nfc->byte_input;
    byte_input_set_header_text(byte_input, "Enter uid in hex");
    byte_input_set_result_callback(
        byte_input,
        nfc_scene_set_uid_byte_input_callback,
        NULL,
        nfc,
        nfc->device.data.uid,
        nfc->device.data.uid_len);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewByteInput);
}

const bool nfc_scene_set_uid_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == SCENE_SET_UID_CUSTOM_EVENT) {
        view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_save_name);
        view_dispatcher_send_navigation_event(
            nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        return true;
    }
    return false;
}

const void nfc_scene_set_uid_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    byte_input_set_result_callback(nfc->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(nfc->byte_input, "");
}

AppScene* nfc_scene_set_uid_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneSetUid;
    scene->on_enter = nfc_scene_set_uid_on_enter;
    scene->on_event = nfc_scene_set_uid_on_event;
    scene->on_exit = nfc_scene_set_uid_on_exit;

    return scene;
}

void nfc_scene_set_uid_free(AppScene* scene) {
    free(scene);
}
