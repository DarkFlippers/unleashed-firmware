#include "widget_element_i.h"

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t radius;
    bool fill;
} GuiCircleModel;

static void gui_circle_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiCircleModel* model = element->model;
    if(model->fill) {
        canvas_draw_disc(canvas, model->x, model->y, model->radius);
    } else {
        canvas_draw_circle(canvas, model->x, model->y, model->radius);
    }
}

static void gui_circle_free(WidgetElement* gui_circle) {
    furi_assert(gui_circle);

    free(gui_circle->model);
    free(gui_circle);
}

WidgetElement* widget_element_circle_create(uint8_t x, uint8_t y, uint8_t radius, bool fill) {
    // Allocate and init model
    GuiCircleModel* model = malloc(sizeof(GuiCircleModel));
    model->x = x;
    model->y = y;
    model->radius = radius;
    model->fill = fill;

    // Allocate and init Element
    WidgetElement* gui_circle = malloc(sizeof(WidgetElement));
    gui_circle->parent = NULL;
    gui_circle->input = NULL;
    gui_circle->draw = gui_circle_draw;
    gui_circle->free = gui_circle_free;
    gui_circle->model = model;

    return gui_circle;
}
