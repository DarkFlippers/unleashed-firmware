#include "ibutton-scene-readed-key-menu.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexWrite,
    SubmenuIndexEmulate,
    SubmenuIndexNameAndSave,
} SubmenuIndex;

void iButtonSceneReadedKeyMenu::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &iButtonSceneReadedKeyMenu::submenu_callback);

    submenu_add_item(submenu, "Write", SubmenuIndexWrite, callback, app);
    submenu_add_item(submenu, "Name and save", SubmenuIndexNameAndSave, callback, app);
    submenu_add_item(submenu, "Emulate", SubmenuIndexEmulate, callback, app);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneReadedKeyMenu::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        switch(event->payload.menu_index) {
        case SubmenuIndexWrite:
            app->switch_to_next_scene(iButtonApp::Scene::SceneWrite);
            break;
        case SubmenuIndexEmulate:
            app->switch_to_next_scene(iButtonApp::Scene::SceneEmulate);
            break;
        case SubmenuIndexNameAndSave:
            app->switch_to_next_scene(iButtonApp::Scene::SceneSaveName);
            break;
        }
        consumed = true;
    } else if(event->type == iButtonEvent::Type::EventTypeBack) {
        app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneRead});
        consumed = true;
    }

    return consumed;
}

void iButtonSceneReadedKeyMenu::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_clean(submenu);
}

void iButtonSceneReadedKeyMenu::submenu_callback(void* context, uint32_t index) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}