#include "ibutton_scene_add_value.h"
#include "../ibutton_app.h"
#include <dolphin/dolphin.h>

static void byte_input_callback(void* context) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeByteEditResult;
    app->get_view_manager()->send_event(&event);
}

void iButtonSceneAddValue::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    ByteInput* byte_input = view_manager->get_byte_input();
    iButtonKey* key = app->get_key();

    memcpy(this->new_key_data, ibutton_key_get_data_p(key), ibutton_key_get_data_size(key));

    byte_input_set_result_callback(
        byte_input,
        byte_input_callback,
        NULL,
        app,
        this->new_key_data,
        ibutton_key_get_data_size(key));

    byte_input_set_header_text(byte_input, "Enter the key");

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewByteInput);
}

bool iButtonSceneAddValue::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeByteEditResult) {
        ibutton_key_set_data(app->get_key(), this->new_key_data, IBUTTON_KEY_DATA_SIZE);
        DOLPHIN_DEED(DolphinDeedIbuttonAdd);
        app->switch_to_next_scene(iButtonApp::Scene::SceneSaveName);
        consumed = true;
    }

    return consumed;
}

void iButtonSceneAddValue::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    ByteInput* byte_input = view_manager->get_byte_input();

    byte_input_set_result_callback(byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(byte_input, {0});
}