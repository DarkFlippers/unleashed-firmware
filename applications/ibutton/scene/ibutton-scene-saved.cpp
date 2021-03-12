#include "ibutton-scene-saved.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

void iButtonSceneSavedList::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &iButtonSceneSavedList::submenu_callback);

    KeyStore* store = app->get_key_store();

    if(store->get_key_count() > 0) {
        for(uint8_t i = 0; i < store->get_key_count(); i++) {
            submenu_add_item(submenu, store->get_key_name(i), i, callback, app);
        }
    } else {
        submenu_add_item(submenu, "Empty", 0, NULL, NULL);
    }

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewSubmenu);
}

bool iButtonSceneSavedList::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeMenuSelected) {
        uint8_t key_index = event->payload.menu_index;

        // store data
        iButtonKey* stored_key_data = app->get_key();
        KeyStore* store = app->get_key_store();

        app->set_stored_key_index(key_index);
        stored_key_data->set_name(store->get_key_name(key_index));
        stored_key_data->set_type(store->get_key_type(key_index));
        stored_key_data->set_data(store->get_key_data(key_index), stored_key_data->get_size());

        app->switch_to_next_scene(iButtonApp::Scene::SceneSavedKeyMenu);
        consumed = true;
    }

    return consumed;
}

void iButtonSceneSavedList::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view = app->get_view_manager();
    Submenu* submenu = view->get_submenu();

    submenu_clean(submenu);
}

void iButtonSceneSavedList::submenu_callback(void* context, uint32_t index) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeMenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}