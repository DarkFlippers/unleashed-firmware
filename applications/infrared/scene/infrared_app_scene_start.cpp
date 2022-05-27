#include "../infrared_app.h"

typedef enum {
    SubmenuIndexUniversalLibrary,
    SubmenuIndexLearnNewRemote,
    SubmenuIndexSavedRemotes,
} SubmenuIndex;

static void submenu_callback(void* context, uint32_t index) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;

    event.type = InfraredAppEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

void InfraredAppSceneStart::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_add_item(
        submenu, "Universal Library", SubmenuIndexUniversalLibrary, submenu_callback, app);
    submenu_add_item(
        submenu, "Learn New Remote", SubmenuIndexLearnNewRemote, submenu_callback, app);
    submenu_add_item(submenu, "Saved Remotes", SubmenuIndexSavedRemotes, submenu_callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);

    string_set_str(app->file_path, InfraredApp::infrared_directory);
    submenu_item_selected = 0;

    view_manager->switch_to(InfraredAppViewManager::ViewId::Submenu);
}

bool InfraredAppSceneStart::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexUniversalLibrary:
            app->switch_to_next_scene(InfraredApp::Scene::Universal);
            break;
        case SubmenuIndexLearnNewRemote:
            app->set_learn_new_remote(true);
            app->switch_to_next_scene(InfraredApp::Scene::Learn);
            break;
        case SubmenuIndexSavedRemotes:
            app->switch_to_next_scene(InfraredApp::Scene::RemoteList);
            break;
        default:
            furi_assert(0);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void InfraredAppSceneStart::on_exit(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    app->get_remote_manager()->reset_remote();
    submenu_reset(submenu);
}
