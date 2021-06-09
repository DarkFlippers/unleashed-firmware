#include "../irda-app.hpp"

typedef enum {
    SubmenuIndexUniversalLibrary,
    SubmenuIndexLearnNewRemote,
    SubmenuIndexSavedRemotes,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneStart::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(
        submenu, "Universal library", SubmenuIndexUniversalLibrary, submenu_callback, app);
    submenu_add_item(
        submenu, "Learn new remote", SubmenuIndexLearnNewRemote, submenu_callback, app);
    submenu_add_item(submenu, "Saved remotes", SubmenuIndexSavedRemotes, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);
    submenu_item_selected = 0;

    view_manager->switch_to(IrdaAppViewManager::ViewType::Submenu);
}

bool IrdaAppSceneStart::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexUniversalLibrary:
            app->switch_to_next_scene(IrdaApp::Scene::Universal);
            break;
        case SubmenuIndexLearnNewRemote:
            app->set_learn_new_remote(true);
            app->switch_to_next_scene(IrdaApp::Scene::Learn);
            break;
        case SubmenuIndexSavedRemotes:
            app->switch_to_next_scene(IrdaApp::Scene::RemoteList);
            break;
        default:
            furi_assert(0);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneStart::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_clean(submenu);
}
