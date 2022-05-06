#include "lfrfid_debug_app_scene_tune.h"
#include <furi_hal.h>

static void comparator_trigger_callback(bool level, void* comp_ctx) {
    UNUSED(comp_ctx);
    furi_hal_gpio_write(&gpio_ext_pa7, !level);
}

void LfRfidDebugAppSceneTune::on_enter(LfRfidDebugApp* app, bool /* need_restore */) {
    app->view_controller.switch_to<LfRfidViewTuneVM>();
    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeOutputPushPull);

    furi_hal_rfid_comp_set_callback(comparator_trigger_callback, this);
    furi_hal_rfid_comp_start();

    furi_hal_rfid_pins_read();
    furi_hal_rfid_tim_read(125000, 0.5);
    furi_hal_rfid_tim_read_start();
}

bool LfRfidDebugAppSceneTune::on_event(LfRfidDebugApp* app, LfRfidDebugApp::Event* /* event */) {
    bool consumed = false;

    LfRfidViewTuneVM* tune = app->view_controller;

    if(tune->is_dirty()) {
        furi_hal_rfid_set_read_period(tune->get_ARR());
        furi_hal_rfid_set_read_pulse(tune->get_CCR());
    }

    return consumed;
}

void LfRfidDebugAppSceneTune::on_exit(LfRfidDebugApp* /* app */) {
    furi_hal_rfid_comp_stop();
    furi_hal_rfid_comp_set_callback(NULL, NULL);

    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeAnalog);
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_tim_reset();
    furi_hal_rfid_pins_reset();
}
