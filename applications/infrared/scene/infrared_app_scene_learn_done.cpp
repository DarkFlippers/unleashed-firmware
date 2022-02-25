#include "../infrared_app.h"
#include <dolphin/dolphin.h>

void InfraredAppSceneLearnDone::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();

    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    DOLPHIN_DEED(DolphinDeedIrSave);

    if(app->get_learn_new_remote()) {
        popup_set_header(popup, "New remote\ncreated!", 0, 0, AlignLeft, AlignTop);
    } else {
        popup_set_header(popup, "Saved!", 5, 7, AlignLeft, AlignTop);
    }

    popup_set_callback(popup, InfraredApp::popup_callback);
    popup_set_context(popup, app);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_manager->switch_to(InfraredAppViewManager::ViewId::Popup);
}

bool InfraredAppSceneLearnDone::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::PopupTimer) {
        app->switch_to_next_scene(InfraredApp::Scene::Remote);
        consumed = true;
    }

    return consumed;
}

void InfraredAppSceneLearnDone::on_exit(InfraredApp* app) {
    app->set_learn_new_remote(false);
    InfraredAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, nullptr, 0, 0, AlignLeft, AlignTop);
}
