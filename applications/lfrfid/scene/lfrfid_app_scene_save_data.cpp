#include "lfrfid_app_scene_save_data.h"
#include <dolphin/dolphin.h>

void LfRfidAppSceneSaveData::on_enter(LfRfidApp* app, bool need_restore) {
    auto byte_input = app->view_controller.get<ByteInputVM>();
    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);

    if(need_restore) {
        protocol_dict_set_data(app->dict, app->protocol_id, app->old_key_data, size);
    } else {
        protocol_dict_get_data(app->dict, app->protocol_id, app->old_key_data, size);
    }

    protocol_dict_get_data(app->dict, app->protocol_id, app->new_key_data, size);

    byte_input->set_header_text("Enter the data in hex");

    byte_input->set_result_callback(save_callback, NULL, app, app->new_key_data, size);

    app->view_controller.switch_to<ByteInputVM>();
}

bool LfRfidAppSceneSaveData::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Next) {
        size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
        protocol_dict_set_data(app->dict, app->protocol_id, app->new_key_data, size);
        DOLPHIN_DEED(DolphinDeedRfidAdd);
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SaveName);
    }

    return consumed;
}

void LfRfidAppSceneSaveData::on_exit(LfRfidApp* app) {
    app->view_controller.get<ByteInputVM>()->clean();
}

void LfRfidAppSceneSaveData::save_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}
