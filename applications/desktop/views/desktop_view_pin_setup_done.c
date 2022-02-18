#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <toolbox/version.h>
#include <assets_icons.h>
#include <dolphin/helpers/dolphin_state.h>
#include <dolphin/dolphin.h>

#include "../desktop_i.h"
#include "desktop_view_pin_setup_done.h"

struct DesktopViewPinSetupDone {
    View* view;
    DesktopViewPinSetupDoneDoneCallback callback;
    void* context;
};

static void desktop_view_pin_done_draw(Canvas* canvas, void* model) {
    furi_assert(canvas);
    furi_assert(model);

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(
        canvas, 64, 0, AlignCenter, AlignTop, "Prepare to use\narrows as\nPIN symbols");

    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text(canvas, 58, 24, "Prepare to use\narrows as\nPIN symbols");

    canvas_draw_icon(canvas, 16, 18, &I_Pin_attention_dpad_29x29);
    elements_button_right(canvas, "Next");
}

static bool desktop_view_pin_done_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopViewPinSetupDone* instance = context;
    bool consumed = false;

    if((event->key == InputKeyRight) && (event->type == InputTypeShort)) {
        instance->callback(instance->context);
        consumed = true;
    }

    return consumed;
}

void desktop_view_pin_done_set_callback(
    DesktopViewPinSetupDone* instance,
    DesktopViewPinSetupDoneDoneCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

DesktopViewPinSetupDone* desktop_view_pin_done_alloc() {
    DesktopViewPinSetupDone* view = malloc(sizeof(DesktopViewPinSetupDone));
    view->view = view_alloc();
    view_allocate_model(view->view, ViewModelTypeLockFree, 1);
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, desktop_view_pin_done_draw);
    view_set_input_callback(view->view, desktop_view_pin_done_input);

    return view;
}

void desktop_view_pin_done_free(DesktopViewPinSetupDone* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* desktop_view_pin_done_get_view(DesktopViewPinSetupDone* instance) {
    furi_assert(instance);
    return instance->view;
}
