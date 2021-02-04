#include "dialog_ex.h"
#include <gui/elements.h>
#include <furi.h>

struct DialogEx {
    View* view;
    void* context;
    DialogExResultCallback callback;
};

typedef struct {
    const char* text;
    uint8_t x;
    uint8_t y;
    Align horizontal;
    Align vertical;
} TextElement;

typedef struct {
    int8_t x;
    int8_t y;
    IconName name;
} IconElement;

typedef struct {
    TextElement header;
    TextElement text;
    IconElement icon;

    const char* left_text;
    const char* center_text;
    const char* right_text;
} DialogExModel;

static void dialog_ex_view_draw_callback(Canvas* canvas, void* _model) {
    DialogExModel* model = _model;

    // Prepare canvas
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // TODO other criteria for the draw
    if(model->icon.x >= 0 && model->icon.y >= 0) {
        canvas_draw_icon_name(canvas, model->icon.x, model->icon.y, model->icon.name);
    }

    // Draw header
    if(model->header.text != NULL) {
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(
            canvas,
            model->header.x,
            model->header.y,
            model->header.horizontal,
            model->header.vertical,
            model->header.text);
    }

    // Draw text
    if(model->text.text != NULL) {
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(
            canvas,
            model->text.x,
            model->text.y,
            model->text.horizontal,
            model->text.vertical,
            model->text.text);
    }

    // Draw buttons
    if(model->left_text != NULL) {
        elements_button_left(canvas, model->left_text);
    }

    if(model->center_text != NULL) {
        elements_button_center(canvas, model->center_text);
    }

    if(model->right_text != NULL) {
        elements_button_right(canvas, model->right_text);
    }
}

static bool dialog_ex_view_input_callback(InputEvent* event, void* context) {
    DialogEx* dialog_ex = context;
    bool consumed = false;
    const char* left_text = NULL;
    const char* center_text = NULL;
    const char* right_text = NULL;

    with_view_model(
        dialog_ex->view, (DialogExModel * model) {
            left_text = model->left_text;
            center_text = model->center_text;
            right_text = model->right_text;
        });

    // Process key presses only
    if(event->state && dialog_ex->callback) {
        if(event->input == InputLeft && left_text != NULL) {
            dialog_ex->callback(DialogExResultLeft, dialog_ex->context);
            consumed = true;
        } else if(event->input == InputOk && center_text != NULL) {
            dialog_ex->callback(DialogExResultCenter, dialog_ex->context);
            consumed = true;
        } else if(event->input == InputRight && right_text != NULL) {
            dialog_ex->callback(DialogExResultRight, dialog_ex->context);
            consumed = true;
        }
    }

    return consumed;
}

DialogEx* dialog_ex_alloc() {
    DialogEx* dialog_ex = furi_alloc(sizeof(DialogEx));
    dialog_ex->view = view_alloc();
    view_set_context(dialog_ex->view, dialog_ex);
    view_allocate_model(dialog_ex->view, ViewModelTypeLockFree, sizeof(DialogExModel));
    view_set_draw_callback(dialog_ex->view, dialog_ex_view_draw_callback);
    view_set_input_callback(dialog_ex->view, dialog_ex_view_input_callback);
    with_view_model(
        dialog_ex->view, (DialogExModel * model) {
            model->header.text = NULL;
            model->header.x = 0;
            model->header.y = 0;
            model->header.horizontal = AlignLeft;
            model->header.vertical = AlignBottom;

            model->text.text = NULL;
            model->text.x = 0;
            model->text.y = 0;
            model->text.horizontal = AlignLeft;
            model->text.vertical = AlignBottom;

            // TODO other criteria for the draw
            model->icon.x = -1;
            model->icon.y = -1;
            model->icon.name = I_ButtonCenter_7x7;

            model->left_text = NULL;
            model->center_text = NULL;
            model->right_text = NULL;
        });
    return dialog_ex;
}

void dialog_ex_free(DialogEx* dialog_ex) {
    furi_assert(dialog_ex);
    view_free(dialog_ex->view);
    free(dialog_ex);
}

View* dialog_ex_get_view(DialogEx* dialog_ex) {
    furi_assert(dialog_ex);
    return dialog_ex->view;
}

void dialog_ex_set_result_callback(DialogEx* dialog_ex, DialogExResultCallback callback) {
    furi_assert(dialog_ex);
    dialog_ex->callback = callback;
}

void dialog_ex_set_context(DialogEx* dialog_ex, void* context) {
    furi_assert(dialog_ex);
    dialog_ex->context = context;
}

void dialog_ex_set_header(
    DialogEx* dialog_ex,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    furi_assert(dialog_ex);
    with_view_model(
        dialog_ex->view, (DialogExModel * model) {
            model->header.text = text;
            model->header.x = x;
            model->header.y = y;
            model->header.horizontal = horizontal;
            model->header.vertical = vertical;
        });
}

void dialog_ex_set_text(
    DialogEx* dialog_ex,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    furi_assert(dialog_ex);
    with_view_model(
        dialog_ex->view, (DialogExModel * model) {
            model->text.text = text;
            model->text.x = x;
            model->text.y = y;
            model->text.horizontal = horizontal;
            model->text.vertical = vertical;
        });
}

void dialog_ex_set_icon(DialogEx* dialog_ex, int8_t x, int8_t y, IconName name) {
    furi_assert(dialog_ex);
    with_view_model(
        dialog_ex->view, (DialogExModel * model) {
            model->icon.x = x;
            model->icon.y = y;
            model->icon.name = name;
        });
}

void dialog_ex_set_left_button_text(DialogEx* dialog_ex, const char* text) {
    furi_assert(dialog_ex);
    with_view_model(
        dialog_ex->view, (DialogExModel * model) { model->left_text = text; });
}

void dialog_ex_set_center_button_text(DialogEx* dialog_ex, const char* text) {
    furi_assert(dialog_ex);
    with_view_model(
        dialog_ex->view, (DialogExModel * model) { model->center_text = text; });
}

void dialog_ex_set_right_button_text(DialogEx* dialog_ex, const char* text) {
    furi_assert(dialog_ex);
    with_view_model(
        dialog_ex->view, (DialogExModel * model) { model->right_text = text; });
}
