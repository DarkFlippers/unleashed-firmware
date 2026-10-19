#include "lfrfid_view_read.h"
#include <gui/elements.h>
#include <assets_icons.h>

#define TEMP_STR_LEN 128

struct LfRfidReadView {
    View* view;
};

typedef struct {
    IconAnimation* icon;
    LfRfidReadViewMode read_mode;
} LfRfidReadViewModel;

static void lfrfid_view_read_draw_callback(Canvas* canvas, void* _model) {
    LfRfidReadViewModel* model = _model;
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_icon(canvas, 0, 8, &I_NFC_manual_60x50);

    canvas_set_font(canvas, FontPrimary);

    static const char* const steps[] = {"ASK", "PSK", "134kHz"};
    int8_t active = -1;
    if(model->read_mode == LfRfidReadAsk) {
        active = 0;
    } else if(model->read_mode == LfRfidReadPsk) {
        active = 1;
    } else if(model->read_mode == LfRfidReadAsk134) {
        active = 2;
    }

    if(active >= 0) {
        char header[16];
        snprintf(header, sizeof(header), "Reading %d/%zu", active + 1, COUNT_OF(steps));
        canvas_draw_str(canvas, 70, 14, header);

        for(size_t i = 0; i < COUNT_OF(steps); i++) {
            const uint8_t y = 26 + i * 11;
            if(i == (size_t)active) {
                canvas_set_font(canvas, FontPrimary);
                canvas_draw_icon(canvas, 70, y - 7, &I_ButtonRight_4x7);
                canvas_draw_icon_animation(
                    canvas, 77 + canvas_string_width(canvas, steps[i]) + 3, y - 8, model->icon);
            } else {
                canvas_set_font(canvas, FontSecondary);
            }
            canvas_draw_str(canvas, 77, y, steps[i]);
        }
    } else {
        canvas_draw_str(canvas, 72, 16, "Reading");

        if(model->read_mode == LfRfidReadAskOnly) {
            canvas_draw_str(canvas, 77, 35, "ASK");
        } else if(model->read_mode == LfRfidReadAsk134Only) {
            canvas_draw_str(canvas, 77, 35, "FDX-B");
        } else {
            canvas_draw_str(canvas, 77, 35, "PSK");
        }
        canvas_draw_icon_animation(canvas, 112, 27, model->icon);
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 61, 60, "Don't move card");
}

void lfrfid_view_read_enter(void* context) {
    LfRfidReadView* read_view = context;
    with_view_model(
        read_view->view, LfRfidReadViewModel * model, { icon_animation_start(model->icon); }, true);
}

void lfrfid_view_read_exit(void* context) {
    LfRfidReadView* read_view = context;
    with_view_model(
        read_view->view, LfRfidReadViewModel * model, { icon_animation_stop(model->icon); }, false);
}

LfRfidReadView* lfrfid_view_read_alloc(void) {
    LfRfidReadView* read_view = malloc(sizeof(LfRfidReadView));
    read_view->view = view_alloc();
    view_set_context(read_view->view, read_view);
    view_allocate_model(read_view->view, ViewModelTypeLocking, sizeof(LfRfidReadViewModel));

    with_view_model(
        read_view->view,
        LfRfidReadViewModel * model,
        {
            model->icon = icon_animation_alloc(&A_Round_loader_8x8);
            view_tie_icon_animation(read_view->view, model->icon);
        },
        false);

    view_set_draw_callback(read_view->view, lfrfid_view_read_draw_callback);
    view_set_enter_callback(read_view->view, lfrfid_view_read_enter);
    view_set_exit_callback(read_view->view, lfrfid_view_read_exit);

    return read_view;
}

void lfrfid_view_read_free(LfRfidReadView* read_view) {
    with_view_model(
        read_view->view, LfRfidReadViewModel * model, { icon_animation_free(model->icon); }, false);

    view_free(read_view->view);
    free(read_view);
}

View* lfrfid_view_read_get_view(LfRfidReadView* read_view) {
    return read_view->view;
}

void lfrfid_view_read_set_read_mode(LfRfidReadView* read_view, LfRfidReadViewMode mode) {
    with_view_model(
        read_view->view,
        LfRfidReadViewModel * model,
        {
            icon_animation_stop(model->icon);
            icon_animation_start(model->icon);
            model->read_mode = mode;
        },
        true);
}
