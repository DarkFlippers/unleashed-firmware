#include "widget_element_i.h"
#include <m-string.h>
#include <gui/elements.h>

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    Align horizontal;
    Align vertical;
    string_t text;
} GuiTextBoxModel;

static void gui_text_box_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiTextBoxModel* model = element->model;

    if(string_size(model->text)) {
        elements_text_box(
            canvas,
            model->x,
            model->y,
            model->width,
            model->height,
            model->horizontal,
            model->vertical,
            string_get_cstr(model->text));
    }
}

static void gui_text_box_free(WidgetElement* gui_string) {
    furi_assert(gui_string);

    GuiTextBoxModel* model = gui_string->model;
    string_clear(model->text);
    free(gui_string->model);
    free(gui_string);
}

WidgetElement* widget_element_text_box_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    Align horizontal,
    Align vertical,
    const char* text) {
    furi_assert(text);

    // Allocate and init model
    GuiTextBoxModel* model = malloc(sizeof(GuiTextBoxModel));
    model->x = x;
    model->y = y;
    model->width = width;
    model->height = height;
    model->horizontal = horizontal;
    model->vertical = vertical;
    string_init_set_str(model->text, text);

    // Allocate and init Element
    WidgetElement* gui_string = malloc(sizeof(WidgetElement));
    gui_string->parent = NULL;
    gui_string->input = NULL;
    gui_string->draw = gui_text_box_draw;
    gui_string->free = gui_text_box_free;
    gui_string->model = model;

    return gui_string;
}
