#include "widget_element_i.h"

typedef struct {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
} GuiLineModel;

static void gui_line_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiLineModel* model = element->model;
    canvas_draw_line(canvas, model->x1, model->y1, model->x2, model->y2);
}

static void gui_line_free(WidgetElement* gui_line) {
    furi_assert(gui_line);

    free(gui_line->model);
    free(gui_line);
}

WidgetElement* widget_element_line_create(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    // Allocate and init model
    GuiLineModel* model = malloc(sizeof(GuiLineModel));
    model->x1 = x1;
    model->y1 = y1;
    model->x2 = x2;
    model->y2 = y2;

    // Allocate and init Element
    WidgetElement* gui_line = malloc(sizeof(WidgetElement));
    gui_line->parent = NULL;
    gui_line->input = NULL;
    gui_line->draw = gui_line_draw;
    gui_line->free = gui_line_free;
    gui_line->model = model;

    return gui_line;
}
