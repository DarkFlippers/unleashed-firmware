#include "../lfrfid_debug_i.h"
#include <furi_hal.h>

static void comparator_trigger_callback(bool level, void* comp_ctx) {
    UNUSED(comp_ctx);
    furi_hal_gpio_write(&gpio_ext_pa7, !level);
}

void lfrfid_debug_view_tune_callback(void* context) {
    LfRfidDebug* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0xBA);
}

void lfrfid_debug_scene_tune_on_enter(void* context) {
    LfRfidDebug* app = context;

    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeOutputPushPull);

    furi_hal_rfid_comp_set_callback(comparator_trigger_callback, app);
    furi_hal_rfid_comp_start();

    furi_hal_rfid_tim_read_start(125000, 0.5);

    lfrfid_debug_view_tune_set_callback(app->tune_view, lfrfid_debug_view_tune_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidDebugViewTune);
}

bool lfrfid_debug_scene_tune_on_event(void* context, SceneManagerEvent event) {
    UNUSED(event);

    LfRfidDebug* app = context;
    bool consumed = false;

    if(lfrfid_debug_view_tune_is_dirty(app->tune_view)) {
        furi_hal_rfid_set_read_period(lfrfid_debug_view_tune_get_arr(app->tune_view));
        furi_hal_rfid_set_read_pulse(lfrfid_debug_view_tune_get_ccr(app->tune_view));
    }

    return consumed;
}

void lfrfid_debug_scene_tune_on_exit(void* context) {
    UNUSED(context);

    furi_hal_rfid_comp_stop();
    furi_hal_rfid_comp_set_callback(NULL, NULL);

    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeAnalog);
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}
