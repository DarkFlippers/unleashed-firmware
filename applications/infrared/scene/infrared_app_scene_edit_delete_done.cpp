#include "../infrared_app.h"

void InfraredAppSceneEditDeleteDone::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();

    popup_set_icon(popup, 0, 2, &I_DolphinMafia_115x62);
    popup_set_header(popup, "Deleted", 83, 19, AlignLeft, AlignBottom);

    popup_set_callback(popup, InfraredApp::popup_callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(InfraredAppViewManager::ViewId::Popup);
}

bool InfraredAppSceneEditDeleteDone::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::PopupTimer) {
        if(app->get_edit_element() == InfraredApp::EditElement::Remote) {
            app->search_and_switch_to_previous_scene(
                {InfraredApp::Scene::Start, InfraredApp::Scene::RemoteList});
        } else {
            app->search_and_switch_to_previous_scene({InfraredApp::Scene::Remote});
        }
        consumed = true;
    }

    return consumed;
}

void InfraredAppSceneEditDeleteDone::on_exit(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, nullptr, 0, 0, AlignLeft, AlignTop);
}
