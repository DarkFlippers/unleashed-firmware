#include "lfrfid_app_scene_start.h"

typedef enum {
    SubmenuRead,
    SubmenuSaved,
    SubmenuAddManually,
} SubmenuIndex;

void LfRfidAppSceneStart::on_enter(LfRfidApp* app, bool need_restore) {
    auto submenu = app->view_controller.get<SubmenuVM>();

    submenu->add_item("Read", SubmenuRead, submenu_callback, app);
    submenu->add_item("Saved", SubmenuSaved, submenu_callback, app);
    submenu->add_item("Add Manually", SubmenuAddManually, submenu_callback, app);

    if(need_restore) {
        submenu->set_selected_item(submenu_item_selected);
    }

    app->view_controller.switch_to<SubmenuVM>();

    // clear key
    app->worker.key.clear();
}

bool LfRfidAppSceneStart::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuRead:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::Read);
            break;
        case SubmenuSaved:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SelectKey);
            break;
        case SubmenuAddManually:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SaveType);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneStart::on_exit(LfRfidApp* app) {
    app->view_controller.get<SubmenuVM>()->clean();
}

void LfRfidAppSceneStart::submenu_callback(void* context, uint32_t index) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;

    event.type = LfRfidApp::EventType::MenuSelected;
    event.payload.menu_index = index;

    app->view_controller.send_event(&event);
}
