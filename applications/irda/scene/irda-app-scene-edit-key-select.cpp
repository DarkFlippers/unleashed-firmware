#include "../irda-app.hpp"
#include "gui/modules/submenu.h"

static void submenu_callback(void* context, uint32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneEditKeySelect::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    int i = 0;

    const char* header = app->get_edit_action() == IrdaApp::EditAction::Rename ? "Rename key:" :
                                                                                 "Delete key:";
    submenu_set_header(submenu, header);

    auto remote_manager = app->get_remote_manager();
    buttons_names = remote_manager->get_button_list();
    for(const auto& it : buttons_names) {
        submenu_add_item(submenu, it.c_str(), i++, submenu_callback, app);
    }

    view_manager->switch_to(IrdaAppViewManager::ViewType::Submenu);
}

bool IrdaAppSceneEditKeySelect::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::MenuSelected) {
        auto remote_manager = app->get_remote_manager();
        remote_manager->set_current_button(event->payload.menu_index);
        consumed = true;
        if(app->get_edit_action() == IrdaApp::EditAction::Rename) {
            app->switch_to_next_scene(IrdaApp::Scene::EditRename);
        } else {
            app->switch_to_next_scene(IrdaApp::Scene::EditDelete);
        }
    }

    return consumed;
}

void IrdaAppSceneEditKeySelect::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_clean(submenu);
}
