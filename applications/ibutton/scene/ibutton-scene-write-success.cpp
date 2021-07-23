#include "ibutton-scene-write-success.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include "../ibutton-key.h"
#include <callback-connector.h>

void iButtonSceneWriteSuccess::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    auto callback = cbc::obtain_connector(this, &iButtonSceneWriteSuccess::popup_callback);

    popup_set_icon(popup, 0, 12, &I_iButtonDolphinVerySuccess_108x52);
    popup_set_text(popup, "Successfully written!", 40, 12, AlignLeft, AlignBottom);

    popup_set_callback(popup, callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);

    app->notify_success();
    app->notify_green_on();
}

bool iButtonSceneWriteSuccess::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeBack) {
        app->search_and_switch_to_previous_scene(
            {iButtonApp::Scene::SceneReadedKeyMenu, iButtonApp::Scene::SceneStart});
        consumed = true;
    }

    return consumed;
}

void iButtonSceneWriteSuccess::on_exit(iButtonApp* app) {
    Popup* popup = app->get_view_manager()->get_popup();

    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    popup_disable_timeout(popup);
    popup_set_context(popup, NULL);
    popup_set_callback(popup, NULL);
}

void iButtonSceneWriteSuccess::popup_callback(void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;
    event.type = iButtonEvent::Type::EventTypeBack;
    app->get_view_manager()->send_event(&event);
    app->notify_green_off();
}