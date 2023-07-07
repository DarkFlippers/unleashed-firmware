#include "infos_view.h"

void draw_infos_view(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 3);
    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str_aligned(canvas, 3, 3, AlignLeft, AlignTop, "Wiring: ");
    canvas_draw_str_aligned(canvas, 43, 3, AlignLeft, AlignTop, "C0->SCL");
    canvas_draw_str_aligned(canvas, 43, 13, AlignLeft, AlignTop, "C1->SDA");
    canvas_draw_str_aligned(canvas, 43, 23, AlignLeft, AlignTop, "GND->GND");
    canvas_draw_icon(canvas, 15, 33, &I_Voltage_16x16);
    canvas_draw_str_aligned(canvas, 30, 38, AlignLeft, AlignTop, "Only use 3v3 levels");
}