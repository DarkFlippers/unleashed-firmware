#include "variable-item-list.h"
#include "gui/canvas.h"
#include <m-array.h>
#include <furi.h>
#include <gui/elements.h>
#include <stdint.h>

struct VariableItem {
    const char* label;
    uint8_t current_value_index;
    string_t current_value_text;
    uint8_t values_count;
    VariableItemChangeCallback change_callback;
    void* context;
};

ARRAY_DEF(VariableItemArray, VariableItem, M_POD_OPLIST);

struct VariableItemList {
    View* view;
};

typedef struct {
    VariableItemArray_t items;
    uint8_t position;
    uint8_t window_position;
} VariableItemListModel;

static void variable_item_list_process_up(VariableItemList* variable_item_list);
static void variable_item_list_process_down(VariableItemList* variable_item_list);
static void variable_item_list_process_left(VariableItemList* variable_item_list);
static void variable_item_list_process_right(VariableItemList* variable_item_list);

static void variable_item_list_draw_callback(Canvas* canvas, void* _model) {
    VariableItemListModel* model = _model;

    const uint8_t item_height = 16;
    const uint8_t item_width = 123;

    canvas_clear(canvas);

    uint8_t position = 0;
    VariableItemArray_it_t it;

    canvas_set_font(canvas, FontSecondary);
    for(VariableItemArray_it(it, model->items); !VariableItemArray_end_p(it);
        VariableItemArray_next(it)) {
        uint8_t item_position = position - model->window_position;
        uint8_t items_on_screen = 4;
        uint8_t y_offset = 0;

        if(item_position < items_on_screen) {
            const VariableItem* item = VariableItemArray_cref(it);
            uint8_t item_y = y_offset + (item_position * item_height);
            uint8_t item_text_y = item_y + item_height - 4;

            if(position == model->position) {
                canvas_set_color(canvas, ColorBlack);
                elements_slightly_rounded_box(canvas, 0, item_y + 1, item_width, item_height - 2);
                canvas_set_color(canvas, ColorWhite);
            } else {
                canvas_set_color(canvas, ColorBlack);
            }

            canvas_draw_str(canvas, 6, item_text_y, item->label);

            if(item->current_value_index > 0) {
                canvas_draw_str(canvas, 73, item_text_y, "<");
            }

            canvas_draw_str(canvas, 84, item_text_y, string_get_cstr(item->current_value_text));

            if(item->current_value_index < (item->values_count - 1)) {
                canvas_draw_str(canvas, 113, item_text_y, ">");
            }
        }

        position++;
    }

    elements_scrollbar(canvas, model->position, VariableItemArray_size(model->items));
}

