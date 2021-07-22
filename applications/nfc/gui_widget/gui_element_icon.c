#include "gui_element_i.h"
#include "gui_element_icon.h"
#include "gui_widget.h"

#include <m-string.h>

typedef struct {
    uint8_t x;
    uint8_t y;
    const Icon* icon;
} GuiIconModel;

static void gui_icon_draw(Canvas* canvas, GuiElement* element) {
    furi_assert(canvas);
    furi_assert(element);
    GuiIconModel* model = element->model;

    if(model->icon) {
        canvas_draw_icon(canvas, model->x, model->y, model->icon);
    }
}

static void gui_icon_free(GuiElement* gui_icon) {
    furi_assert(gui_icon);

    if(gui_icon->parent != NULL) {
        // TODO deattach element
    }
    free(gui_icon->model);
    free(gui_icon);
}

GuiElement* gui_icon_create(uint8_t x, uint8_t y, const Icon* icon) {
    furi_assert(icon);

    // Allocate and init model
    GuiIconModel* model = furi_alloc(sizeof(GuiIconModel));
    model->x = x;
    model->y = y;
    model->icon = icon;

    // Allocate and init Element
    GuiElement* gui_icon = furi_alloc(sizeof(GuiElement));
    gui_icon->parent = NULL;
    gui_icon->input = NULL;
    gui_icon->draw = gui_icon_draw;
    gui_icon->free = gui_icon_free;
    gui_icon->model = model;

    return gui_icon;
}
