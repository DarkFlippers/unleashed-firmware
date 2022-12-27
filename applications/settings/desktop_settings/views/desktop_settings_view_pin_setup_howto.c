#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <toolbox/version.h>
#include <assets_icons.h>
#include <dolphin/helpers/dolphin_state.h>
#include <dolphin/dolphin.h>

#include "desktop_settings_view_pin_setup_howto.h"

struct DesktopSettingsViewPinSetupHowto {
    View* view;
    DesktopSettingsViewPinSetupHowtoDoneCallback callback;
    void* context;
};

static void desktop_settings_view_pin_setup_howto_draw(Canvas* canvas, void* model) {
    furi_assert(canvas);
    furi_assert(model);

    canvas_draw_icon(canvas, 16, 18, &I_Pin_attention_dpad_29x29);
    elements_button_right(canvas, "Next");

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 0, AlignCenter, AlignTop, "Setting up PIN");

    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text(canvas, 58, 24, "Prepare to use\narrows as\nPIN symbols");
}

static bool desktop_settings_view_pin_setup_howto_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopSettingsViewPinSetupHowto* instance = context;
    bool consumed = false;

    if((event->key == InputKeyRight) && (event->type == InputTypeShort)) {
        instance->callback(instance->context);
        consumed = true;
    }

    return consumed;
}

void desktop_settings_view_pin_setup_howto_set_callback(
    DesktopSettingsViewPinSetupHowto* instance,
    DesktopSettingsViewPinSetupHowtoDoneCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

DesktopSettingsViewPinSetupHowto* desktop_settings_view_pin_setup_howto_alloc() {
    DesktopSettingsViewPinSetupHowto* view = malloc(sizeof(DesktopSettingsViewPinSetupHowto));
    view->view = view_alloc();
    view_allocate_model(view->view, ViewModelTypeLockFree, 1);
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, desktop_settings_view_pin_setup_howto_draw);
    view_set_input_callback(view->view, desktop_settings_view_pin_setup_howto_input);

    return view;
}

void desktop_settings_view_pin_setup_howto_free(DesktopSettingsViewPinSetupHowto* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* desktop_settings_view_pin_setup_howto_get_view(DesktopSettingsViewPinSetupHowto* instance) {
    furi_assert(instance);
    return instance->view;
}
