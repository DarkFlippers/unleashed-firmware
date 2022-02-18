#include "power_off.h"
#include <furi.h>
#include <gui/elements.h>

struct PowerOff {
    View* view;
};

typedef struct {
    uint32_t time_left_sec;
} PowerOffModel;

static void power_off_draw_callback(Canvas* canvas, void* _model) {
    furi_assert(_model);
    PowerOffModel* model = _model;
    char buff[32];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 1, AlignCenter, AlignTop, "Battery low!");
    canvas_draw_icon(canvas, 0, 18, &I_BatteryBody_52x28);
    canvas_draw_icon(canvas, 16, 25, &I_FaceNopower_29x14);
    elements_bubble(canvas, 54, 17, 70, 30);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas, 70, 23, AlignLeft, AlignTop, "Connect me\n to charger.");
    snprintf(buff, sizeof(buff), "Poweroff in %lds.", model->time_left_sec);
    canvas_draw_str_aligned(canvas, 64, 60, AlignCenter, AlignBottom, buff);
}

PowerOff* power_off_alloc() {
    PowerOff* power_off = malloc(sizeof(PowerOff));
    power_off->view = view_alloc();
    view_allocate_model(power_off->view, ViewModelTypeLocking, sizeof(PowerOffModel));
    view_set_draw_callback(power_off->view, power_off_draw_callback);
    return power_off;
}

void power_off_free(PowerOff* power_off) {
    furi_assert(power_off);
    view_free(power_off->view);
    free(power_off);
}

View* power_off_get_view(PowerOff* power_off) {
    furi_assert(power_off);
    return power_off->view;
}

void power_off_set_time_left(PowerOff* power_off, uint8_t time_left) {
    furi_assert(power_off);
    with_view_model(
        power_off->view, (PowerOffModel * model) {
            model->time_left_sec = time_left;
            return true;
        });
}
