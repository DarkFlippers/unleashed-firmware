#include "ibutton-scene-start.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexRead,
    SubmenuIndexSaved,
    SubmenuIndexAdd,
} SubmenuIndex;

void iButtonSceneStart::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &iButtonSceneStart::submenu_callback);

    submenu_add_item(submenu, "Read", SubmenuIndexRead, callback, app);
    submenu_add_item(submenu, "Saved", SubmenuIndexSaved, callback, app);
    submenu_add_item(submenu, "Add manually", SubmenuIndexAdd, callback, app);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneStart::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        switch(event->payload.menu_index) {
        case SubmenuIndexRead:
            app->switch_to_next_scene(iButtonApp::Scene::SceneRead);
            break;
        case SubmenuIndexSaved:
            app->switch_to_next_scene(iButtonApp::Scene::SceneSavedList);
            break;
        case SubmenuIndexAdd:
            app->switch_to_next_scene(iButtonApp::Scene::SceneAddType);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void iButtonSceneStart::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_clean(submenu);
}

void iButtonSceneStart::submenu_callback(void* context, uint32_t index) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}