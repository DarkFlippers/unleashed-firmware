#include "infrared_debug_view.h"

#include <gui/canvas.h>
#include <gui/elements.h>

#include <stdlib.h>
#include <string.h>

#define INFRARED_DEBUG_TEXT_LENGTH 64

struct InfraredDebugView {
    View* view;
};

typedef struct {
    char text[INFRARED_DEBUG_TEXT_LENGTH];
} InfraredDebugViewModel;

static void infrared_debug_view_draw_callback(Canvas* canvas, void* model) {
    InfraredDebugViewModel* debug_view_model = model;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 0, AlignCenter, AlignTop, "INFRARED monitor\n");
    canvas_set_font(canvas, FontKeyboard);

    if(strlen(debug_view_model->text)) {
        elements_multiline_text_aligned(
            canvas, 64, 43, AlignCenter, AlignCenter, debug_view_model->text);
    }
}

InfraredDebugView* infrared_debug_view_alloc() {
    InfraredDebugView* debug_view = malloc(sizeof(InfraredDebugView));
    debug_view->view = view_alloc();
    view_allocate_model(debug_view->view, ViewModelTypeLocking, sizeof(InfraredDebugViewModel));
    view_set_draw_callback(debug_view->view, infrared_debug_view_draw_callback);
    view_set_context(debug_view->view, debug_view);
    return debug_view;
}
void infrared_debug_view_free(InfraredDebugView* debug_view) {
    view_free(debug_view->view);
    free(debug_view);
}

View* infrared_debug_view_get_view(InfraredDebugView* debug_view) {
    return debug_view->view;
}

void infrared_debug_view_set_text(InfraredDebugView* debug_view, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    InfraredDebugViewModel* model = view_get_model(debug_view->view);
    vsnprintf(model->text, INFRARED_DEBUG_TEXT_LENGTH, fmt, args);
    view_commit_model(debug_view->view, true);

    va_end(args);
}
