#include "lf-rfid-scene-start.h"
#include "../lf-rfid-app.h"
#include "../lf-rfid-view-manager.h"
#include "../lf-rfid-event.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexReadNormal,
    SubmenuIndexReadIndala,
    SubmenuIndexEmulateEM,
    SubmenuIndexEmulateHID,
    SubmenuIndexEmulateIndala,
    SubmenuIndexTune
} SubmenuIndex;

void LfrfidSceneStart::on_enter(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &LfrfidSceneStart::submenu_callback);

    submenu_add_item(submenu, "Read Normal", SubmenuIndexReadNormal, callback, app);
    submenu_add_item(submenu, "Read Indala", SubmenuIndexReadIndala, callback, app);
    submenu_add_item(submenu, "Emulate EM", SubmenuIndexEmulateEM, callback, app);
    submenu_add_item(submenu, "Emulate HID", SubmenuIndexEmulateHID, callback, app);
    submenu_add_item(submenu, "Emulate Indala", SubmenuIndexEmulateIndala, callback, app);
    submenu_add_item(submenu, "Tune", SubmenuIndexTune, callback, app);

    view_manager->switch_to(LfrfidAppViewManager::ViewType::Submenu);
}

bool LfrfidSceneStart::on_event(LfrfidApp* app, LfrfidEvent* event) {
    bool consumed = false;

    if(event->type == LfrfidEvent::Type::MenuSelected) {
        switch(event->payload.menu_index) {
        case SubmenuIndexReadNormal:
            app->switch_to_next_scene(LfrfidApp::Scene::ReadNormal);
            break;
        case SubmenuIndexReadIndala:
            app->switch_to_next_scene(LfrfidApp::Scene::ReadIndala);
            break;
        case SubmenuIndexEmulateEM:
            app->switch_to_next_scene(LfrfidApp::Scene::EmulateEM);
            break;
            break;
        case SubmenuIndexEmulateHID:
            app->switch_to_next_scene(LfrfidApp::Scene::EmulateHID);
            break;
        case SubmenuIndexEmulateIndala:
            app->switch_to_next_scene(LfrfidApp::Scene::EmulateIndala);
            break;
        case SubmenuIndexTune:
            app->switch_to_next_scene(LfrfidApp::Scene::Tune);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void LfrfidSceneStart::on_exit(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_clean(submenu);
}

void LfrfidSceneStart::submenu_callback(void* context, uint32_t index) {
    LfrfidApp* app = static_cast<LfrfidApp*>(context);
    LfrfidEvent event;

    event.type = LfrfidEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}