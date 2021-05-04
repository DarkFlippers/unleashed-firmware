#include "lf-rfid-scene-tune.h"
#include "../lf-rfid-app.h"
#include "../lf-rfid-view-manager.h"
#include "../lf-rfid-event.h"
#include <callback-connector.h>

void LfrfidSceneTune::on_enter(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();
    //LfRfidViewTune* tune = view_manager->get_tune();

    view_manager->switch_to(LfrfidAppViewManager::ViewType::Tune);

    reader.start(RfidReader::Type::Indala);
}

bool LfrfidSceneTune::on_event(LfrfidApp* app, LfrfidEvent* event) {
    bool consumed = false;

    if(event->type == LfrfidEvent::Type::Tick) {
        LfRfidViewTune* tune = app->get_view_manager()->get_tune();

        if(tune->is_dirty()) {
            LFRFID_TIM.Instance->ARR = tune->get_ARR();
            LFRFID_TIM.Instance->CCR1 = tune->get_CCR();
        }
    }

    return consumed;
}

void LfrfidSceneTune::on_exit(LfrfidApp* app) {
    //LfRfidViewTune* tune = app->get_view_manager()->get_tune();

    reader.stop();
}