static bool variable_item_list_input_callback(InputEvent* event, void* context) {
    VariableItemList* variable_item_list = context;
    furi_assert(variable_item_list);
    bool consumed = false;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyUp:
            consumed = true;
            variable_item_list_process_up(variable_item_list);
            break;
        case InputKeyDown:
            consumed = true;
            variable_item_list_process_down(variable_item_list);
            break;
        case InputKeyLeft:
            consumed = true;
            variable_item_list_process_left(variable_item_list);
            break;
        case InputKeyRight:
            consumed = true;
            variable_item_list_process_right(variable_item_list);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void variable_item_list_process_up(VariableItemList* variable_item_list) {
    with_view_model(
        variable_item_list->view, (VariableItemListModel * model) {
            uint8_t items_on_screen = 4;
            if(model->position > 0) {
                model->position--;
                if(((model->position - model->window_position) < 1) &&
                   model->window_position > 0) {
                    model->window_position--;
                }
            } else {
                model->position = VariableItemArray_size(model->items) - 1;
                if(model->position > (items_on_screen - 1)) {
                    model->window_position = model->position - (items_on_screen - 1);
                }
            }
            return true;
        });
}

void variable_item_list_process_down(VariableItemList* variable_item_list) {
    with_view_model(
        variable_item_list->view, (VariableItemListModel * model) {
            uint8_t items_on_screen = 4;
            if(model->position < (VariableItemArray_size(model->items) - 1)) {
                model->position++;
                if((model->position - model->window_position) > (items_on_screen - 2) &&
                   model->window_position <
                       (VariableItemArray_size(model->items) - items_on_screen)) {
                    model->window_position++;
                }
            } else {
                model->position = 0;
                model->window_position = 0;
            }
            return true;
        });
}

VariableItem* variable_item_list_get_selected_item(VariableItemListModel* model) {
    VariableItem* item = NULL;

    VariableItemArray_it_t it;
    uint8_t position = 0;
    for(VariableItemArray_it(it, model->items); !VariableItemArray_end_p(it);
        VariableItemArray_next(it)) {
        if(position == model->position) {
            break;
        }
        position++;
    }

    item = VariableItemArray_ref(it);

    furi_assert(item);
    return item;
}

void variable_item_list_process_left(VariableItemList* variable_item_list) {
    with_view_model(
        variable_item_list->view, (VariableItemListModel * model) {
            VariableItem* item = variable_item_list_get_selected_item(model);
            if(item->current_value_index > 0) {
                item->current_value_index--;
                if(item->change_callback) {
                    item->change_callback(item);
                }
            }
            return true;
        });
}

void variable_item_list_process_right(VariableItemList* variable_item_list) {
    with_view_model(
        variable_item_list->view, (VariableItemListModel * model) {
            VariableItem* item = variable_item_list_get_selected_item(model);
            if(item->current_value_index < (item->values_count - 1)) {
                item->current_value_index++;
                if(item->change_callback) {
                    item->change_callback(item);
                }
            }
            return true;
        });
}

VariableItemList* variable_item_list_alloc() {
    VariableItemList* variable_item_list = furi_alloc(sizeof(VariableItemList));
    variable_item_list->view = view_alloc();
    view_set_context(variable_item_list->view, variable_item_list);
    view_allocate_model(
        variable_item_list->view, ViewModelTypeLocking, sizeof(VariableItemListModel));
    view_set_draw_callback(variable_item_list->view, variable_item_list_draw_callback);
    view_set_input_callback(variable_item_list->view, variable_item_list_input_callback);

    with_view_model(
        variable_item_list->view, (VariableItemListModel * model) {
            VariableItemArray_init(model->items);
            model->position = 0;
            model->window_position = 0;
            return true;
        });

    return variable_item_list;
}

void variable_item_list_free(VariableItemList* variable_item_list) {
    furi_assert(variable_item_list);

    with_view_model(
        variable_item_list->view, (VariableItemListModel * model) {
            VariableItemArray_it_t it;
            for(VariableItemArray_it(it, model->items); !VariableItemArray_end_p(it);
                VariableItemArray_next(it)) {
                string_clear(VariableItemArray_ref(it)->current_value_text);
            }
            VariableItemArray_clear(model->items);
            return false;
        });
    view_free(variable_item_list->view);
    free(variable_item_list);
}

View* variable_item_list_get_view(VariableItemList* variable_item_list) {
    furi_assert(variable_item_list);
    return variable_item_list->view;
}

VariableItem* variable_item_list_add(
    VariableItemList* variable_item_list,
    const char* label,
    uint8_t values_count,
    VariableItemChangeCallback change_callback,
    void* context) {
    VariableItem* item = NULL;
    furi_assert(label);
    furi_assert(variable_item_list);

    with_view_model(
        variable_item_list->view, (VariableItemListModel * model) {
            item = VariableItemArray_push_new(model->items);
            item->label = label;
            item->values_count = values_count;
            item->change_callback = change_callback;
            item->context = context;
            item->current_value_index = 0;
            string_init(item->current_value_text);
            return true;
        });

    return item;
}

void variable_item_set_current_value_index(VariableItem* item, uint8_t current_value_index) {
    item->current_value_index = current_value_index;
}

void variable_item_set_current_value_text(VariableItem* item, const char* current_value_text) {
    string_set_str(item->current_value_text, current_value_text);
}

uint8_t variable_item_get_current_value_index(VariableItem* item) {
    return item->current_value_index;
}

void* variable_item_get_context(VariableItem* item) {
    return item->context;
}