#include "../infrared_app.h"
#include "../infrared_app_event.h"
#include "infrared.h"
#include <infrared_worker.h>

static void signal_received_callback(void* context, InfraredWorkerSignal* received_signal) {
    furi_assert(context);
    furi_assert(received_signal);

    InfraredApp* app = static_cast<InfraredApp*>(context);

    if(infrared_worker_signal_is_decoded(received_signal)) {
        InfraredAppSignal signal(infrared_worker_get_decoded_signal(received_signal));
        app->set_received_signal(signal);
    } else {
        const uint32_t* timings;
        size_t timings_cnt;
        infrared_worker_get_raw_signal(received_signal, &timings, &timings_cnt);
        InfraredAppSignal signal(
            timings, timings_cnt, INFRARED_COMMON_CARRIER_FREQUENCY, INFRARED_COMMON_DUTY_CYCLE);
        app->set_received_signal(signal);
    }

    infrared_worker_rx_set_received_signal_callback(app->get_infrared_worker(), NULL, NULL);
    InfraredAppEvent event;
    event.type = InfraredAppEvent::Type::InfraredMessageReceived;
    auto view_manager = app->get_view_manager();
    view_manager->send_event(&event);
}

void InfraredAppSceneLearn::on_enter(InfraredApp* app) {
    auto view_manager = app->get_view_manager();
    auto popup = view_manager->get_popup();

    auto worker = app->get_infrared_worker();
    infrared_worker_rx_set_received_signal_callback(worker, signal_received_callback, app);
    infrared_worker_rx_start(worker);

    popup_set_icon(popup, 0, 32, &I_InfraredLearnShort_128x31);
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignCenter);
    popup_set_text(
        popup, "Point the remote at IR port\nand push the button", 5, 10, AlignLeft, AlignCenter);
    popup_set_callback(popup, NULL);

    view_manager->switch_to(InfraredAppViewManager::ViewId::Popup);
}

bool InfraredAppSceneLearn::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    switch(event->type) {
    case InfraredAppEvent::Type::Tick:
        consumed = true;
        app->notify_blink_read();
        break;
    case InfraredAppEvent::Type::InfraredMessageReceived:
        app->notify_success();
        app->switch_to_next_scene_without_saving(InfraredApp::Scene::LearnSuccess);
        break;
    case InfraredAppEvent::Type::Back:
        consumed = true;
        app->switch_to_previous_scene();
        break;
    default:
        furi_assert(0);
    }

    return consumed;
}

void InfraredAppSceneLearn::on_exit(InfraredApp* app) {
    infrared_worker_rx_stop(app->get_infrared_worker());
    auto view_manager = app->get_view_manager();
    auto popup = view_manager->get_popup();
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignCenter);
}
