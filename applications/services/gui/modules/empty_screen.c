#include "empty_screen.h"
#include <furi.h>

struct EmptyScreen {
    View* view;
};

static void empty_screen_view_draw_callback(Canvas* canvas, void* _model) {
    UNUSED(_model);
    canvas_clear(canvas);
}

static bool empty_screen_view_input_callback(InputEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);
    return false;
}

EmptyScreen* empty_screen_alloc(void) {
    EmptyScreen* empty_screen = malloc(sizeof(EmptyScreen));
    empty_screen->view = view_alloc();
    view_set_context(empty_screen->view, empty_screen);
    view_set_draw_callback(empty_screen->view, empty_screen_view_draw_callback);
    view_set_input_callback(empty_screen->view, empty_screen_view_input_callback);
    return empty_screen;
}

void empty_screen_free(EmptyScreen* empty_screen) {
    furi_check(empty_screen);
    view_free(empty_screen->view);
    free(empty_screen);
}

View* empty_screen_get_view(EmptyScreen* empty_screen) {
    furi_check(empty_screen);
    return empty_screen->view;
}
