#include <furi.h>
#include "../desktop_i.h"
#include <furi-hal.h>
#include <furi-hal-version.h>

#include "desktop_hw_mismatch.h"

void desktop_hw_mismatch_set_callback(
    DesktopHwMismatchView* main_view,
    DesktopHwMismatchViewCallback callback,
    void* context) {
    furi_assert(main_view);
    furi_assert(callback);
    main_view->callback = callback;
    main_view->context = context;
}

void desktop_hw_mismatch_render(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 15, "!!!! HW Mismatch !!!!");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, 64, "HW target: F%d", furi_hal_version_get_hw_target());
    canvas_draw_str(canvas, 5, 27, buffer);
    canvas_draw_str(canvas, 5, 38, "FW target: " TARGET);
}

View* desktop_hw_mismatch_get_view(DesktopHwMismatchView* hw_mismatch_view) {
    furi_assert(hw_mismatch_view);
    return hw_mismatch_view->view;
}

bool desktop_hw_mismatch_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopHwMismatchView* hw_mismatch_view = context;

    if(event->type == InputTypeShort) {
        hw_mismatch_view->callback(DesktopHwMismatchEventExit, hw_mismatch_view->context);
    }

    return true;
}

DesktopHwMismatchView* desktop_hw_mismatch_alloc() {
    DesktopHwMismatchView* hw_mismatch_view = furi_alloc(sizeof(DesktopHwMismatchView));
    hw_mismatch_view->view = view_alloc();
    view_allocate_model(
        hw_mismatch_view->view, ViewModelTypeLocking, sizeof(DesktopHwMismatchViewModel));
    view_set_context(hw_mismatch_view->view, hw_mismatch_view);
    view_set_draw_callback(hw_mismatch_view->view, (ViewDrawCallback)desktop_hw_mismatch_render);
    view_set_input_callback(hw_mismatch_view->view, desktop_hw_mismatch_input);

    return hw_mismatch_view;
}

void desktop_hw_mismatch_free(DesktopHwMismatchView* hw_mismatch_view) {
    furi_assert(hw_mismatch_view);

    view_free(hw_mismatch_view->view);
    free(hw_mismatch_view);
}
