#include "lfrfid-app-scene-readed-menu.h"

typedef enum {
    SubmenuWrite,
    SubmenuNameAndSave,
    SubmenuEmulate,
} SubmenuIndex;

void LfRfidAppSceneReadedMenu::on_enter(LfRfidApp* app, bool need_restore) {
    auto submenu = app->view_controller.get<SubmenuVM>();

    submenu->add_item("Write", SubmenuWrite, submenu_callback, app);
    submenu->add_item("Name and Save", SubmenuNameAndSave, submenu_callback, app);
    submenu->add_item("Emulate", SubmenuEmulate, submenu_callback, app);

    if(need_restore) {
        submenu->set_selected_item(submenu_item_selected);
    }

    app->view_controller.switch_to<SubmenuVM>();
}

bool LfRfidAppSceneReadedMenu::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuWrite:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::Write);
            break;
        case SubmenuNameAndSave:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SaveName);
            break;
        case SubmenuEmulate:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::Emulate);
            break;
        }
        consumed = true;
    } else if(event->type == LfRfidApp::EventType::Back) {
        app->scene_controller.search_and_switch_to_previous_scene({LfRfidApp::SceneType::Start});
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneReadedMenu::on_exit(LfRfidApp* app) {
    app->view_controller.get<SubmenuVM>()->clean();
}

void LfRfidAppSceneReadedMenu::submenu_callback(void* context, uint32_t index) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;

    event.type = LfRfidApp::EventType::MenuSelected;
    event.payload.menu_index = index;

    app->view_controller.send_event(&event);
}
