#include "lfrfid-app-scene-save-type.h"

void LfRfidAppSceneSaveType::on_enter(LfRfidApp* app, bool need_restore) {
    auto submenu = app->view_controller.get<SubmenuVM>();

    for(uint8_t i = 0; i <= static_cast<uint8_t>(LfrfidKeyType::KeyI40134); i++) {
        submenu->add_item(
            lfrfid_key_get_type_string(static_cast<LfrfidKeyType>(i)), i, submenu_callback, app);
    }

    if(need_restore) {
        submenu->set_selected_item(submenu_item_selected);
    }

    app->view_controller.switch_to<SubmenuVM>();

    // clear key name
    app->worker.key.set_name("");
}

bool LfRfidAppSceneSaveType::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        app->worker.key.set_type(static_cast<LfrfidKeyType>(event->payload.menu_index));
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SaveData);
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneSaveType::on_exit(LfRfidApp* app) {
    app->view_controller.get<SubmenuVM>()->clean();
}

void LfRfidAppSceneSaveType::submenu_callback(void* context, uint32_t index) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;

    event.type = LfRfidApp::EventType::MenuSelected;
    event.payload.menu_index = index;

    app->view_controller.send_event(&event);
}
