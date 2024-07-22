#include "lfrfid_debug_view_tune.h"
#include <gui/elements.h>

#define TEMP_STR_LEN 128

struct LfRfidTuneView {
    View* view;
};

typedef struct {
    bool dirty;
    bool fine;
    uint32_t ARR;
    uint32_t CCR;
    int pos;
    void (*update_callback)(void* context);
    void* update_context;
} LfRfidTuneViewModel;

static void lfrfid_debug_view_tune_draw_callback(Canvas* canvas, void* _model) {
    LfRfidTuneViewModel* model = _model;
    canvas_set_color(canvas, ColorBlack);

    if(model->fine) {
        canvas_draw_box(
            canvas,
            128 - canvas_string_width(canvas, "Fine") - 4,
            0,
            canvas_string_width(canvas, "Fine") + 4,
            canvas_current_font_height(canvas) + 1);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str_aligned(canvas, 128 - 2, 2, AlignRight, AlignTop, "Fine");
    canvas_set_color(canvas, ColorBlack);

    char buffer[TEMP_STR_LEN + 1];
    double freq = ((double)SystemCoreClock / (model->ARR + 1));
    double duty = (double)((model->CCR + 1) * 100) / (model->ARR + 1);
    snprintf(
        buffer,
        TEMP_STR_LEN,
        "%sARR: %lu\n"
        "freq = %.4f\n"
        "%sCCR: %lu\n"
        "duty = %.4f",
        model->pos == 0 ? ">" : "",
        model->ARR,
        freq,
        model->pos == 1 ? ">" : "",
        model->CCR,
        duty);
    elements_multiline_text_aligned(canvas, 2, 2, AlignLeft, AlignTop, buffer);
}

static void lfrfid_debug_view_tune_button_up(LfRfidTuneView* tune_view) {
    with_view_model(
        tune_view->view,
        LfRfidTuneViewModel * model,
        {
            if(model->pos > 0) model->pos--;
        },
        true);
}

static void lfrfid_debug_view_tune_button_down(LfRfidTuneView* tune_view) {
    with_view_model(
        tune_view->view,
        LfRfidTuneViewModel * model,
        {
            if(model->pos < 1) model->pos++;
        },
        true);
}

static void lfrfid_debug_view_tune_button_left(LfRfidTuneView* tune_view) {
    with_view_model(
        tune_view->view,
        LfRfidTuneViewModel * model,
        {
            if(model->pos == 0) {
                if(model->fine) {
                    model->ARR -= 1;
                } else {
                    model->ARR -= 10;
                }
            } else if(model->pos == 1) {
                if(model->fine) {
                    model->CCR -= 1;
                } else {
                    model->CCR -= 10;
                }
            }

            model->dirty = true;
        },
        true);
}

static void lfrfid_debug_view_tune_button_right(LfRfidTuneView* tune_view) {
    with_view_model(
        tune_view->view,
        LfRfidTuneViewModel * model,
        {
            if(model->pos == 0) {
                if(model->fine) {
                    model->ARR += 1;
                } else {
                    model->ARR += 10;
                }
            } else if(model->pos == 1) {
                if(model->fine) {
                    model->CCR += 1;
                } else {
                    model->CCR += 10;
                }
            }

            model->dirty = true;
        },
        true);
}

static void lfrfid_debug_view_tune_button_ok(LfRfidTuneView* tune_view) {
    with_view_model(
        tune_view->view, LfRfidTuneViewModel * model, { model->fine = !model->fine; }, true);
}

static bool lfrfid_debug_view_tune_input_callback(InputEvent* event, void* context) {
    LfRfidTuneView* tune_view = context;
    bool consumed = false;

    // Process key presses only
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        consumed = true;

        switch(event->key) {
        case InputKeyLeft:
            lfrfid_debug_view_tune_button_left(tune_view);
            break;
        case InputKeyRight:
            lfrfid_debug_view_tune_button_right(tune_view);
            break;
        case InputKeyUp:
            lfrfid_debug_view_tune_button_up(tune_view);
            break;
        case InputKeyDown:
            lfrfid_debug_view_tune_button_down(tune_view);
            break;
        case InputKeyOk:
            lfrfid_debug_view_tune_button_ok(tune_view);
            break;
        default:
            consumed = false;
            break;
        }

        if(event->key == InputKeyLeft || event->key == InputKeyRight) {
            with_view_model(
                tune_view->view,
                LfRfidTuneViewModel * model,
                {
                    if(model->update_callback) {
                        model->update_callback(model->update_context);
                    }
                },
                false);
        }
    }

    return consumed;
}

LfRfidTuneView* lfrfid_debug_view_tune_alloc(void) {
    LfRfidTuneView* tune_view = malloc(sizeof(LfRfidTuneView));
    tune_view->view = view_alloc();
    view_set_context(tune_view->view, tune_view);
    view_allocate_model(tune_view->view, ViewModelTypeLocking, sizeof(LfRfidTuneViewModel));
    lfrfid_debug_view_tune_clean(tune_view);
    view_set_draw_callback(tune_view->view, lfrfid_debug_view_tune_draw_callback);
    view_set_input_callback(tune_view->view, lfrfid_debug_view_tune_input_callback);

    return tune_view;
}

void lfrfid_debug_view_tune_free(LfRfidTuneView* tune_view) {
    view_free(tune_view->view);
    free(tune_view);
}

View* lfrfid_debug_view_tune_get_view(LfRfidTuneView* tune_view) {
    return tune_view->view;
}

void lfrfid_debug_view_tune_clean(LfRfidTuneView* tune_view) {
    with_view_model(
        tune_view->view,
        LfRfidTuneViewModel * model,
        {
            model->dirty = true;
            model->fine = false;
            model->ARR = 511;
            model->CCR = 255;
            model->pos = 0;
            model->update_callback = NULL;
            model->update_context = NULL;
        },
        true);
}

bool lfrfid_debug_view_tune_is_dirty(LfRfidTuneView* tune_view) {
    bool result = false;
    with_view_model(
        tune_view->view,
        LfRfidTuneViewModel * model,
        {
            result = model->dirty;
            model->dirty = false;
        },
        false);

    return result;
}

uint32_t lfrfid_debug_view_tune_get_arr(LfRfidTuneView* tune_view) {
    uint32_t result = false;
    with_view_model(tune_view->view, LfRfidTuneViewModel * model, { result = model->ARR; }, false);

    return result;
}

uint32_t lfrfid_debug_view_tune_get_ccr(LfRfidTuneView* tune_view) {
    uint32_t result = false;
    with_view_model(tune_view->view, LfRfidTuneViewModel * model, { result = model->CCR; }, false);

    return result;
}

void lfrfid_debug_view_tune_set_callback(
    LfRfidTuneView* tune_view,
    void (*callback)(void* context),
    void* context) {
    with_view_model(
        tune_view->view,
        LfRfidTuneViewModel * model,
        {
            model->update_callback = callback;
            model->update_context = context;
        },
        false);
}
