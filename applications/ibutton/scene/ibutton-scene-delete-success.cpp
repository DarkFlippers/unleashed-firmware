#include "ibutton-scene-delete-success.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include "../ibutton-key.h"
#include <callback-connector.h>

void iButtonSceneDeleteSuccess::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    auto callback = cbc::obtain_connector(this, &iButtonSceneDeleteSuccess::popup_callback);

    popup_set_icon(popup, 0, 2, I_DolphinMafia_115x62);
    popup_set_text(popup, "Deleted", 83, 19, AlignLeft, AlignBottom);

    popup_set_callback(popup, callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);
}

bool iButtonSceneDeleteSuccess::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeBack) {
        app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneSelectKey});
        consumed = true;
    }

    return consumed;
}

void iButtonSceneDeleteSuccess::on_exit(iButtonApp* app) {
    Popup* popup = app->get_view_manager()->get_popup();

    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, -1, -1, I_DolphinWait_61x59);

    popup_disable_timeout(popup);
    popup_set_context(popup, NULL);
    popup_set_callback(popup, NULL);
}

void iButtonSceneDeleteSuccess::popup_callback(void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;
    event.type = iButtonEvent::Type::EventTypeBack;
    app->get_view_manager()->send_event(&event);
}