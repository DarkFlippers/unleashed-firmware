#include "power_off.h"
#include <furi.h>
#include <gui/elements.h>

struct PowerOff {
    View* view;
};

typedef struct {
    PowerOffResponse response;
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
    if(model->response == PowerOffResponseDefault) {
        snprintf(buff, sizeof(buff), "Charge me!\nOff in %lds!", model->time_left_sec);
        elements_multiline_text_aligned(canvas, 70, 23, AlignLeft, AlignTop, buff);

        elements_button_left(canvas, "Cancel");
        elements_button_center(canvas, "OK");
        elements_button_right(canvas, "Hide");
    } else {
        snprintf(buff, sizeof(buff), "Charge me!\nDont't forget!");
        elements_multiline_text_aligned(canvas, 70, 23, AlignLeft, AlignTop, buff);

        canvas_draw_str_aligned(canvas, 64, 60, AlignCenter, AlignBottom, "Hold a second...");
    }
}

static bool power_off_input_callback(InputEvent* event, void* context) {
    PowerOff* power_off = context;

    bool consumed = false;
    PowerOffModel* model = view_get_model(power_off->view);
    if(model->response == PowerOffResponseDefault && event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            model->response = PowerOffResponseOk;
            consumed = true;
        } else if(event->key == InputKeyLeft) {
            model->response = PowerOffResponseCancel;
            consumed = true;
        } else if(event->key == InputKeyRight) {
            model->response = PowerOffResponseHide;
            consumed = true;
        }
    }
    view_commit_model(power_off->view, consumed);

    return true;
}

PowerOff* power_off_alloc() {
    PowerOff* power_off = malloc(sizeof(PowerOff));

    power_off->view = view_alloc();
    view_allocate_model(power_off->view, ViewModelTypeLocking, sizeof(PowerOffModel));
    view_set_context(power_off->view, power_off);
    view_set_draw_callback(power_off->view, power_off_draw_callback);
    view_set_input_callback(power_off->view, power_off_input_callback);

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

PowerOffResponse power_off_get_response(PowerOff* power_off) {
    furi_assert(power_off);
    PowerOffResponse response;
    with_view_model(
        power_off->view, (PowerOffModel * model) {
            response = model->response;
            return false;
        });
    return response;
}
