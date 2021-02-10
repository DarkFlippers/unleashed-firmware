#include "dialog.h"
#include <gui/elements.h>
#include <furi.h>

struct Dialog {
    View* view;
    void* context;
    DialogResultCallback callback;
};

typedef struct {
    const char* header_text;
    const char* text;
    const char* left_text;
    const char* right_text;
} DialogModel;

static void dialog_view_draw_callback(Canvas* canvas, void* _model) {
    DialogModel* model = _model;
    uint8_t canvas_center = canvas_width(canvas) / 2;

    // Prepare canvas
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Draw header
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, canvas_center, 17, AlignCenter, AlignBottom, model->header_text);

    // Draw text
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas, canvas_center, 32, AlignCenter, AlignCenter, model->text);

    // Draw buttons
    elements_button_left(canvas, model->left_text);
    elements_button_right(canvas, model->right_text);
}

static bool dialog_view_input_callback(InputEvent* event, void* context) {
    Dialog* dialog = context;
    bool consumed = false;

    // Process key presses only
    if(event->type == InputTypeShort && dialog->callback) {
        if(event->key == InputKeyLeft) {
            dialog->callback(DialogResultLeft, dialog->context);
            consumed = true;
        } else if(event->key == InputKeyRight) {
            dialog->callback(DialogResultRight, dialog->context);
            consumed = true;
        }
    }

    return consumed;
}

Dialog* dialog_alloc() {
    Dialog* dialog = furi_alloc(sizeof(Dialog));
    dialog->view = view_alloc();
    view_set_context(dialog->view, dialog);
    view_allocate_model(dialog->view, ViewModelTypeLockFree, sizeof(DialogModel));
    view_set_draw_callback(dialog->view, dialog_view_draw_callback);
    view_set_input_callback(dialog->view, dialog_view_input_callback);
    return dialog;
}

void dialog_free(Dialog* dialog) {
    furi_assert(dialog);
    view_free(dialog->view);
    free(dialog);
}

View* dialog_get_view(Dialog* dialog) {
    furi_assert(dialog);
    return dialog->view;
}

void dialog_set_result_callback(Dialog* dialog, DialogResultCallback callback) {
    furi_assert(dialog);
    dialog->callback = callback;
}

void dialog_set_context(Dialog* dialog, void* context) {
    furi_assert(dialog);
    dialog->context = context;
}

void dialog_set_header_text(Dialog* dialog, const char* text) {
    furi_assert(dialog);
    furi_assert(text);
    with_view_model(
        dialog->view, (DialogModel * model) { model->header_text = text; });
}

void dialog_set_text(Dialog* dialog, const char* text) {
    furi_assert(dialog);
    furi_assert(text);
    with_view_model(
        dialog->view, (DialogModel * model) { model->text = text; });
}

void dialog_set_left_button_text(Dialog* dialog, const char* text) {
    furi_assert(dialog);
    furi_assert(text);
    with_view_model(
        dialog->view, (DialogModel * model) { model->left_text = text; });
}

void dialog_set_right_button_text(Dialog* dialog, const char* text) {
    furi_assert(dialog);
    furi_assert(text);
    with_view_model(
        dialog->view, (DialogModel * model) { model->right_text = text; });
}
