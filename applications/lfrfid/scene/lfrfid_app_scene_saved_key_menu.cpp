#include "lfrfid_app_scene_saved_key_menu.h"

typedef enum {
    SubmenuEmulate,
    SubmenuWrite,
    SubmenuEdit,
    SubmenuDelete,
    SubmenuInfo,
} SubmenuIndex;

void LfRfidAppSceneSavedKeyMenu::on_enter(LfRfidApp* app, bool need_restore) {
    auto submenu = app->view_controller.get<SubmenuVM>();

    submenu->add_item("Emulate", SubmenuEmulate, submenu_callback, app);
    submenu->add_item("Write", SubmenuWrite, submenu_callback, app);
    submenu->add_item("Edit", SubmenuEdit, submenu_callback, app);
    submenu->add_item("Delete", SubmenuDelete, submenu_callback, app);
    submenu->add_item("Info", SubmenuInfo, submenu_callback, app);

    if(need_restore) {
        submenu->set_selected_item(submenu_item_selected);
    }

    app->view_controller.switch_to<SubmenuVM>();
}

bool LfRfidAppSceneSavedKeyMenu::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuEmulate:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::Emulate);
            break;
        case SubmenuWrite:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::Write);
            break;
        case SubmenuEdit:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SaveData);
            break;
        case SubmenuDelete:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::DeleteConfirm);
            break;
        case SubmenuInfo:
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SavedInfo);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneSavedKeyMenu::on_exit(LfRfidApp* app) {
    app->view_controller.get<SubmenuVM>()->clean();
}

void LfRfidAppSceneSavedKeyMenu::submenu_callback(void* context, uint32_t index) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;

    event.type = LfRfidApp::EventType::MenuSelected;
    event.payload.menu_index = index;

    app->view_controller.send_event(&event);
}
