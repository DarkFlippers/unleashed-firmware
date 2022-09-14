#include "widget_element_i.h"

typedef struct {
    uint8_t x;
    uint8_t y;
    const Icon* icon;
} GuiIconModel;

static void gui_icon_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiIconModel* model = element->model;

    if(model->icon) {
        canvas_draw_icon(canvas, model->x, model->y, model->icon);
    }
}

static void gui_icon_free(WidgetElement* gui_icon) {
    furi_assert(gui_icon);

    free(gui_icon->model);
    free(gui_icon);
}

WidgetElement* widget_element_icon_create(uint8_t x, uint8_t y, const Icon* icon) {
    furi_assert(icon);

    // Allocate and init model
    GuiIconModel* model = malloc(sizeof(GuiIconModel));
    model->x = x;
    model->y = y;
    model->icon = icon;

    // Allocate and init Element
    WidgetElement* gui_icon = malloc(sizeof(WidgetElement));
    gui_icon->parent = NULL;
    gui_icon->input = NULL;
    gui_icon->draw = gui_icon_draw;
    gui_icon->free = gui_icon_free;
    gui_icon->model = model;

    return gui_icon;
}
