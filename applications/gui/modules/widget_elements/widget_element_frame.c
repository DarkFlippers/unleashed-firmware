#include "widget_element_i.h"

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t radius;
} GuiFrameModel;

static void gui_frame_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiFrameModel* model = element->model;
    canvas_draw_rframe(canvas, model->x, model->y, model->width, model->height, model->radius);
}

static void gui_frame_free(WidgetElement* gui_frame) {
    furi_assert(gui_frame);

    free(gui_frame->model);
    free(gui_frame);
}

WidgetElement* widget_element_frame_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius) {
    // Allocate and init model
    GuiFrameModel* model = malloc(sizeof(GuiFrameModel));
    model->x = x;
    model->y = y;
    model->width = width;
    model->height = height;
    model->radius = radius;

    // Allocate and init Element
    WidgetElement* gui_frame = malloc(sizeof(WidgetElement));
    gui_frame->parent = NULL;
    gui_frame->input = NULL;
    gui_frame->draw = gui_frame_draw;
    gui_frame->free = gui_frame_free;
    gui_frame->model = model;

    return gui_frame;
}
