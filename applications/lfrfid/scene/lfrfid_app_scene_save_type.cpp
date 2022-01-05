#include "lfrfid_app_scene_save_type.h"

void LfRfidAppSceneSaveType::on_enter(LfRfidApp* app, bool need_restore) {
    auto submenu = app->view_controller.get<SubmenuVM>();

    for(uint8_t i = 0; i <= keys_count; i++) {
        string_init_printf(
            submenu_name[i],
            "%s %s",
            lfrfid_key_get_manufacturer_string(static_cast<LfrfidKeyType>(i)),
            lfrfid_key_get_type_string(static_cast<LfrfidKeyType>(i)));
        submenu->add_item(string_get_cstr(submenu_name[i]), i, submenu_callback, app);
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
    for(uint8_t i = 0; i <= keys_count; i++) {
        string_clear(submenu_name[i]);
    }
}

void LfRfidAppSceneSaveType::submenu_callback(void* context, uint32_t index) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;

    event.type = LfRfidApp::EventType::MenuSelected;
    event.payload.menu_index = index;

    app->view_controller.send_event(&event);
}
