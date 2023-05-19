#include "variable_item_list.h"
#include <gui/elements.h>
#include <gui/canvas.h>
#include <furi.h>
#include <assets_icons.h>
#include <m-array.h>
#include <stdint.h>

struct VariableItem {
    const char* label;
    uint8_t current_value_index;
    FuriString* current_value_text;
    uint8_t values_count;
    VariableItemChangeCallback change_callback;
    bool locked;
    FuriString* locked_message;
    void* context;
};

ARRAY_DEF(VariableItemArray, VariableItem, M_POD_OPLIST);

struct VariableItemList {
    View* view;
    VariableItemListEnterCallback callback;
    void* context;
    FuriTimer* scroll_timer;
    FuriTimer* locked_timer;
};

typedef struct {
    VariableItemArray_t items;
    uint8_t position;
    uint8_t window_position;
    size_t scroll_counter;
    bool locked_message_visible;
} VariableItemListModel;

static void variable_item_list_process_up(VariableItemList* variable_item_list);
static void variable_item_list_process_down(VariableItemList* variable_item_list);
static void variable_item_list_process_left(VariableItemList* variable_item_list);
static void variable_item_list_process_right(VariableItemList* variable_item_list);
static void variable_item_list_process_ok(VariableItemList* variable_item_list);

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
            size_t scroll_counter = 0;

            if(position == model->position) {
                canvas_set_color(canvas, ColorBlack);
                elements_slightly_rounded_box(canvas, 0, item_y + 1, item_width, item_height - 2);
                canvas_set_color(canvas, ColorWhite);
                scroll_counter = model->scroll_counter;
                if(scroll_counter < 1) {
                    scroll_counter = 0;
                } else {
                    scroll_counter -= 1;
                }
            } else {
                canvas_set_color(canvas, ColorBlack);
            }

            if(item->current_value_index == 0 && furi_string_empty(item->current_value_text)) {
                // Only left text, no right text
                canvas_draw_str(canvas, 6, item_text_y, item->label);
            } else {
                elements_scrollable_text_line_str(
                    canvas, 6, item_text_y, 66, item->label, scroll_counter, false, false);
            }

            if(item->locked) {
                canvas_draw_icon(canvas, 110, item_text_y - 8, &I_Lock_7x8);
            } else {
                if(item->current_value_index > 0) {
                    canvas_draw_str(canvas, 73, item_text_y, "<");
                }

                elements_scrollable_text_line_str(
                    canvas,
                    (115 + 73) / 2 + 1,
                    item_text_y,
                    37,
                    furi_string_get_cstr(item->current_value_text),
                    scroll_counter,
                    false,
                    true);

                if(item->current_value_index < (item->values_count - 1)) {
                    canvas_draw_str(canvas, 115, item_text_y, ">");
                }
            }
        }

        position++;
    }

    elements_scrollbar(canvas, model->position, VariableItemArray_size(model->items));

    if(model->locked_message_visible) {
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 8, 10, 110, 48);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_icon(canvas, 10, 14, &I_WarningDolphin_45x42);
        canvas_draw_rframe(canvas, 8, 8, 112, 50, 3);
        canvas_draw_rframe(canvas, 9, 9, 110, 48, 2);
        elements_multiline_text_aligned(
            canvas,
            84,
            32,
            AlignCenter,
            AlignCenter,
            furi_string_get_cstr(
                VariableItemArray_get(model->items, model->position)->locked_message));
    }
}

void variable_item_list_set_selected_item(VariableItemList* variable_item_list, uint8_t index) {
    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
            uint8_t position = index;
            if(position >= VariableItemArray_size(model->items)) {
                position = 0;
            }

            model->position = position;
            model->window_position = position;

            if(model->window_position > 0) {
                model->window_position -= 1;
            }

            if(VariableItemArray_size(model->items) <= 4) {
                model->window_position = 0;
            } else {
                if(model->window_position >= (VariableItemArray_size(model->items) - 4)) {
                    model->window_position = (VariableItemArray_size(model->items) - 4);
                }
            }
        },
        true);
}

