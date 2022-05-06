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

    canvas_draw_icon(canvas, 8, 14, &I_Drive_112x35);
    canvas_set_font(canvas, FontSecondary);

    if(model->display_msg == U2fMsgNotConnected) {
        canvas_draw_icon(canvas, 22, 15, &I_Connect_me_62x31);
        canvas_draw_str_aligned(
            canvas, 128 / 2, 3, AlignCenter, AlignTop, "Connect me to computer");
    } else if(model->display_msg == U2fMsgIdle) {
        canvas_draw_icon(canvas, 22, 15, &I_Connected_62x31);
        canvas_draw_str_aligned(canvas, 128 / 2, 3, AlignCenter, AlignTop, "Connected!");
    } else if(model->display_msg == U2fMsgRegister) {
        elements_button_center(canvas, "OK");
        canvas_draw_icon(canvas, 22, 15, &I_Auth_62x31);
        canvas_draw_str_aligned(canvas, 128 / 2, 3, AlignCenter, AlignTop, "Press OK to register");
    } else if(model->display_msg == U2fMsgAuth) {
        elements_button_center(canvas, "OK");
        canvas_draw_icon(canvas, 22, 15, &I_Auth_62x31);
        canvas_draw_str_aligned(
            canvas, 128 / 2, 3, AlignCenter, AlignTop, "Press OK to authenticate");
    } else if(model->display_msg == U2fMsgSuccess) {
        canvas_draw_icon(canvas, 22, 15, &I_Connected_62x31);
        canvas_draw_str_aligned(
            canvas, 128 / 2, 3, AlignCenter, AlignTop, "Authentication successfull!");
    } else if(model->display_msg == U2fMsgError) {
        canvas_draw_icon(canvas, 22, 15, &I_Error_62x31);
        canvas_draw_str_aligned(canvas, 128 / 2, 3, AlignCenter, AlignTop, "Ceritficate error");
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
    U2fView* u2f = malloc(sizeof(U2fView));

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
            UNUSED(model);
            u2f->callback = callback;
            u2f->context = context;
            return false;
        });
}

void u2f_view_set_state(U2fView* u2f, U2fViewMsg msg) {
    with_view_model(
        u2f->view, (U2fModel * model) {
            model->display_msg = msg;
            return true;
        });
}
