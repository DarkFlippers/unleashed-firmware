#include "u2f_view.h"
#include <gui/elements.h>

struct U2fView {
    View* view;
    U2fOkCallback callback;
    void* context;
};

typedef struct {
    U2fViewMsg display_msg;
} U2fModel;

static void u2f_view_draw_callback(Canvas* canvas, void* _model) {
    U2fModel* model = _model;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "U2F Demo");

    if(model->display_msg == U2fMsgRegister) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 0, 45, AlignLeft, AlignBottom, "Registration");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 0, 63, AlignLeft, AlignBottom, "Press [OK] to confirm");
    } else if(model->display_msg == U2fMsgAuth) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 0, 45, AlignLeft, AlignBottom, "Authentication");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 0, 63, AlignLeft, AlignBottom, "Press [OK] to confirm");
    } else if(model->display_msg == U2fMsgError) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignCenter, "U2F data missing");
    }
}

static bool u2f_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    U2fView* u2f = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            consumed = true;
            if(u2f->callback != NULL) u2f->callback(InputTypeShort, u2f->context);
        }
    }

    return consumed;
}

U2fView* u2f_view_alloc() {
    U2fView* u2f = furi_alloc(sizeof(U2fView));

    u2f->view = view_alloc();
    view_allocate_model(u2f->view, ViewModelTypeLocking, sizeof(U2fModel));
    view_set_context(u2f->view, u2f);
    view_set_draw_callback(u2f->view, u2f_view_draw_callback);
    view_set_input_callback(u2f->view, u2f_view_input_callback);

    return u2f;
}

void u2f_view_free(U2fView* u2f) {
    furi_assert(u2f);
    view_free(u2f->view);
    free(u2f);
}

View* u2f_view_get_view(U2fView* u2f) {
    furi_assert(u2f);
    return u2f->view;
}

void u2f_view_set_ok_callback(U2fView* u2f, U2fOkCallback callback, void* context) {
    furi_assert(u2f);
    furi_assert(callback);
    with_view_model(
        u2f->view, (U2fModel * model) {
            u2f->callback = callback;
            u2f->context = context;
            return false;
        });
}

void u2f_view_set_state(U2fView* u2f, U2fViewMsg msg) {
    with_view_model(
        u2f->view, (U2fModel * model) {
            model->display_msg = msg;
            return false;
        });
}