uint8_t variable_item_list_get_selected_item_index(VariableItemList* variable_item_list) {
    VariableItemListModel* model = view_get_model(variable_item_list->view);
    uint8_t idx = model->position;
    view_commit_model(variable_item_list->view, false);
    return idx;
}

static bool variable_item_list_input_callback(InputEvent* event, void* context) {
    VariableItemList* variable_item_list = context;
    furi_assert(variable_item_list);
    bool consumed = false;

    bool locked_message_visible = false;
    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        { locked_message_visible = model->locked_message_visible; },
        false);

    if((!(event->type == InputTypePress) && !(event->type == InputTypeRelease)) &&
       locked_message_visible) {
        with_view_model(
            variable_item_list->view,
            VariableItemListModel * model,
            { model->locked_message_visible = false; },
            true);
        consumed = true;
    } else if(event->type == InputTypeShort) {
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
        case InputKeyOk:
            variable_item_list_process_ok(variable_item_list);
            break;
        default:
            break;
        }
    } else if(event->type == InputTypeRepeat) {
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
        variable_item_list->view,
        VariableItemListModel * model,
        {
            uint8_t items_on_screen = 4;
            if(model->position > 0) {
                model->position--;

                if((model->position == model->window_position) && (model->window_position > 0)) {
                    model->window_position--;
                }
            } else {
                model->position = VariableItemArray_size(model->items) - 1;
                if(model->position > (items_on_screen - 1)) {
                    model->window_position = model->position - (items_on_screen - 1);
                }
            }
            model->scroll_counter = 0;
        },
        true);
}

void variable_item_list_process_down(VariableItemList* variable_item_list) {
    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
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
            model->scroll_counter = 0;
        },
        true);
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
        variable_item_list->view,
        VariableItemListModel * model,
        {
            VariableItem* item = variable_item_list_get_selected_item(model);
            if(item->locked) {
                model->locked_message_visible = true;
                furi_timer_start(
                    variable_item_list->locked_timer, furi_kernel_get_tick_frequency() * 3);
            } else if(item->current_value_index > 0) {
                item->current_value_index--;
                model->scroll_counter = 0;
                if(item->change_callback) {
                    item->change_callback(item);
                }
            }
        },
        true);
}

void variable_item_list_process_right(VariableItemList* variable_item_list) {
    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
            VariableItem* item = variable_item_list_get_selected_item(model);
            if(item->locked) {
                model->locked_message_visible = true;
                furi_timer_start(
                    variable_item_list->locked_timer, furi_kernel_get_tick_frequency() * 3);
            } else if(item->current_value_index < (item->values_count - 1)) {
                item->current_value_index++;
                model->scroll_counter = 0;
                if(item->change_callback) {
                    item->change_callback(item);
                }
            }
        },
        true);
}

void variable_item_list_process_ok(VariableItemList* variable_item_list) {
    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
            VariableItem* item = variable_item_list_get_selected_item(model);
            if(item->locked) {
                model->locked_message_visible = true;
                furi_timer_start(
                    variable_item_list->locked_timer, furi_kernel_get_tick_frequency() * 3);
            } else if(variable_item_list->callback) {
                variable_item_list->callback(variable_item_list->context, model->position);
            }
        },
        true);
}

static void variable_item_list_scroll_timer_callback(void* context) {
    VariableItemList* variable_item_list = context;
    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        { model->scroll_counter++; },
        true);
}

void variable_item_list_locked_timer_callback(void* context) {
    furi_assert(context);
    VariableItemList* variable_item_list = context;

    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        { model->locked_message_visible = false; },
        true);
}

