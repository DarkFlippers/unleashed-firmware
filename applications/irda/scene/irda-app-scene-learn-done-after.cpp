#include "../irda-app.hpp"
#include <gui/modules/popup.h>

void IrdaAppSceneLearnDoneAfter::on_enter(IrdaApp* app) {
    auto view_manager = app->get_view_manager();
    auto popup = view_manager->get_popup();

    popup_set_icon(popup, 0, 30, &I_IrdaSendShort_128x34);
    popup_set_text(
        popup, "Get ready!\nPoint flipper at target.", 64, 16, AlignCenter, AlignCenter);

    popup_set_callback(popup, IrdaApp::popup_callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(IrdaAppViewManager::ViewType::Popup);
}

bool IrdaAppSceneLearnDoneAfter::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::PopupTimer) {
        app->switch_to_next_scene(IrdaApp::Scene::Remote);
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneLearnDoneAfter::on_exit(IrdaApp* app) {
}
