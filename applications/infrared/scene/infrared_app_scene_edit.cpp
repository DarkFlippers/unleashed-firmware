#include "../infrared_app.h"
#include "gui/modules/submenu.h"

typedef enum {
    SubmenuIndexAddKey,
    SubmenuIndexRenameKey,
    SubmenuIndexDeleteKey,
    SubmenuIndexRenameRemote,
    SubmenuIndexDeleteRemote,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;

    event.type = InfraredAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void InfraredAppSceneEdit::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(submenu, "Add Key", SubmenuIndexAddKey, submenu_callback, app);
    submenu_add_item(submenu, "Rename Key", SubmenuIndexRenameKey, submenu_callback, app);
    submenu_add_item(submenu, "Delete Key", SubmenuIndexDeleteKey, submenu_callback, app);
    submenu_add_item(submenu, "Rename Remote", SubmenuIndexRenameRemote, submenu_callback, app);
    submenu_add_item(submenu, "Delete Remote", SubmenuIndexDeleteRemote, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);
    submenu_item_selected = 0;

    view_manager->switch_to(InfraredAppViewManager::ViewId::Submenu);
}

bool InfraredAppSceneEdit::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexAddKey:
            app->set_learn_new_remote(false);
            app->switch_to_next_scene(InfraredApp::Scene::Learn);
            break;
        case SubmenuIndexRenameKey:
            app->set_edit_action(InfraredApp::EditAction::Rename);
            app->set_edit_element(InfraredApp::EditElement::Button);
            app->switch_to_next_scene(InfraredApp::Scene::EditKeySelect);
            break;
        case SubmenuIndexDeleteKey:
            app->set_edit_action(InfraredApp::EditAction::Delete);
            app->set_edit_element(InfraredApp::EditElement::Button);
            app->switch_to_next_scene(InfraredApp::Scene::EditKeySelect);
            break;
        case SubmenuIndexRenameRemote:
            app->set_edit_action(InfraredApp::EditAction::Rename);
            app->set_edit_element(InfraredApp::EditElement::Remote);
            app->switch_to_next_scene(InfraredApp::Scene::EditRename);
            break;
        case SubmenuIndexDeleteRemote:
            app->set_edit_action(InfraredApp::EditAction::Delete);
            app->set_edit_element(InfraredApp::EditElement::Remote);
            app->switch_to_next_scene(InfraredApp::Scene::EditDelete);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void InfraredAppSceneEdit::on_exit(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_reset(submenu);
}
