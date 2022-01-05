#include "ibutton_scene_save_success.h"
#include "../ibutton_app.h"
#include "../ibutton_view_manager.h"
#include "../ibutton_event.h"
#include "../ibutton_key.h"
#include <callback-connector.h>

void iButtonSceneSaveSuccess::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    auto callback = cbc::obtain_connector(this, &iButtonSceneSaveSuccess::popup_callback);

    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_text(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);

    popup_set_callback(popup, callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);
}

bool iButtonSceneSaveSuccess::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeBack) {
        app->search_and_switch_to_previous_scene(
            {iButtonApp::Scene::SceneReadedKeyMenu,
             iButtonApp::Scene::SceneSavedKeyMenu,
             iButtonApp::Scene::SceneAddType});
        consumed = true;
    }

    return consumed;
}

void iButtonSceneSaveSuccess::on_exit(iButtonApp* app) {
    Popup* popup = app->get_view_manager()->get_popup();

    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    popup_disable_timeout(popup);
    popup_set_context(popup, NULL);
    popup_set_callback(popup, NULL);
}

void iButtonSceneSaveSuccess::popup_callback(void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;
    event.type = iButtonEvent::Type::EventTypeBack;
    app->get_view_manager()->send_event(&event);
}