#include <nfc/scenes/nfc_scene_save_name.h>

#include <furi.h>

#include "../nfc_i.h"
#include "../views/nfc_detect.h"

#include <gui/view_dispatcher.h>

#define SCENE_SAVE_NAME_CUSTOM_EVENT (0UL)

void nfc_scene_save_name_text_input_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(
        nfc->nfc_common.view_dispatcher, SCENE_SAVE_NAME_CUSTOM_EVENT);
}

const void nfc_scene_save_name_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    TextInput* text_input = nfc->text_input;
    nfc_set_text_store(nfc, "");
    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        nfc_scene_save_name_text_input_callback,
        nfc,
        nfc->text_store,
        sizeof(nfc->text_store));
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewTextInput);
}

const bool nfc_scene_save_name_on_event(void* context, uint32_t event) {
    Nfc* nfc = (Nfc*)context;

    if(event == SCENE_SAVE_NAME_CUSTOM_EVENT) {
        memcpy(&nfc->device.dev_name, nfc->text_store, strlen(nfc->text_store));
        if(nfc_device_save(&nfc->device, "test")) {
            view_dispatcher_add_scene(nfc->nfc_common.view_dispatcher, nfc->scene_save_success);
            view_dispatcher_send_navigation_event(
                nfc->nfc_common.view_dispatcher, ViewNavigatorEventNext);
        } else {
            view_dispatcher_send_back_search_scene_event(
                nfc->nfc_common.view_dispatcher, NfcSceneStart);
        }
        return true;
    }
    return false;
}

const void nfc_scene_save_name_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    text_input_set_header_text(nfc->text_input, NULL);
}

AppScene* nfc_scene_save_name_alloc() {
    AppScene* scene = furi_alloc(sizeof(AppScene));
    scene->id = NfcSceneSaveName;
    scene->on_enter = nfc_scene_save_name_on_enter;
    scene->on_event = nfc_scene_save_name_on_event;
    scene->on_exit = nfc_scene_save_name_on_exit;

    return scene;
}

void nfc_scene_save_name_free(AppScene* scene) {
    free(scene);
}
