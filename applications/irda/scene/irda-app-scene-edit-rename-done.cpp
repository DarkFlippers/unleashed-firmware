#include "../irda-app.h"

void IrdaAppSceneEditRenameDone::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();

    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);

    popup_set_text(popup, "Saved!", 13, 22, AlignLeft, AlignTop);

    popup_set_callback(popup, IrdaApp::popup_callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(IrdaAppViewManager::ViewType::Popup);
}

bool IrdaAppSceneEditRenameDone::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::PopupTimer) {
        app->switch_to_next_scene(IrdaApp::Scene::Remote);
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneEditRenameDone::on_exit(IrdaApp* app) {
}
