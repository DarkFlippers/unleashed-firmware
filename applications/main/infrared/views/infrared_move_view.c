#include "infrared_move_view.h"

#include <gui/canvas.h>
#include <gui/elements.h>

#include <stdlib.h>
#include <string.h>

#define LIST_ITEMS 4U
#define LIST_LINE_H 13U
#define HEADER_H 12U
#define MOVE_X_OFFSET 5U

struct InfraredMoveView {
    View* view;
    InfraredMoveCallback move_cb;
    void* cb_context;
};

typedef struct {
    const char** btn_names;
    uint32_t btn_number;
    int32_t list_offset;
    int32_t item_idx;
    bool is_moving;

    InfraredMoveGetItemCallback get_item_cb;
} InfraredMoveViewModel;

static void infrared_move_view_draw_callback(Canvas* canvas, void* _model) {
    InfraredMoveViewModel* model = _model;

    UNUSED(model);

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(
        canvas, canvas_width(canvas) / 2, 0, AlignCenter, AlignTop, "Select a button to move");

    bool show_scrollbar = model->btn_number > LIST_ITEMS;

    canvas_set_font(canvas, FontSecondary);

    for(uint32_t i = 0; i < MIN(model->btn_number, LIST_ITEMS); i++) {
        int32_t idx = CLAMP((uint32_t)(i + model->list_offset), model->btn_number, 0u);
        uint8_t x_offset = (model->is_moving && model->item_idx == idx) ? MOVE_X_OFFSET : 0;
        uint8_t y_offset = HEADER_H + i * LIST_LINE_H;
        uint8_t box_end_x = canvas_width(canvas) - (show_scrollbar ? 6 : 1);

        canvas_set_color(canvas, ColorBlack);
        if(model->item_idx == idx) {
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
            canvas, x_offset + 3, y_offset + 3, AlignLeft, AlignTop, model->btn_names[idx]);
    }

    if(show_scrollbar) {
        elements_scrollbar_pos(
            canvas,
            canvas_width(canvas),
            HEADER_H,
            canvas_height(canvas) - HEADER_H,
            model->item_idx,
            model->btn_number);
    }
}

static void update_list_offset(InfraredMoveViewModel* model) {
    int32_t bounds = model->btn_number > (LIST_ITEMS - 1) ? 2 : model->btn_number;

    if((model->btn_number > (LIST_ITEMS - 1)) &&
       (model->item_idx >= ((int32_t)model->btn_number - 1))) {
        model->list_offset = model->item_idx - (LIST_ITEMS - 1);
    } else if(model->list_offset < model->item_idx - bounds) {
        model->list_offset = CLAMP(
            model->item_idx - (int32_t)(LIST_ITEMS - 2), (int32_t)model->btn_number - bounds, 0);
    } else if(model->list_offset > model->item_idx - bounds) {
        model->list_offset = CLAMP(model->item_idx - 1, (int32_t)model->btn_number - bounds, 0);
    }
}

static bool infrared_move_view_input_callback(InputEvent* event, void* context) {
    InfraredMoveView* move_view = context;

    bool consumed = false;

    if(((event->type == InputTypeShort || event->type == InputTypeRepeat)) &&
       ((event->key == InputKeyUp) || (event->key == InputKeyDown))) {
        bool is_moving = false;
        uint32_t index_old = 0;
        uint32_t index_new = 0;
        with_view_model(
            move_view->view,
            InfraredMoveViewModel * model,
            {
                is_moving = model->is_moving;
                index_old = model->item_idx;
                if(event->key == InputKeyUp) {
                    if(model->item_idx <= 0) {
                        model->item_idx = model->btn_number;
                    }
                    model->item_idx--;
                } else if(event->key == InputKeyDown) {
                    model->item_idx++;
                    if(model->item_idx >= (int32_t)(model->btn_number)) {
                        model->item_idx = 0;
                    }
                }
                index_new = model->item_idx;
                update_list_offset(model);
            },
            !is_moving);
        if((is_moving) && (move_view->move_cb)) {
            move_view->move_cb(index_old, index_new, move_view->cb_context);
            infrared_move_view_list_update(move_view);
        }
        consumed = true;
    }

    if((event->key == InputKeyOk) && (event->type == InputTypeShort)) {
        with_view_model(
            move_view->view,
            InfraredMoveViewModel * model,
            { model->is_moving = !(model->is_moving); },
            true);
        consumed = true;
    }
    return consumed;
}

static void infrared_move_view_on_exit(void* context) {
    furi_assert(context);
    InfraredMoveView* move_view = context;

    with_view_model(
        move_view->view,
        InfraredMoveViewModel * model,
        {
            if(model->btn_names) {
                free(model->btn_names);
                model->btn_names = NULL;
            }
            model->btn_number = 0;
            model->get_item_cb = NULL;
        },
        false);
    move_view->cb_context = NULL;
}

void infrared_move_view_set_callback(InfraredMoveView* move_view, InfraredMoveCallback callback) {
    furi_assert(move_view);
    move_view->move_cb = callback;
}

void infrared_move_view_list_init(
    InfraredMoveView* move_view,
    uint32_t item_count,
    InfraredMoveGetItemCallback load_cb,
    void* context) {
    furi_assert(move_view);
    move_view->cb_context = context;
    with_view_model(
        move_view->view,
        InfraredMoveViewModel * model,
        {
            furi_assert(model->btn_names == NULL);
            model->btn_names = malloc(sizeof(char*) * item_count);
            model->btn_number = item_count;
            model->get_item_cb = load_cb;
        },
        false);
}

void infrared_move_view_list_update(InfraredMoveView* move_view) {
    furi_assert(move_view);
    with_view_model(
        move_view->view,
        InfraredMoveViewModel * model,
        {
            for(uint32_t i = 0; i < model->btn_number; i++) {
                if(!model->get_item_cb) break;
                model->btn_names[i] = model->get_item_cb(i, move_view->cb_context);
            }
        },
        true);
}

InfraredMoveView* infrared_move_view_alloc(void) {
    InfraredMoveView* move_view = malloc(sizeof(InfraredMoveView));
    move_view->view = view_alloc();
    view_allocate_model(move_view->view, ViewModelTypeLocking, sizeof(InfraredMoveViewModel));
    view_set_draw_callback(move_view->view, infrared_move_view_draw_callback);
    view_set_input_callback(move_view->view, infrared_move_view_input_callback);
    view_set_exit_callback(move_view->view, infrared_move_view_on_exit);
    view_set_context(move_view->view, move_view);
    return move_view;
}

void infrared_move_view_free(InfraredMoveView* move_view) {
    view_free(move_view->view);
    free(move_view);
}

View* infrared_move_view_get_view(InfraredMoveView* move_view) {
    return move_view->view;
}
