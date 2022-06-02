#include "lfrfid_app_scene_save_data.h"
#include <dolphin/dolphin.h>

void LfRfidAppSceneSaveData::on_enter(LfRfidApp* app, bool need_restore) {
    auto byte_input = app->view_controller.get<ByteInputVM>();
    RfidKey& key = app->worker.key;

    if(need_restore) printf("restored\r\n");

    if(need_restore) {
        key.set_data(old_key_data, key.get_type_data_count());
    } else {
        memcpy(old_key_data, key.get_data(), key.get_type_data_count());
    }

    memcpy(new_key_data, key.get_data(), key.get_type_data_count());
    byte_input->set_header_text("Enter the data in hex");

    byte_input->set_result_callback(
        save_callback, NULL, app, new_key_data, app->worker.key.get_type_data_count());

    app->view_controller.switch_to<ByteInputVM>();
}

bool LfRfidAppSceneSaveData::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;
    RfidKey& key = app->worker.key;

    if(event->type == LfRfidApp::EventType::Next) {
        key.set_data(new_key_data, key.get_type_data_count());
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