VariableItemList* variable_item_list_alloc() {
    VariableItemList* variable_item_list = malloc(sizeof(VariableItemList));
    variable_item_list->view = view_alloc();
    view_set_context(variable_item_list->view, variable_item_list);
    view_allocate_model(
        variable_item_list->view, ViewModelTypeLocking, sizeof(VariableItemListModel));
    view_set_draw_callback(variable_item_list->view, variable_item_list_draw_callback);
    view_set_input_callback(variable_item_list->view, variable_item_list_input_callback);

    variable_item_list->locked_timer = furi_timer_alloc(
        variable_item_list_locked_timer_callback, FuriTimerTypeOnce, variable_item_list);

    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
            VariableItemArray_init(model->items);
            model->position = 0;
            model->window_position = 0;
            model->scroll_counter = 0;
        },
        true);
    variable_item_list->scroll_timer = furi_timer_alloc(
        variable_item_list_scroll_timer_callback, FuriTimerTypePeriodic, variable_item_list);
    furi_timer_start(variable_item_list->scroll_timer, 333);

    return variable_item_list;
}

void variable_item_list_free(VariableItemList* variable_item_list) {
    furi_assert(variable_item_list);

    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
            VariableItemArray_it_t it;
            for(VariableItemArray_it(it, model->items); !VariableItemArray_end_p(it);
                VariableItemArray_next(it)) {
                furi_string_free(VariableItemArray_ref(it)->current_value_text);
                furi_string_free(VariableItemArray_ref(it)->locked_message);
            }
            VariableItemArray_clear(model->items);
        },
        false);
    furi_timer_stop(variable_item_list->scroll_timer);
    furi_timer_free(variable_item_list->scroll_timer);
    furi_timer_stop(variable_item_list->locked_timer);
    furi_timer_free(variable_item_list->locked_timer);
    view_free(variable_item_list->view);
    free(variable_item_list);
}

void variable_item_list_reset(VariableItemList* variable_item_list) {
    furi_assert(variable_item_list);

    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
            VariableItemArray_it_t it;
            for(VariableItemArray_it(it, model->items); !VariableItemArray_end_p(it);
                VariableItemArray_next(it)) {
                furi_string_free(VariableItemArray_ref(it)->current_value_text);
                furi_string_free(VariableItemArray_ref(it)->locked_message);
            }
            VariableItemArray_reset(model->items);
        },
        false);
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
        variable_item_list->view,
        VariableItemListModel * model,
        {
            item = VariableItemArray_push_new(model->items);
            item->label = label;
            item->values_count = values_count;
            item->change_callback = change_callback;
            item->context = context;
            item->current_value_index = 0;
            item->current_value_text = furi_string_alloc();
            item->locked = false;
            item->locked_message = furi_string_alloc();
        },
        true);

    return item;
}

void variable_item_list_set_enter_callback(
    VariableItemList* variable_item_list,
    VariableItemListEnterCallback callback,
    void* context) {
    furi_assert(callback);
    with_view_model(
        variable_item_list->view,
        VariableItemListModel * model,
        {
            UNUSED(model);
            variable_item_list->callback = callback;
            variable_item_list->context = context;
        },
        false);
}

void variable_item_set_current_value_index(VariableItem* item, uint8_t current_value_index) {
    item->current_value_index = current_value_index;
}

void variable_item_set_values_count(VariableItem* item, uint8_t values_count) {
    item->values_count = values_count;
}

void variable_item_set_current_value_text(VariableItem* item, const char* current_value_text) {
    furi_string_set(item->current_value_text, current_value_text);
}

void variable_item_set_locked(VariableItem* item, bool locked, const char* locked_message) {
    item->locked = locked;
    if(locked) {
        furi_assert(locked_message);
        furi_string_set(item->locked_message, locked_message);
    }
}

uint8_t variable_item_get_current_value_index(VariableItem* item) {
    return item->current_value_index;
}

void* variable_item_get_context(VariableItem* item) {
    return item->context;
}
