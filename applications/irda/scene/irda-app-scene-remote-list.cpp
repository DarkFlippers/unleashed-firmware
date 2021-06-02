#include "../irda-app.hpp"

typedef enum {
    SubmenuIndexPlus = -1,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneRemoteList::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto remote_manager = app->get_remote_manager();
    int i = 0;

    remote_names = remote_manager->get_remote_list();
    for(auto& a : remote_names) {
        submenu_add_item(submenu, a.c_str(), i++, submenu_callback, app);
    }
    submenu_add_item(
        submenu, "                           +", SubmenuIndexPlus, submenu_callback, app);

    view_manager->switch_to(IrdaAppViewManager::ViewType::Submenu);
}

bool IrdaAppSceneRemoteList::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::MenuSelected) {
        switch(event->payload.menu_index) {
        case SubmenuIndexPlus:
            app->set_learn_new_remote(true);
            app->switch_to_next_scene(IrdaApp::Scene::Learn);
            break;
        default:
            auto remote_manager = app->get_remote_manager();
            remote_manager->set_current_remote(event->payload.menu_index);
            app->switch_to_next_scene(IrdaApp::Scene::Remote);
            consumed = true;
            break;
        }
    }

    return consumed;
}

void IrdaAppSceneRemoteList::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_clean(submenu);
}
