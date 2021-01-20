#include "dialog.h"
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
    // Prepare canvas
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    // Draw header
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, model->header_text);
    // Draw text
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 22, model->text);
    // Draw buttons
    uint8_t bottom_base_line = canvas_height(canvas) - 2;
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, bottom_base_line, model->left_text);
    canvas_draw_str(canvas, 69, bottom_base_line, model->right_text);
}

static bool dialog_view_input_callback(InputEvent* event, void* context) {
    Dialog* dialog = context;
    // Process key presses only
    if(event->state && dialog->callback) {
        if(event->input == InputLeft) {
            dialog->callback(DialogResultLeft, dialog->context);
        } else if(event->input == InputRight) {
            dialog->callback(DialogResultRight, dialog->context);
        }
    }
    // All input events consumed
    return true;
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
