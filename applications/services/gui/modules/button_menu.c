#include "button_menu.h"
#include "gui/canvas.h"
#include "gui/elements.h"
#include "input/input.h"
#include <m-array.h>
#include <furi.h>
#include <stdint.h>

#define ITEM_FIRST_OFFSET 17
#define ITEM_NEXT_OFFSET 4
#define ITEM_HEIGHT 14
#define ITEM_WIDTH 64
#define BUTTONS_PER_SCREEN 6

struct ButtonMenuItem {
    const char* label;
    int32_t index;
    ButtonMenuItemCallback callback;
    ButtonMenuItemType type;
    void* callback_context;
};

ARRAY_DEF(ButtonMenuItemArray, ButtonMenuItem, M_POD_OPLIST);

struct ButtonMenu {
    View* view;
    bool freeze_input;
};

typedef struct {
    ButtonMenuItemArray_t items;
    uint8_t position;
    const char* header;
} ButtonMenuModel;

static void button_menu_draw_control_button(
    Canvas* canvas,
    uint8_t item_position,
    const char* text,
    bool selected) {
    furi_assert(canvas);
    furi_assert(text);

    uint8_t item_x = 0;
    uint8_t item_y = ITEM_FIRST_OFFSET + (item_position * (ITEM_HEIGHT + ITEM_NEXT_OFFSET));

    canvas_set_color(canvas, ColorBlack);

    if(selected) {
        elements_slightly_rounded_box(canvas, item_x, item_y, ITEM_WIDTH, ITEM_HEIGHT);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str_aligned(
        canvas,
        item_x + (ITEM_WIDTH / 2),
        item_y + (ITEM_HEIGHT / 2),
        AlignCenter,
        AlignCenter,
        text);
}

static void button_menu_draw_common_button(
    Canvas* canvas,
    uint8_t item_position,
    const char* text,
    bool selected) {
    furi_assert(canvas);
    furi_assert(text);

    uint8_t item_x = 0;
    uint8_t item_y = ITEM_FIRST_OFFSET + (item_position * (ITEM_HEIGHT + ITEM_NEXT_OFFSET));

    canvas_set_color(canvas, ColorBlack);

    if(selected) {
        canvas_draw_rbox(canvas, item_x, item_y, ITEM_WIDTH, ITEM_HEIGHT, 5);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_draw_rframe(canvas, item_x, item_y, ITEM_WIDTH, ITEM_HEIGHT, 5);
    }

    string_t disp_str;
    string_init_set_str(disp_str, text);
    elements_string_fit_width(canvas, disp_str, ITEM_WIDTH - 6);

    canvas_draw_str_aligned(
        canvas,
        item_x + (ITEM_WIDTH / 2),
        item_y + (ITEM_HEIGHT / 2),
        AlignCenter,
        AlignCenter,
        string_get_cstr(disp_str));

    string_clear(disp_str);
}

static void button_menu_view_draw_callback(Canvas* canvas, void* _model) {
    furi_assert(canvas);
    furi_assert(_model);

    ButtonMenuModel* model = (ButtonMenuModel*)_model;
    canvas_set_font(canvas, FontSecondary);

    uint8_t item_position = 0;
    int8_t active_screen = model->position / BUTTONS_PER_SCREEN;
    size_t items_size = ButtonMenuItemArray_size(model->items);
    int8_t max_screen = ((int16_t)items_size - 1) / BUTTONS_PER_SCREEN;
    ButtonMenuItemArray_it_t it;

    if(active_screen > 0) {
        canvas_draw_icon(canvas, 28, 1, &I_InfraredArrowUp_4x8);
    }

    if(max_screen > active_screen) {
        canvas_draw_icon(canvas, 28, 123, &I_InfraredArrowDown_4x8);
    }

    if(model->header) {
        string_t disp_str;
        string_init_set_str(disp_str, model->header);
        elements_string_fit_width(canvas, disp_str, ITEM_WIDTH - 6);
        canvas_draw_str_aligned(
            canvas, 32, 10, AlignCenter, AlignCenter, string_get_cstr(disp_str));
        string_clear(disp_str);
    }

    for(ButtonMenuItemArray_it(it, model->items); !ButtonMenuItemArray_end_p(it);
        ButtonMenuItemArray_next(it), ++item_position) {
        if(active_screen == (item_position / BUTTONS_PER_SCREEN)) {
            if(ButtonMenuItemArray_cref(it)->type == ButtonMenuItemTypeControl) {
                button_menu_draw_control_button(
                    canvas,
                    item_position % BUTTONS_PER_SCREEN,
                    ButtonMenuItemArray_cref(it)->label,
                    (item_position == model->position));
            } else if(ButtonMenuItemArray_cref(it)->type == ButtonMenuItemTypeCommon) {
                button_menu_draw_common_button(
                    canvas,
                    item_position % BUTTONS_PER_SCREEN,
                    ButtonMenuItemArray_cref(it)->label,
                    (item_position == model->position));
            }
        }
    }
}

static void button_menu_process_up(ButtonMenu* button_menu) {
    furi_assert(button_menu);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            if(model->position > 0) {
                model->position--;
            } else {
                model->position = ButtonMenuItemArray_size(model->items) - 1;
            }
            return true;
        });
}

