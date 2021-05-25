#include "ibutton-scene-saved-key-menu.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexEmulate,
    SubmenuIndexWrite,
    SubmenuIndexEdit,
    SubmenuIndexDelete,
    SubmenuIndexInfo,
} SubmenuIndex;

void iButtonSceneSavedKeyMenu::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &iButtonSceneSavedKeyMenu::submenu_callback);

    submenu_add_item(submenu, "Emulate", SubmenuIndexEmulate, callback, app);
    submenu_add_item(submenu, "Write", SubmenuIndexWrite, callback, app);
    submenu_add_item(submenu, "Edit", SubmenuIndexEdit, callback, app);
    submenu_add_item(submenu, "Delete", SubmenuIndexDelete, callback, app);
    submenu_add_item(submenu, "Info", SubmenuIndexInfo, callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneSavedKeyMenu::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexWrite:
            app->switch_to_next_scene(iButtonApp::Scene::SceneWrite);
            break;
        case SubmenuIndexEmulate:
            app->switch_to_next_scene(iButtonApp::Scene::SceneEmulate);
            break;
        case SubmenuIndexEdit:
            app->switch_to_next_scene(iButtonApp::Scene::SceneAddValue);
            break;
        case SubmenuIndexDelete:
            app->switch_to_next_scene(iButtonApp::Scene::SceneDeleteConfirm);
            break;
        case SubmenuIndexInfo:
            app->switch_to_next_scene(iButtonApp::Scene::SceneInfo);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void iButtonSceneSavedKeyMenu::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_clean(submenu);
}

void iButtonSceneSavedKeyMenu::submenu_callback(void* context, uint32_t index) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}