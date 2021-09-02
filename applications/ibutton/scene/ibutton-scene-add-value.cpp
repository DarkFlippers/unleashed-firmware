#include "ibutton-scene-add-value.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

void iButtonSceneAddValue::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    ByteInput* byte_input = view_manager->get_byte_input();
    auto callback = cbc::obtain_connector(this, &iButtonSceneAddValue::byte_input_callback);
    memcpy(this->new_key_data, app->get_key()->get_data(), app->get_key()->get_type_data_size());
    byte_input_set_result_callback(
        byte_input, callback, NULL, app, this->new_key_data, app->get_key()->get_type_data_size());
    byte_input_set_header_text(byte_input, "Enter the key");

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewByteInput);
}

bool iButtonSceneAddValue::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeByteEditResult) {
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

void iButtonSceneAddValue::byte_input_callback(void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeByteEditResult;
    memcpy(app->get_key()->get_data(), this->new_key_data, app->get_key()->get_type_data_size());
    app->get_view_manager()->send_event(&event);
}