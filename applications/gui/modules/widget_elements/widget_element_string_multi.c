#include "widget_element_i.h"
#include <m-string.h>
#include <gui/elements.h>

typedef struct {
    uint8_t x;
    uint8_t y;
    Align horizontal;
    Align vertical;
    Font font;
    string_t text;
} GuiStringMultiModel;

static void gui_string_multi_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiStringMultiModel* model = element->model;

    if(string_size(model->text)) {
        canvas_set_font(canvas, model->font);
        elements_multiline_text_aligned(
            canvas,
            model->x,
            model->y,
            model->horizontal,
            model->vertical,
            string_get_cstr(model->text));
    }
}

static void gui_string_multi_free(WidgetElement* gui_string) {
    furi_assert(gui_string);

    GuiStringMultiModel* model = gui_string->model;
    string_clear(model->text);
    free(gui_string->model);
    free(gui_string);
}

WidgetElement* widget_element_string_multi_create(
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text) {
    furi_assert(text);

    // Allocate and init model
    GuiStringMultiModel* model = furi_alloc(sizeof(GuiStringMultiModel));
    model->x = x;
    model->y = y;
    model->horizontal = horizontal;
    model->vertical = vertical;
    model->font = font;
    string_init_set_str(model->text, text);

    // Allocate and init Element
    WidgetElement* gui_string = furi_alloc(sizeof(WidgetElement));
    gui_string->parent = NULL;
    gui_string->input = NULL;
    gui_string->draw = gui_string_multi_draw;
    gui_string->free = gui_string_multi_free;
    gui_string->model = model;

    return gui_string;
}
