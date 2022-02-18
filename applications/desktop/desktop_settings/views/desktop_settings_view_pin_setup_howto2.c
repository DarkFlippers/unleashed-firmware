#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <toolbox/version.h>
#include <assets_icons.h>
#include <dolphin/helpers/dolphin_state.h>
#include <dolphin/dolphin.h>

#include "desktop_settings_view_pin_setup_howto2.h"

struct DesktopSettingsViewPinSetupHowto2 {
    View* view;
    DesktopSettingsViewPinSetupHowto2Callback cancel_callback;
    DesktopSettingsViewPinSetupHowto2Callback ok_callback;
    void* context;
};

static void desktop_settings_view_pin_setup_howto2_draw(Canvas* canvas, void* model) {
    furi_assert(canvas);
    furi_assert(model);

    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas,
        64,
        24,
        AlignCenter,
        AlignCenter,
        "Forgotten PIN can only be\n"
        "reset with entire device.\n"
        "Read docs How to reset PIN.");

    elements_button_right(canvas, "OK");
    elements_button_left(canvas, "Cancel");
}

static bool desktop_settings_view_pin_setup_howto2_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopSettingsViewPinSetupHowto2* instance = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight) {
            instance->ok_callback(instance->context);
            consumed = true;
        } else if(event->key == InputKeyLeft) {
            instance->cancel_callback(instance->context);
            consumed = true;
        }
    }

    return consumed;
}

void desktop_settings_view_pin_setup_howto2_set_context(
    DesktopSettingsViewPinSetupHowto2* instance,
    void* context) {
    furi_assert(instance);
    instance->context = context;
}

void desktop_settings_view_pin_setup_howto2_set_cancel_callback(
    DesktopSettingsViewPinSetupHowto2* instance,
    DesktopSettingsViewPinSetupHowto2Callback callback) {
    furi_assert(instance);
    instance->cancel_callback = callback;
}

void desktop_settings_view_pin_setup_howto2_set_ok_callback(
    DesktopSettingsViewPinSetupHowto2* instance,
    DesktopSettingsViewPinSetupHowto2Callback callback) {
    furi_assert(instance);
    instance->ok_callback = callback;
}

DesktopSettingsViewPinSetupHowto2* desktop_settings_view_pin_setup_howto2_alloc() {
    DesktopSettingsViewPinSetupHowto2* view = malloc(sizeof(DesktopSettingsViewPinSetupHowto2));
    view->view = view_alloc();
    view_allocate_model(view->view, ViewModelTypeLockFree, 1);
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, desktop_settings_view_pin_setup_howto2_draw);
    view_set_input_callback(view->view, desktop_settings_view_pin_setup_howto2_input);

    return view;
}

void desktop_settings_view_pin_setup_howto2_free(DesktopSettingsViewPinSetupHowto2* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* desktop_settings_view_pin_setup_howto2_get_view(DesktopSettingsViewPinSetupHowto2* instance) {
    furi_assert(instance);
    return instance->view;
}