static void button_menu_process_down(ButtonMenu* button_menu) {
    furi_assert(button_menu);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            if(model->position < (ButtonMenuItemArray_size(model->items) - 1)) {
                model->position++;
            } else {
                model->position = 0;
            }
            return true;
        });
}

static void button_menu_process_ok(ButtonMenu* button_menu, InputType type) {
    furi_assert(button_menu);

    ButtonMenuItem* item = NULL;

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            if(model->position < (ButtonMenuItemArray_size(model->items))) {
                item = ButtonMenuItemArray_get(model->items, model->position);
            }
            return false;
        });

    if(item) {
        if(item->type == ButtonMenuItemTypeControl) {
            if(type == InputTypeShort) {
                if(item && item->callback) {
                    item->callback(item->callback_context, item->index, type);
                }
            }
        }
        if(item->type == ButtonMenuItemTypeCommon) {
            if((type == InputTypePress) || (type == InputTypeRelease)) {
                if(item && item->callback) {
                    item->callback(item->callback_context, item->index, type);
                }
            }
        }
    }
}

static bool button_menu_view_input_callback(InputEvent* event, void* context) {
    furi_assert(event);

    ButtonMenu* button_menu = context;
    bool consumed = false;

    if(event->key == InputKeyOk) {
        if((event->type == InputTypeRelease) || (event->type == InputTypePress)) {
            consumed = true;
            button_menu->freeze_input = (event->type == InputTypePress);
            button_menu_process_ok(button_menu, event->type);
        } else if(event->type == InputTypeShort) {
            consumed = true;
            button_menu_process_ok(button_menu, event->type);
        }
    }

    if(!button_menu->freeze_input &&
       ((event->type == InputTypeRepeat) || (event->type == InputTypeShort))) {
        switch(event->key) {
        case InputKeyUp:
            consumed = true;
            button_menu_process_up(button_menu);
            break;
        case InputKeyDown:
            consumed = true;
            button_menu_process_down(button_menu);
            break;
        default:
            break;
        }
    }

    return consumed;
}

View* button_menu_get_view(ButtonMenu* button_menu) {
    furi_assert(button_menu);
    return button_menu->view;
}

void button_menu_reset(ButtonMenu* button_menu) {
    furi_assert(button_menu);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            ButtonMenuItemArray_reset(model->items);
            model->position = 0;
            model->header = NULL;
            return true;
        });
}

void button_menu_set_header(ButtonMenu* button_menu, const char* header) {
    furi_assert(button_menu);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            model->header = header;
            return true;
        });
}

ButtonMenuItem* button_menu_add_item(
    ButtonMenu* button_menu,
    const char* label,
    int32_t index,
    ButtonMenuItemCallback callback,
    ButtonMenuItemType type,
    void* callback_context) {
    ButtonMenuItem* item = NULL;
    furi_assert(label);
    furi_assert(button_menu);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            item = ButtonMenuItemArray_push_new(model->items);
            item->label = label;
            item->index = index;
            item->type = type;
            item->callback = callback;
            item->callback_context = callback_context;
            return true;
        });

    return item;
}

ButtonMenu* button_menu_alloc(void) {
    ButtonMenu* button_menu = malloc(sizeof(ButtonMenu));
    button_menu->view = view_alloc();
    view_set_orientation(button_menu->view, ViewOrientationVertical);
    view_set_context(button_menu->view, button_menu);
    view_allocate_model(button_menu->view, ViewModelTypeLocking, sizeof(ButtonMenuModel));
    view_set_draw_callback(button_menu->view, button_menu_view_draw_callback);
    view_set_input_callback(button_menu->view, button_menu_view_input_callback);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            ButtonMenuItemArray_init(model->items);
            model->position = 0;
            model->header = NULL;
            return true;
        });

    button_menu->freeze_input = false;
    return button_menu;
}

void button_menu_free(ButtonMenu* button_menu) {
    furi_assert(button_menu);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            ButtonMenuItemArray_clear(model->items);
            return true;
        });
    view_free(button_menu->view);
    free(button_menu);
}

void button_menu_set_selected_item(ButtonMenu* button_menu, uint32_t index) {
    furi_assert(button_menu);

    with_view_model(
        button_menu->view, (ButtonMenuModel * model) {
            uint8_t item_position = 0;
            ButtonMenuItemArray_it_t it;
            for(ButtonMenuItemArray_it(it, model->items); !ButtonMenuItemArray_end_p(it);
                ButtonMenuItemArray_next(it), ++item_position) {
                if((uint32_t)ButtonMenuItemArray_cref(it)->index == index) {
                    model->position = item_position;
                    break;
                }
            }
            return true;
        });
}
