#include "../irda-app.hpp"

void IrdaAppSceneLearnDone::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();

    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);

    if(app->get_learn_new_remote()) {
        popup_set_text(popup, "New remote\ncreated!", 5, 7, AlignLeft, AlignTop);
    } else {
        popup_set_text(popup, "Saved!", 5, 7, AlignLeft, AlignTop);
    }

    popup_set_callback(popup, IrdaApp::popup_callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(IrdaAppViewManager::ViewType::Popup);
}

bool IrdaAppSceneLearnDone::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::PopupTimer) {
        app->switch_to_next_scene(IrdaApp::Scene::Remote);
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneLearnDone::on_exit(IrdaApp* app) {
    app->set_learn_new_remote(false);
}
