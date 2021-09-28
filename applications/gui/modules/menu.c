#include "menu.h"

#include <m-array.h>
#include <gui/elements.h>
#include <furi.h>

struct Menu {
    View* view;
};

typedef struct {
    const char* label;
    IconAnimation* icon;
    uint32_t index;
    MenuItemCallback callback;
    void* callback_context;
} MenuItem;

ARRAY_DEF(MenuItemArray, MenuItem, M_POD_OPLIST);

typedef struct {
    MenuItemArray_t items;
    uint8_t position;
} MenuModel;

static void menu_process_up(Menu* menu);
static void menu_process_down(Menu* menu);
static void menu_process_ok(Menu* menu);

static void menu_draw_callback(Canvas* canvas, void* _model) {
    MenuModel* model = _model;

    canvas_clear(canvas);

    uint8_t position = model->position;
    size_t items_count = MenuItemArray_size(model->items);
    if(items_count) {
        MenuItem* item;
        size_t shift_position;
        // First line
        canvas_set_font(canvas, FontSecondary);
        shift_position = (0 + position + items_count - 1) % items_count;
        item = MenuItemArray_get(model->items, shift_position);
        if(item->icon) {
            canvas_draw_icon_animation(canvas, 4, 3, item->icon);
            icon_animation_stop(item->icon);
        }
        canvas_draw_str(canvas, 22, 14, item->label);
        // Second line main
        canvas_set_font(canvas, FontPrimary);
        shift_position = (1 + position + items_count - 1) % items_count;
        item = MenuItemArray_get(model->items, shift_position);
        if(item->icon) {
            canvas_draw_icon_animation(canvas, 4, 25, item->icon);
            icon_animation_start(item->icon);
        }
        canvas_draw_str(canvas, 22, 36, item->label);
        // Third line
        canvas_set_font(canvas, FontSecondary);
        shift_position = (2 + position + items_count - 1) % items_count;
        item = MenuItemArray_get(model->items, shift_position);
        if(item->icon) {
            canvas_draw_icon_animation(canvas, 4, 47, item->icon);
            icon_animation_stop(item->icon);
        }
        canvas_draw_str(canvas, 22, 58, item->label);
        // Frame and scrollbar
        elements_frame(canvas, 0, 21, 128 - 5, 21);
        elements_scrollbar(canvas, position, items_count);
    } else {
        canvas_draw_str(canvas, 2, 32, "Empty");
        elements_scrollbar(canvas, 0, 0);
    }
}

static bool menu_input_callback(InputEvent* event, void* context) {
    Menu* menu = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            consumed = true;
            menu_process_up(menu);
        } else if(event->key == InputKeyDown) {
            consumed = true;
            menu_process_down(menu);
        } else if(event->key == InputKeyOk) {
            consumed = true;
            menu_process_ok(menu);
        }
    }

    return consumed;
}

Menu* menu_alloc() {
    Menu* menu = furi_alloc(sizeof(Menu));
    menu->view = view_alloc(menu->view);
    view_set_context(menu->view, menu);
    view_allocate_model(menu->view, ViewModelTypeLocking, sizeof(MenuModel));
    view_set_draw_callback(menu->view, menu_draw_callback);
    view_set_input_callback(menu->view, menu_input_callback);

    with_view_model(
        menu->view, (MenuModel * model) {
            MenuItemArray_init(model->items);
            model->position = 0;
            return true;
        });

    return menu;
}

void menu_free(Menu* menu) {
    furi_assert(menu);
    with_view_model(
        menu->view, (MenuModel * model) {
            MenuItemArray_clear(model->items);
            return true;
        });
    view_free(menu->view);
    free(menu);
}

View* menu_get_view(Menu* menu) {
    furi_assert(menu);
    return (menu->view);
}

void menu_add_item(
    Menu* menu,
    const char* label,
    IconAnimation* icon,
    uint32_t index,
    MenuItemCallback callback,
    void* context) {
    furi_assert(menu);
    furi_assert(label);

    MenuItem* item = NULL;
    with_view_model(
        menu->view, (MenuModel * model) {
            item = MenuItemArray_push_new(model->items);
            item->label = label;
            item->icon = icon;
            item->index = index;
            item->callback = callback;
            item->callback_context = context;
            return true;
        });
}

void menu_clean(Menu* menu) {
    furi_assert(menu);
    with_view_model(
        menu->view, (MenuModel * model) {
            MenuItemArray_clean(model->items);
            model->position = 0;
            return true;
        });
}

static void menu_process_up(Menu* menu) {
    with_view_model(
        menu->view, (MenuModel * model) {
            if(model->position > 0) {
                model->position--;
            } else {
                model->position = MenuItemArray_size(model->items) - 1;
            }
            return true;
        });
}

static void menu_process_down(Menu* menu) {
    with_view_model(
        menu->view, (MenuModel * model) {
            if(model->position < MenuItemArray_size(model->items) - 1) {
                model->position++;
            } else {
                model->position = 0;
            }
            return true;
        });
}

static void menu_process_ok(Menu* menu) {
    MenuItem* item = NULL;
    with_view_model(
        menu->view, (MenuModel * model) {
            if(model->position < MenuItemArray_size(model->items)) {
                item = MenuItemArray_get(model->items, model->position);
            }
            return true;
        });
    if(item && item->callback) {
        item->callback(item->callback_context, item->index);
    }
}
