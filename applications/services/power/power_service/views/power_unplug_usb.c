#include "power_unplug_usb.h"
#include <furi.h>
#include <gui/elements.h>
#include <assets_icons.h>

struct PowerUnplugUsb {
    View* view;
};

static void power_unplug_usb_draw_callback(Canvas* canvas, void* _model) {
    UNUSED(_model);

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, 0, 0, &I_Unplug_bg_top_128x14);
    canvas_draw_box(canvas, 0, 14, 128, (64 - 10 - 14));
    canvas_draw_icon(canvas, 0, (64 - 10), &I_Unplug_bg_bottom_128x10);

    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(
        canvas, 64, 32, AlignCenter, AlignCenter, "It's now safe to unplug\nthe USB cable");
}

PowerUnplugUsb* power_unplug_usb_alloc(void) {
    PowerUnplugUsb* power_unplug_usb = malloc(sizeof(PowerUnplugUsb));

    power_unplug_usb->view = view_alloc();
    view_set_context(power_unplug_usb->view, power_unplug_usb);
    view_set_draw_callback(power_unplug_usb->view, power_unplug_usb_draw_callback);
    view_set_input_callback(power_unplug_usb->view, NULL);

    return power_unplug_usb;
}

void power_unplug_usb_free(PowerUnplugUsb* power_unplug_usb) {
    furi_assert(power_unplug_usb);
    view_free(power_unplug_usb->view);
    free(power_unplug_usb);
}

View* power_unplug_usb_get_view(PowerUnplugUsb* power_unplug_usb) {
    furi_assert(power_unplug_usb);
    return power_unplug_usb->view;
}
