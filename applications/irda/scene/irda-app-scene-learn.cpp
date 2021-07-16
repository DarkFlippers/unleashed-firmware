#include "../irda-app.hpp"
#include "../irda-app-event.hpp"
#include <irda_worker.h>

static void signal_received_callback(void* context, IrdaWorkerSignal* received_signal) {
    furi_assert(context);
    furi_assert(received_signal);

    IrdaApp* app = static_cast<IrdaApp*>(context);

    if(irda_worker_signal_is_decoded(received_signal)) {
        IrdaAppSignal signal(irda_worker_get_decoded_message(received_signal));
        app->set_received_signal(signal);
    } else {
        const uint32_t* timings;
        size_t timings_cnt;
        irda_worker_get_raw_signal(received_signal, &timings, &timings_cnt);
        IrdaAppSignal signal(timings, timings_cnt);
        app->set_received_signal(signal);
    }

    irda_worker_set_received_signal_callback(app->get_irda_worker(), NULL);
    IrdaAppEvent event;
    event.type = IrdaAppEvent::Type::IrdaMessageReceived;
    auto view_manager = app->get_view_manager();
    view_manager->send_event(&event);
}

void IrdaAppSceneLearn::on_enter(IrdaApp* app) {
    auto view_manager = app->get_view_manager();
    auto popup = view_manager->get_popup();

    auto worker = app->get_irda_worker();
    irda_worker_set_context(worker, app);
    irda_worker_set_received_signal_callback(worker, signal_received_callback);
    irda_worker_start(worker);

    popup_set_icon(popup, 0, 32, &I_IrdaLearnShort_128x31);
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

    switch(event->type) {
    case IrdaAppEvent::Type::Tick:
        consumed = true;
        app->notify_red_blink();
        break;
    case IrdaAppEvent::Type::IrdaMessageReceived:
        app->notify_success();
        app->switch_to_next_scene_without_saving(IrdaApp::Scene::LearnSuccess);
        irda_worker_stop(app->get_irda_worker());
        break;
    case IrdaAppEvent::Type::Back:
        consumed = true;
        irda_worker_stop(app->get_irda_worker());
        app->switch_to_previous_scene();
        break;
    default:
        furi_assert(0);
    }

    return consumed;
}

void IrdaAppSceneLearn::on_exit(IrdaApp* app) {
}
