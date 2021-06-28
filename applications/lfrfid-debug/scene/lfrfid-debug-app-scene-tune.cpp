#include "lfrfid-debug-app-scene-tune.h"

void LfRfidDebugAppSceneTune::on_enter(LfRfidDebugApp* app, bool need_restore) {
    app->view_controller.switch_to<LfRfidViewTuneVM>();

    api_hal_rfid_pins_read();
    api_hal_rfid_tim_read(125000, 0.5);
    api_hal_rfid_tim_read_start();
}

bool LfRfidDebugAppSceneTune::on_event(LfRfidDebugApp* app, LfRfidDebugApp::Event* event) {
    bool consumed = false;

    LfRfidViewTuneVM* tune = app->view_controller;

    if(tune->is_dirty()) {
        api_hal_rfid_set_read_period(tune->get_ARR());
        api_hal_rfid_set_read_pulse(tune->get_CCR());
    }

    return consumed;
}

void LfRfidDebugAppSceneTune::on_exit(LfRfidDebugApp* app) {
    api_hal_rfid_tim_read_stop();
    api_hal_rfid_tim_reset();
    api_hal_rfid_pins_reset();
}