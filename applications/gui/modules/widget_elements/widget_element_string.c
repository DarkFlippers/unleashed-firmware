#include "widget_element_i.h"
#include <m-string.h>

typedef struct {
    uint8_t x;
    uint8_t y;
    Align horizontal;
    Align vertical;
    Font font;
    string_t text;
} GuiStringModel;

static void gui_string_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiStringModel* model = element->model;

    if(string_size(model->text)) {
        canvas_set_font(canvas, model->font);
        canvas_draw_str_aligned(
            canvas,
            model->x,
            model->y,
            model->horizontal,
            model->vertical,
            string_get_cstr(model->text));
    }
}

static void gui_string_free(WidgetElement* gui_string) {
    furi_assert(gui_string);

    GuiStringModel* model = gui_string->model;
    string_clear(model->text);
    free(gui_string->model);
    free(gui_string);
}

WidgetElement* widget_element_string_create(
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text) {
    furi_assert(text);

    // Allocate and init model
    GuiStringModel* model = malloc(sizeof(GuiStringModel));
    model->x = x;
    model->y = y;
    model->horizontal = horizontal;
    model->vertical = vertical;
    model->font = font;
    string_init_set_str(model->text, text);

    // Allocate and init Element
    WidgetElement* gui_string = malloc(sizeof(WidgetElement));
    gui_string->parent = NULL;
    gui_string->input = NULL;
    gui_string->draw = gui_string_draw;
    gui_string->free = gui_string_free;
    gui_string->model = model;

    return gui_string;
}
