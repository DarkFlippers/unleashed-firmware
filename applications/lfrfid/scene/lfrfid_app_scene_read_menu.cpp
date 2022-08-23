#include "lfrfid_app_scene_read_menu.h"

typedef enum {
    SubmenuSave,
    SubmenuEmulate,
    SubmenuWrite,
} SubmenuIndex;

void LfRfidAppSceneReadKeyMenu::on_enter(LfRfidApp* app, bool need_restore) {
    auto submenu = app->view_controller.get<SubmenuVM>();

    submenu->add_item("Save", SubmenuSave, submenu_callback, app);
    submenu->add_item("Emulate", SubmenuEmulate, submenu_callback, app);
    submenu->add_item("Write", SubmenuWrite, submenu_callback, app);

    if(need_restore) {
        submenu->set_selected_item(submenu_item_selected);
    }

    app->view_controller.switch_to<SubmenuVM>();
}

bool LfRfidAppSceneReadKeyMenu::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::MenuSelected) {
        submenu_item_selected = event->payload.signed_int;
        switch(event->payload.signed_int) {
        case SubmenuWrite:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::Write);
            break;
        case SubmenuSave:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SaveName);
            break;
        case SubmenuEmulate:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::Emulate);
            break;
        }
        consumed = true;
    } else if(event->type == LfRfidApp::EventType::Back) {
        app->scene_controller.switch_to_previous_scene();
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneReadKeyMenu::on_exit(LfRfidApp* app) {
    app->view_controller.get<SubmenuVM>()->clean();
}

void LfRfidAppSceneReadKeyMenu::submenu_callback(void* context, uint32_t index) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;

    event.type = LfRfidApp::EventType::MenuSelected;
    event.payload.signed_int = index;

    app->view_controller.send_event(&event);
}
