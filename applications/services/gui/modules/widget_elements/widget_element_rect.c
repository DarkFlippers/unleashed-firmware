#include "widget_element_i.h"

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t radius;
    bool fill;
} GuiRectModel;

static void gui_rect_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiRectModel* model = element->model;
    if(model->fill) {
        canvas_draw_rbox(canvas, model->x, model->y, model->width, model->height, model->radius);
    } else {
        canvas_draw_rframe(canvas, model->x, model->y, model->width, model->height, model->radius);
    }
}

static void gui_rect_free(WidgetElement* gui_rect) {
    furi_assert(gui_rect);

    free(gui_rect->model);
    free(gui_rect);
}

WidgetElement* widget_element_rect_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius,
    bool fill) {
    // Allocate and init model
    GuiRectModel* model = malloc(sizeof(GuiRectModel));
    model->x = x;
    model->y = y;
    model->width = width;
    model->height = height;
    model->radius = radius;
    model->fill = fill;

    // Allocate and init Element
    WidgetElement* gui_rect = malloc(sizeof(WidgetElement));
    gui_rect->parent = NULL;
    gui_rect->input = NULL;
    gui_rect->draw = gui_rect_draw;
    gui_rect->free = gui_rect_free;
    gui_rect->model = model;

    return gui_rect;
}
