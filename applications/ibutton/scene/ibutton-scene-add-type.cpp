#include "ibutton-scene-add-type.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexCyfral,
    SubmenuIndexDallas,
    SubmenuIndexMetakom,
} SubmenuIndex;

void iButtonSceneAddType::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &iButtonSceneAddType::submenu_callback);

    submenu_add_item(submenu, "Cyfral", SubmenuIndexCyfral, callback, app);
    submenu_add_item(submenu, "Dallas", SubmenuIndexDallas, callback, app);
    submenu_add_item(submenu, "Metakom", SubmenuIndexMetakom, callback, app);
    submenu_set_selected_item(submenu, submenu_item_selected);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneAddType::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuIndexCyfral:
            app->get_key()->set_type(iButtonKeyType::KeyCyfral);
            break;
        case SubmenuIndexDallas:
            app->get_key()->set_type(iButtonKeyType::KeyDallas);
            break;
        case SubmenuIndexMetakom:
            app->get_key()->set_type(iButtonKeyType::KeyMetakom);
            break;
        }
        app->get_key()->set_name("");
        app->get_key()->clear_data();
        app->switch_to_next_scene(iButtonApp::Scene::SceneAddValue);
        consumed = true;
    }

    return consumed;
}

void iButtonSceneAddType::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_clean(submenu);
}

void iButtonSceneAddType::submenu_callback(void* context, uint32_t index) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}