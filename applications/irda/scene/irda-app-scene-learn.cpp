#include "../irda-app.hpp"

void IrdaAppSceneLearn::on_enter(IrdaApp* app) {
    auto view_manager = app->get_view_manager();
    auto receiver = app->get_receiver();
    auto event_queue = view_manager->get_event_queue();

    receiver->capture_once_start(event_queue);

    auto popup = view_manager->get_popup();

    popup_set_icon(popup, 0, 32, I_IrdaLearnShort_128x31);
    popup_set_text(
        popup, "Point the remote at IR port\nand push the button", 5, 10, AlignLeft, AlignCenter);
    popup_set_callback(popup, NULL);

    if(app->get_learn_new_remote()) {
        app->notify_double_vibro();
    }

    view_manager->switch_to(IrdaAppViewManager::ViewType::Popup);
}

bool IrdaAppSceneLearn::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::Tick) {
        consumed = true;
        app->notify_red_blink();
    }
    if(event->type == IrdaAppEvent::Type::IrdaMessageReceived) {
        app->notify_success();
        app->switch_to_next_scene_without_saving(IrdaApp::Scene::LearnSuccess);
    }

    return consumed;
}

void IrdaAppSceneLearn::on_exit(IrdaApp* app) {
}
