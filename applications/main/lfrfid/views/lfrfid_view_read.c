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
    LfRfidReadViewState read_state;
} LfRfidReadViewModel;

static void lfrfid_view_read_draw_callback(Canvas* canvas, void* _model) {
    LfRfidReadViewModel* model = _model;
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_icon(canvas, 0, 8, &I_NFC_manual_60x50);

    canvas_set_font(canvas, FontPrimary);

    if(model->read_mode == LfRfidReadAsk) {
        canvas_draw_str(canvas, 70, 8, "Reading 1/3");

        canvas_draw_str(canvas, 77, 20, "ASK");
        canvas_draw_icon(canvas, 70, 13, &I_ButtonRight_4x7);
        canvas_draw_icon_animation(canvas, 112, 12, model->icon);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 77, 33, "PSK");
        canvas_draw_str(canvas, 77, 46, "RTF");
    } else if(model->read_mode == LfRfidReadPsk) {
        canvas_draw_str(canvas, 70, 8, "Reading 2/3");

        canvas_draw_str(canvas, 77, 33, "PSK");
        canvas_draw_icon(canvas, 70, 26, &I_ButtonRight_4x7);
        canvas_draw_icon_animation(canvas, 112, 25, model->icon);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 77, 20, "ASK");
        canvas_draw_str(canvas, 77, 46, "RTF");
    } else if(model->read_mode == LfRfidReadHitag) {
        if(model->read_state == LfRfidReadScanning) {
            canvas_draw_str(canvas, 70, 8, "Reading 3/3");

            canvas_draw_str(canvas, 77, 46, "RTF");
            canvas_draw_icon(canvas, 70, 39, &I_ButtonRight_4x7);
            canvas_draw_icon_animation(canvas, 112, 38, model->icon);

            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 77, 20, "ASK");
            canvas_draw_str(canvas, 77, 33, "PSK");
        } else if(model->read_state == LfRfidReadTagDetected) { //TODO switch to other scene?
            canvas_draw_str(canvas, 65, 8, "Hitag1 found");

            canvas_set_font(canvas, FontSecondary);
            //canvas_draw_str(canvas, 70, 20, "## ## ## ##");	//TODO get tag SN from hitag worker
            canvas_draw_str(canvas, 70, 33, "Reading data");
            //canvas_draw_str(canvas, 70, 46, "Page: X/64");	//TODO get current page index from hitag worker
        }
    } else if(model->read_mode == LfRfidReadAskOnly) {
        canvas_draw_str(canvas, 72, 16, "Reading");
        canvas_draw_str(canvas, 77, 35, "ASK");
        canvas_draw_icon_animation(canvas, 112, 27, model->icon);
    } else if(model->read_mode == LfRfidReadPskOnly) {
        canvas_draw_str(canvas, 72, 16, "Reading");
        canvas_draw_str(canvas, 77, 35, "PSK");
        canvas_draw_icon_animation(canvas, 112, 27, model->icon);
    } else if(model->read_mode == LfRfidReadRtfOnly) {
        if(model->read_state == LfRfidReadScanning) {
            canvas_draw_str(canvas, 72, 16, "Reading");
            canvas_draw_str(canvas, 77, 35, "RTF");
            canvas_draw_icon_animation(canvas, 112, 27, model->icon);
        } else if(model->read_state == LfRfidReadTagDetected) { //TODO switch to other scene?
            canvas_draw_str(canvas, 65, 8, "Hitag1 found");

            canvas_set_font(canvas, FontSecondary);
            //canvas_draw_str(canvas, 70, 20, "## ## ## ##");	//TODO get tag SN from hitag worker
            canvas_draw_str(canvas, 70, 33, "Reading data");
            //canvas_draw_str(canvas, 70, 46, "Page: X/64");	//TODO get current page index from hitag worker
        }
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

LfRfidReadView* lfrfid_view_read_alloc() {
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

void lfrfid_view_read_set_read_state(LfRfidReadView* read_view, LfRfidReadViewState state) {
    with_view_model(
        read_view->view, LfRfidReadViewModel * model, { model->read_state = state; }, true);
}