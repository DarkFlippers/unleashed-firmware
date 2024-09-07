#include "dialog_ex.h"
#include <gui/elements.h>
#include <furi.h>

struct DialogEx {
    View* view;
    void* context;
    DialogExResultCallback callback;
    bool enable_extended_events;
};

typedef struct {
    FuriString* text;
    uint8_t x;
    uint8_t y;
    Align horizontal;
    Align vertical;
} TextElement;

typedef struct {
    int8_t x;
    int8_t y;
    const Icon* icon;
} IconElement;

typedef struct {
    TextElement header;
    TextElement text;
    IconElement icon;

    FuriString* left_text;
    FuriString* center_text;
    FuriString* right_text;
} DialogExModel;

static void dialog_ex_view_draw_callback(Canvas* canvas, void* _model) {
    DialogExModel* model = _model;

    // Prepare canvas
    canvas_set_color(canvas, ColorBlack);

    if(model->icon.icon != NULL) {
        canvas_draw_icon(canvas, model->icon.x, model->icon.y, model->icon.icon);
    }

    // Draw header
    canvas_set_font(canvas, FontPrimary);
    if(furi_string_size(model->header.text)) {
        elements_multiline_text_aligned(
            canvas,
            model->header.x,
            model->header.y,
            model->header.horizontal,
            model->header.vertical,
            furi_string_get_cstr(model->header.text));
    }

    // Draw text
    canvas_set_font(canvas, FontSecondary);
    if(furi_string_size(model->text.text)) {
        elements_multiline_text_aligned(
            canvas,
            model->text.x,
            model->text.y,
            model->text.horizontal,
            model->text.vertical,
            furi_string_get_cstr(model->text.text));
    }

    // Draw buttons
    if(furi_string_size(model->left_text)) {
        elements_button_left(canvas, furi_string_get_cstr(model->left_text));
    }

    if(furi_string_size(model->center_text)) {
        elements_button_center(canvas, furi_string_get_cstr(model->center_text));
    }

    if(furi_string_size(model->right_text)) {
        elements_button_right(canvas, furi_string_get_cstr(model->right_text));
    }
}

static bool dialog_ex_view_input_callback(InputEvent* event, void* context) {
    DialogEx* dialog_ex = context;
    bool consumed = false;
    bool left_text_present = false;
    bool center_text_present = false;
    bool right_text_present = false;

    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        {
            left_text_present = furi_string_size(model->left_text);
            center_text_present = furi_string_size(model->center_text);
            right_text_present = furi_string_size(model->right_text);
        },
        false);

    if(dialog_ex->callback) {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyLeft && left_text_present) {
                dialog_ex->callback(DialogExResultLeft, dialog_ex->context);
                consumed = true;
            } else if(event->key == InputKeyOk && center_text_present) {
                dialog_ex->callback(DialogExResultCenter, dialog_ex->context);
                consumed = true;
            } else if(event->key == InputKeyRight && right_text_present) {
                dialog_ex->callback(DialogExResultRight, dialog_ex->context);
                consumed = true;
            }
        }

        if(event->type == InputTypePress && dialog_ex->enable_extended_events) {
            if(event->key == InputKeyLeft && left_text_present) {
                dialog_ex->callback(DialogExPressLeft, dialog_ex->context);
                consumed = true;
            } else if(event->key == InputKeyOk && center_text_present) {
                dialog_ex->callback(DialogExPressCenter, dialog_ex->context);
                consumed = true;
            } else if(event->key == InputKeyRight && right_text_present) {
                dialog_ex->callback(DialogExPressRight, dialog_ex->context);
                consumed = true;
            }
        }

        if(event->type == InputTypeRelease && dialog_ex->enable_extended_events) {
            if(event->key == InputKeyLeft && left_text_present) {
                dialog_ex->callback(DialogExReleaseLeft, dialog_ex->context);
                consumed = true;
            } else if(event->key == InputKeyOk && center_text_present) {
                dialog_ex->callback(DialogExReleaseCenter, dialog_ex->context);
                consumed = true;
            } else if(event->key == InputKeyRight && right_text_present) {
                dialog_ex->callback(DialogExReleaseRight, dialog_ex->context);
                consumed = true;
            }
        }
    }

    return consumed;
}

