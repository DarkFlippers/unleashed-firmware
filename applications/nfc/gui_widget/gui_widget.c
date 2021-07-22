#include <furi.h>
#include "gui_element_i.h"
#include "gui_widget.h"

#define MAX_GUI_ELEMENTS 8

struct GuiWidget {
    View* view;
    void* context;
};

// TODO rework with M-LIB container
typedef struct {
    GuiElement* element[MAX_GUI_ELEMENTS];
} GuiWidgetModel;

static void gui_widget_view_draw_callback(Canvas* canvas, void* _model) {
    GuiWidgetModel* model = _model;
    canvas_clear(canvas);

    for(uint8_t i = 0; i < MAX_GUI_ELEMENTS; i++) {
        if(model->element[i] != NULL) {
            if(model->element[i]->draw != NULL) {
                model->element[i]->draw(canvas, model->element[i]);
            }
        }
    };
}

static bool gui_widget_view_input_callback(InputEvent* event, void* context) {
    GuiWidget* gui_widget = context;
    bool consumed = false;

    with_view_model(
        gui_widget->view, (GuiWidgetModel * model) {
            for(uint8_t i = 0; i < MAX_GUI_ELEMENTS; i++) {
                if(model->element[i] != NULL) {
                    if(model->element[i]->input != NULL) {
                        consumed = model->element[i]->input(event, model->element[i]);
                    }
                }
            };
            return true;
        });

    return consumed;
}

GuiWidget* gui_widget_alloc() {
    GuiWidget* gui_widget = furi_alloc(sizeof(GuiWidget));
    gui_widget->view = view_alloc();
    view_set_context(gui_widget->view, gui_widget);
    view_allocate_model(gui_widget->view, ViewModelTypeLocking, sizeof(GuiWidgetModel));
    view_set_draw_callback(gui_widget->view, gui_widget_view_draw_callback);
    view_set_input_callback(gui_widget->view, gui_widget_view_input_callback);

    with_view_model(
        gui_widget->view, (GuiWidgetModel * model) {
            for(uint8_t i = 0; i < MAX_GUI_ELEMENTS; i++) {
                model->element[i] = NULL;
            };
            return true;
        });

    return gui_widget;
}

void gui_widget_free(GuiWidget* gui_widget) {
    furi_assert(gui_widget);
    gui_widget_clear(gui_widget);
    view_free(gui_widget->view);
    free(gui_widget);
}

void gui_widget_clear(GuiWidget* gui_widget) {
    furi_assert(gui_widget);

    with_view_model(
        gui_widget->view, (GuiWidgetModel * model) {
            for(uint8_t i = 0; i < MAX_GUI_ELEMENTS; i++) {
                if(model->element[i]) {
                    furi_assert(model->element[i]->free);
                    model->element[i]->free(model->element[i]);
                    model->element[i] = NULL;
                }
            };
            return true;
        });
}

View* gui_widget_get_view(GuiWidget* gui_widget) {
    furi_assert(gui_widget);
    return gui_widget->view;
}

void gui_widget_add_element(GuiWidget* gui_widget, GuiElement* element) {
    furi_assert(gui_widget);
    with_view_model(
        gui_widget->view, (GuiWidgetModel * model) {
            // add element to first null position
            for(uint8_t i = 0; i < MAX_GUI_ELEMENTS; i++) {
                if(model->element[i] == NULL) {
                    model->element[i] = element;
                    element->parent = gui_widget;
                    break;
                }
            };
            return true;
        });
}

void gui_widget_add_string_element(
    GuiWidget* gui_widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text) {
    furi_assert(gui_widget);
    GuiElement* string_element = gui_string_create(x, y, horizontal, vertical, font, text);
    gui_widget_add_element(gui_widget, string_element);
}

void gui_widget_add_button_element(
    GuiWidget* gui_widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context) {
    furi_assert(gui_widget);
    GuiElement* button_element = gui_button_create(button_type, text, callback, context);
    gui_widget_add_element(gui_widget, button_element);
}

void gui_widget_add_icon_element(GuiWidget* gui_widget, uint8_t x, uint8_t y, const Icon* icon) {
    furi_assert(gui_widget);
    furi_assert(icon);
    GuiElement* icon_element = gui_icon_create(x, y, icon);
    gui_widget_add_element(gui_widget, icon_element);
}
