#include "infrared_move_view.h"

#include <m-array.h>

#include <gui/canvas.h>
#include <gui/elements.h>

#include <toolbox/m_cstr_dup.h>

#define LIST_ITEMS    4U
#define LIST_LINE_H   13U
#define HEADER_H      12U
#define MOVE_X_OFFSET 5U

struct InfraredMoveView {
    View* view;
    InfraredMoveCallback callback;
    void* callback_context;
};

ARRAY_DEF(InfraredMoveViewItemArray, const char*, M_CSTR_DUP_OPLIST); //-V575

typedef struct {
    InfraredMoveViewItemArray_t labels;
    int32_t list_offset;
    int32_t current_idx;
    int32_t start_idx;
    bool is_moving;
} InfraredMoveViewModel;

static void infrared_move_view_draw_callback(Canvas* canvas, void* _model) {
    InfraredMoveViewModel* model = _model;

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(
        canvas, canvas_width(canvas) / 2, 0, AlignCenter, AlignTop, "Select a Button to Move");

    const size_t btn_number = InfraredMoveViewItemArray_size(model->labels);
    const bool show_scrollbar = btn_number > LIST_ITEMS;

    canvas_set_font(canvas, FontSecondary);

    for(uint32_t i = 0; i < MIN(btn_number, LIST_ITEMS); i++) {
        int32_t idx = CLAMP((uint32_t)(i + model->list_offset), btn_number, 0U);
        uint8_t x_offset = (model->is_moving && model->current_idx == idx) ? MOVE_X_OFFSET : 0;
        uint8_t y_offset = HEADER_H + i * LIST_LINE_H;
        uint8_t box_end_x = canvas_width(canvas) - (show_scrollbar ? 6 : 1);

        canvas_set_color(canvas, ColorBlack);
        if(model->current_idx == idx) {
            canvas_draw_box(canvas, x_offset, y_offset, box_end_x - x_offset, LIST_LINE_H);

            canvas_set_color(canvas, ColorWhite);
            canvas_draw_dot(canvas, x_offset, y_offset);
            canvas_draw_dot(canvas, x_offset + 1, y_offset);
            canvas_draw_dot(canvas, x_offset, y_offset + 1);
            canvas_draw_dot(canvas, x_offset, y_offset + LIST_LINE_H - 1);
            canvas_draw_dot(canvas, box_end_x - 1, y_offset);
            canvas_draw_dot(canvas, box_end_x - 1, y_offset + LIST_LINE_H - 1);
        }
        canvas_draw_str_aligned(
            canvas,
            x_offset + 3,
            y_offset + 3,
            AlignLeft,
            AlignTop,
            *InfraredMoveViewItemArray_cget(model->labels, idx));
    }

    if(show_scrollbar) {
        elements_scrollbar_pos(
            canvas,
            canvas_width(canvas),
            HEADER_H,
            canvas_height(canvas) - HEADER_H,
            model->current_idx,
            btn_number);
    }
}

static void update_list_offset(InfraredMoveViewModel* model) {
    const size_t btn_number = InfraredMoveViewItemArray_size(model->labels);
    const int32_t bounds = btn_number > (LIST_ITEMS - 1) ? 2 : btn_number;

    if((btn_number > (LIST_ITEMS - 1)) && (model->current_idx >= ((int32_t)btn_number - 1))) {
        model->list_offset = model->current_idx - (LIST_ITEMS - 1);
    } else if(model->list_offset < model->current_idx - bounds) {
        model->list_offset =
            CLAMP(model->current_idx - (int32_t)(LIST_ITEMS - 2), (int32_t)btn_number - bounds, 0);
    } else if(model->list_offset > model->current_idx - bounds) {
        model->list_offset = CLAMP(model->current_idx - 1, (int32_t)btn_number - bounds, 0);
    }
}

static bool infrared_move_view_input_callback(InputEvent* event, void* context) {
    InfraredMoveView* move_view = context;

    bool consumed = false;

    if((event->type == InputTypeShort || event->type == InputTypeRepeat) &&
       ((event->key == InputKeyUp) || (event->key == InputKeyDown))) {
        with_view_model(
            move_view->view,
            InfraredMoveViewModel * model,
            {
                const size_t btn_number = InfraredMoveViewItemArray_size(model->labels);
                const int32_t item_idx_prev = model->current_idx;

                if(event->key == InputKeyUp) {
                    if(model->current_idx <= 0) {
                        model->current_idx = btn_number;
                    }
                    model->current_idx--;

                } else if(event->key == InputKeyDown) {
                    model->current_idx++;
                    if(model->current_idx >= (int32_t)(btn_number)) {
                        model->current_idx = 0;
                    }
                }

                if(model->is_moving) {
                    InfraredMoveViewItemArray_swap_at(
                        model->labels, item_idx_prev, model->current_idx);
                }

                update_list_offset(model);
            },
            true);

        consumed = true;

    } else if((event->key == InputKeyOk) && (event->type == InputTypeShort)) {
        with_view_model(
            move_view->view,
            InfraredMoveViewModel * model,
            {
                if(!model->is_moving) {
                    model->start_idx = model->current_idx;
                } else if(move_view->callback) {
                    move_view->callback(
                        model->start_idx, model->current_idx, move_view->callback_context);
                }
                model->is_moving = !(model->is_moving);
            },
            true);

        consumed = true;

    } else if(event->key == InputKeyBack) {
        with_view_model(
            move_view->view,
            InfraredMoveViewModel * model,
            {
                if(model->is_moving && move_view->callback) {
                    move_view->callback(
                        model->start_idx, model->current_idx, move_view->callback_context);
                }
                model->is_moving = false;
            },
            false);

        // Not consuming, Back event is passed thru
    }

    return consumed;
}

void infrared_move_view_set_callback(
    InfraredMoveView* move_view,
    InfraredMoveCallback callback,
    void* context) {
    furi_assert(move_view);
    move_view->callback = callback;
    move_view->callback_context = context;
}

void infrared_move_view_add_item(InfraredMoveView* move_view, const char* label) {
    with_view_model(
        move_view->view,
        InfraredMoveViewModel * model,
        { InfraredMoveViewItemArray_push_back(model->labels, label); },
        true);
}

void infrared_move_view_reset(InfraredMoveView* move_view) {
    with_view_model(
        move_view->view,
        InfraredMoveViewModel * model,
        {
            InfraredMoveViewItemArray_reset(model->labels);
            model->list_offset = 0;
            model->start_idx = 0;
            model->current_idx = 0;
            model->is_moving = false;
        },
        false);
    move_view->callback_context = NULL;
}

InfraredMoveView* infrared_move_view_alloc(void) {
    InfraredMoveView* move_view = malloc(sizeof(InfraredMoveView));

    move_view->view = view_alloc();
    view_allocate_model(move_view->view, ViewModelTypeLocking, sizeof(InfraredMoveViewModel));
    view_set_draw_callback(move_view->view, infrared_move_view_draw_callback);
    view_set_input_callback(move_view->view, infrared_move_view_input_callback);
    view_set_context(move_view->view, move_view);

    with_view_model(
        move_view->view,
        InfraredMoveViewModel * model,
        { InfraredMoveViewItemArray_init(model->labels); },
        true);

    return move_view;
}

void infrared_move_view_free(InfraredMoveView* move_view) {
    with_view_model(
        move_view->view,
        InfraredMoveViewModel * model,
        { InfraredMoveViewItemArray_clear(model->labels); },
        true);

    view_free(move_view->view);
    free(move_view);
}

View* infrared_move_view_get_view(InfraredMoveView* move_view) {
    return move_view->view;
}