DialogEx* dialog_ex_alloc(void) {
    DialogEx* dialog_ex = malloc(sizeof(DialogEx));
    dialog_ex->view = view_alloc();
    view_set_context(dialog_ex->view, dialog_ex);
    view_allocate_model(dialog_ex->view, ViewModelTypeLocking, sizeof(DialogExModel));
    view_set_draw_callback(dialog_ex->view, dialog_ex_view_draw_callback);
    view_set_input_callback(dialog_ex->view, dialog_ex_view_input_callback);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        {
            model->header.text = furi_string_alloc();
            model->header.x = 0;
            model->header.y = 0;
            model->header.horizontal = AlignLeft;
            model->header.vertical = AlignBottom;

            model->text.text = furi_string_alloc();
            model->text.x = 0;
            model->text.y = 0;
            model->text.horizontal = AlignLeft;
            model->text.vertical = AlignBottom;

            model->icon.x = 0;
            model->icon.y = 0;
            model->icon.icon = NULL;

            model->left_text = furi_string_alloc();
            model->center_text = furi_string_alloc();
            model->right_text = furi_string_alloc();
        },
        false);
    dialog_ex->enable_extended_events = false;
    return dialog_ex;
}

void dialog_ex_free(DialogEx* dialog_ex) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        {
            furi_string_free(model->header.text);
            furi_string_free(model->text.text);
            furi_string_free(model->left_text);
            furi_string_free(model->center_text);
            furi_string_free(model->right_text);
        },
        false);
    view_free(dialog_ex->view);
    free(dialog_ex);
}

View* dialog_ex_get_view(DialogEx* dialog_ex) {
    furi_check(dialog_ex);
    return dialog_ex->view;
}

void dialog_ex_set_result_callback(DialogEx* dialog_ex, DialogExResultCallback callback) {
    furi_check(dialog_ex);
    dialog_ex->callback = callback;
}

void dialog_ex_set_context(DialogEx* dialog_ex, void* context) {
    furi_check(dialog_ex);
    dialog_ex->context = context;
}

void dialog_ex_set_header(
    DialogEx* dialog_ex,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        {
            furi_string_set(model->header.text, text ? text : "");
            model->header.x = x;
            model->header.y = y;
            model->header.horizontal = horizontal;
            model->header.vertical = vertical;
        },
        true);
}

void dialog_ex_set_text(
    DialogEx* dialog_ex,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        {
            furi_string_set(model->text.text, text ? text : "");
            model->text.x = x;
            model->text.y = y;
            model->text.horizontal = horizontal;
            model->text.vertical = vertical;
        },
        true);
}

void dialog_ex_set_icon(DialogEx* dialog_ex, uint8_t x, uint8_t y, const Icon* icon) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        {
            model->icon.x = x;
            model->icon.y = y;
            model->icon.icon = icon;
        },
        true);
}

void dialog_ex_set_left_button_text(DialogEx* dialog_ex, const char* text) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        { furi_string_set(model->left_text, text ? text : ""); },
        true);
}

void dialog_ex_set_center_button_text(DialogEx* dialog_ex, const char* text) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        { furi_string_set(model->center_text, text ? text : ""); },
        true);
}

void dialog_ex_set_right_button_text(DialogEx* dialog_ex, const char* text) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        { furi_string_set(model->right_text, text ? text : ""); },
        true);
}

void dialog_ex_reset(DialogEx* dialog_ex) {
    furi_check(dialog_ex);
    with_view_model(
        dialog_ex->view,
        DialogExModel * model,
        {
            model->icon.icon = NULL;
            furi_string_reset(model->header.text);
            furi_string_reset(model->text.text);

            furi_string_reset(model->left_text);
            furi_string_reset(model->center_text);
            furi_string_reset(model->right_text);
        },
        true);
    dialog_ex->context = NULL;
    dialog_ex->callback = NULL;
}

void dialog_ex_enable_extended_events(DialogEx* dialog_ex) {
    furi_check(dialog_ex);
    dialog_ex->enable_extended_events = true;
}

void dialog_ex_disable_extended_events(DialogEx* dialog_ex) {
    furi_check(dialog_ex);
    dialog_ex->enable_extended_events = false;
}
