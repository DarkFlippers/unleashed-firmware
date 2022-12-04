#include "calibration.h"
#include "../tracking/main_loop.h"
#include "../air_mouse.h"

#include <furi.h>
#include <gui/elements.h>

struct Calibration {
    View* view;
    ViewDispatcher* view_dispatcher;
};

static void calibration_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Calibrating...");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 63, "Please wait");
}

void calibration_enter_callback(void* context) {
    furi_assert(context);
    Calibration* calibration = context;
    calibration_begin();
    view_dispatcher_send_custom_event(calibration->view_dispatcher, 0);
}

bool calibration_custom_callback(uint32_t event, void* context) {
    UNUSED(event);
    furi_assert(context);
    Calibration* calibration = context;

    if(calibration_step()) {
        view_dispatcher_switch_to_view(calibration->view_dispatcher, AirMouseViewSubmenu);
    } else {
        view_dispatcher_send_custom_event(calibration->view_dispatcher, 0);
    }

    return true;
}

void calibration_exit_callback(void* context) {
    furi_assert(context);
    calibration_end();
}

Calibration* calibration_alloc(ViewDispatcher* view_dispatcher) {
    Calibration* calibration = malloc(sizeof(Calibration));
    calibration->view = view_alloc();
    calibration->view_dispatcher = view_dispatcher;
    view_set_context(calibration->view, calibration);
    view_set_draw_callback(calibration->view, calibration_draw_callback);
    view_set_enter_callback(calibration->view, calibration_enter_callback);
    view_set_custom_callback(calibration->view, calibration_custom_callback);
    view_set_exit_callback(calibration->view, calibration_exit_callback);
    return calibration;
}

void calibration_free(Calibration* calibration) {
    furi_assert(calibration);
    view_free(calibration->view);
    free(calibration);
}

View* calibration_get_view(Calibration* calibration) {
    furi_assert(calibration);
    return calibration->view;
}
