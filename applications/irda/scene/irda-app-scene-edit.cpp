#include "../irda-app.hpp"
#include "gui/modules/submenu.h"

typedef enum {
    SubmenuIndexAddKey,
    SubmenuIndexRenameKey,
    SubmenuIndexDeleteKey,
    SubmenuIndexRenameRemote,
    SubmenuIndexDeleteRemote,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneEdit::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(submenu, "Add key", SubmenuIndexAddKey, submenu_callback, app);
    submenu_add_item(submenu, "Rename key", SubmenuIndexRenameKey, submenu_callback, app);
    submenu_add_item(submenu, "Delete key", SubmenuIndexDeleteKey, submenu_callback, app);
    submenu_add_item(submenu, "Rename remote", SubmenuIndexRenameRemote, submenu_callback, app);
    submenu_add_item(submenu, "Delete remote", SubmenuIndexDeleteRemote, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);
    submenu_item_selected = 0;

    view_manager->switch_to(IrdaAppViewManager::ViewType::Submenu);
}

bool IrdaAppSceneEdit::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexAddKey:
            app->set_learn_new_remote(false);
            app->switch_to_next_scene(IrdaApp::Scene::Learn);
            break;
        case SubmenuIndexRenameKey:
            app->set_edit_action(IrdaApp::EditAction::Rename);
            app->set_edit_element(IrdaApp::EditElement::Button);
            app->switch_to_next_scene(IrdaApp::Scene::EditKeySelect);
            break;
        case SubmenuIndexDeleteKey:
            app->set_edit_action(IrdaApp::EditAction::Delete);
            app->set_edit_element(IrdaApp::EditElement::Button);
            app->switch_to_next_scene(IrdaApp::Scene::EditKeySelect);
            break;
        case SubmenuIndexRenameRemote:
            app->set_edit_action(IrdaApp::EditAction::Rename);
            app->set_edit_element(IrdaApp::EditElement::Remote);
            app->switch_to_next_scene(IrdaApp::Scene::EditRename);
            break;
        case SubmenuIndexDeleteRemote:
            app->set_edit_action(IrdaApp::EditAction::Delete);
            app->set_edit_element(IrdaApp::EditElement::Remote);
            app->switch_to_next_scene(IrdaApp::Scene::EditDelete);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneEdit::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_clean(submenu);
}
