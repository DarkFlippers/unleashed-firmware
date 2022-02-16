#include "../irda_app.h"

void IrdaAppSceneEditDeleteDone::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();

    popup_set_icon(popup, 0, 2, &I_DolphinMafia_115x62);
    popup_set_header(popup, "Deleted", 83, 19, AlignLeft, AlignBottom);

    popup_set_callback(popup, IrdaApp::popup_callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(IrdaAppViewManager::ViewType::Popup);
}

bool IrdaAppSceneEditDeleteDone::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::PopupTimer) {
        if(app->get_edit_element() == IrdaApp::EditElement::Remote) {
            app->search_and_switch_to_previous_scene(
                {IrdaApp::Scene::Start, IrdaApp::Scene::RemoteList});
        } else {
            app->search_and_switch_to_previous_scene({IrdaApp::Scene::Remote});
        }
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneEditDeleteDone::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, nullptr, 0, 0, AlignLeft, AlignTop);
}
