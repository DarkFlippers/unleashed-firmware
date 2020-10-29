#include "menu.h"
#include <cmsis_os.h>
#include <stdio.h>
#include <stdbool.h>

#include <flipper_v2.h>
#include <gui/gui.h>
#include <gui/elements.h>

#include "menu_event.h"
#include "menu_item.h"
#include <assets_icons.h>

struct Menu {
    MenuEvent* event;

    // GUI
    Widget* widget;
    Icon* icon;

    // State
    MenuItem* root;
    MenuItem* settings;
    MenuItem* current;
};

void menu_widget_callback(CanvasApi* canvas, void* context);

ValueMutex* menu_init() {
    Menu* menu = furi_alloc(sizeof(Menu));

    // Event dispatcher
    menu->event = menu_event_alloc();

    ValueMutex* menu_mutex = furi_alloc(sizeof(ValueMutex));
    if(menu_mutex == NULL || !init_mutex(menu_mutex, menu, sizeof(Menu))) {
        printf("[menu_task] cannot create menu mutex\n");
        furiac_exit(NULL);
    }

    // Allocate and configure widget
    menu->widget = widget_alloc();

    // Open GUI and register fullscreen widget
    GuiApi* gui = furi_open("gui");
    furi_check(gui);
    gui->add_widget(gui, menu->widget, GuiLayerFullscreen);

    widget_enabled_set(menu->widget, false);
    widget_draw_callback_set(menu->widget, menu_widget_callback, menu_mutex);
    widget_input_callback_set(menu->widget, menu_event_input_callback, menu->event);

    return menu_mutex;
}

void menu_build_main(Menu* menu) {
    furi_assert(menu);
    // Root point
    menu->root = menu_item_alloc_menu(NULL, NULL);

    Icon* icon = assets_icons_get(A_Bluetooth_14);
    menu->settings = menu_item_alloc_menu("Setting", icon);

    menu_item_add(menu, menu->settings);
}

void menu_item_add(Menu* menu, MenuItem* item) {
    menu_item_subitem_add(menu->root, item);
}

void menu_settings_item_add(Menu* menu, MenuItem* item) {
    menu_item_subitem_add(menu->settings, item);
}

void menu_draw_primary(Menu* menu, CanvasApi* canvas) {
}

void menu_draw_secondary(Menu* menu, CanvasApi* canvas) {
}

void menu_widget_callback(CanvasApi* canvas, void* context) {
    furi_assert(canvas);
    furi_assert(context);

    Menu* menu = acquire_mutex((ValueMutex*)context, 100); // wait 10 ms to get mutex
    if(menu == NULL) return; // redraw fail

    furi_assert(menu->current);

    canvas->clear(canvas);
    canvas->set_color(canvas, ColorBlack);

    size_t position = menu_item_get_position(menu->current);
    MenuItemArray_t* items = menu_item_get_subitems(menu->current);
    size_t items_count = MenuItemArray_size(*items);
    if(items_count) {
        MenuItem* item;
        size_t shift_position;
        // First line
        canvas->set_font(canvas, FontSecondary);
        shift_position = (0 + position + items_count - 1) % (MenuItemArray_size(*items));
        item = *MenuItemArray_get(*items, shift_position);
        canvas->draw_icon(canvas, 4, 3, menu_item_get_icon(item));
        canvas->draw_str(canvas, 22, 14, menu_item_get_label(item));
        // Second line main
        canvas->set_font(canvas, FontPrimary);
        shift_position = (1 + position + items_count - 1) % (MenuItemArray_size(*items));
        item = *MenuItemArray_get(*items, shift_position);
        canvas->draw_icon(canvas, 4, 25, menu_item_get_icon(item));
        canvas->draw_str(canvas, 22, 36, menu_item_get_label(item));
        // Third line
        canvas->set_font(canvas, FontSecondary);
        shift_position = (2 + position + items_count - 1) % (MenuItemArray_size(*items));
        item = *MenuItemArray_get(*items, shift_position);
        canvas->draw_icon(canvas, 4, 47, menu_item_get_icon(item));
        canvas->draw_str(canvas, 22, 58, menu_item_get_label(item));
        // Frame and scrollbar
        // elements_frame(canvas, 0, 0, 128 - 5, 21);
        elements_frame(canvas, 0, 21, 128 - 5, 21);
        // elements_frame(canvas, 0, 42, 128 - 5, 21);
        elements_scrollbar(canvas, position, items_count);
    } else {
        canvas->draw_str(canvas, 2, 32, "Empty");
        elements_scrollbar(canvas, 0, 0);
    }

    release_mutex((ValueMutex*)context, menu);
}

void menu_set_icon(Menu* menu, Icon* icon) {
    assert(menu);

    if(menu->icon) {
        icon_stop_animation(menu->icon);
    }

    menu->icon = icon;

    if(menu->icon) {
        icon_start_animation(menu->icon);
    }
}

void menu_update(Menu* menu) {
    furi_assert(menu);

    if(menu->current) {
        size_t position = menu_item_get_position(menu->current);
        MenuItemArray_t* items = menu_item_get_subitems(menu->current);
        size_t items_count = MenuItemArray_size(*items);
        if(items_count) {
            MenuItem* item = *MenuItemArray_get(*items, position);
            menu_set_icon(menu, menu_item_get_icon(item));
        }
    }

    menu_event_activity_notify(menu->event);
    widget_update(menu->widget);
}

void menu_up(Menu* menu) {
    furi_assert(menu);

    size_t position = menu_item_get_position(menu->current);
    MenuItemArray_t* items = menu_item_get_subitems(menu->current);
    if(position == 0) position = MenuItemArray_size(*items);
    position--;
    menu_item_set_position(menu->current, position);
    menu_update(menu);
}

void menu_down(Menu* menu) {
    furi_assert(menu);
    size_t position = menu_item_get_position(menu->current);
    MenuItemArray_t* items = menu_item_get_subitems(menu->current);
    position++;
    position = position % MenuItemArray_size(*items);
    menu_item_set_position(menu->current, position);
    menu_update(menu);
}

void menu_ok(Menu* menu) {
    furi_assert(menu);

    if(!menu->current) {
        widget_enabled_set(menu->widget, true);
        menu->current = menu->root;
        menu_item_set_position(menu->current, 0);
        menu_update(menu);
        return;
    }

    MenuItemArray_t* items = menu_item_get_subitems(menu->current);
    if(!items || MenuItemArray_size(*items) == 0) {
        return;
    }

    size_t position = menu_item_get_position(menu->current);
    MenuItem* item = *MenuItemArray_get(*items, position);
    MenuItemType type = menu_item_get_type(item);

    if(type == MenuItemTypeMenu) {
        menu->current = item;
        menu_item_set_position(menu->current, 0);
        menu_update(menu);
    } else if(type == MenuItemTypeFunction) {
        menu_item_function_call(item);
    }
}

void menu_back(Menu* menu) {
    furi_assert(menu);
    MenuItem* parent = menu_item_get_parent(menu->current);
    if(parent) {
        menu->current = parent;
        menu_update(menu);
    } else {
        menu_exit(menu);
    }
}

void menu_exit(Menu* menu) {
    furi_assert(menu);
    widget_enabled_set(menu->widget, false);
    menu->current = NULL;
    menu_update(menu);
}

void menu_task(void* p) {
    ValueMutex* menu_mutex = menu_init();

    MenuEvent* menu_event = NULL;
    {
        Menu* menu = acquire_mutex_block(menu_mutex);
        furi_check(menu);

        menu_build_main(menu);

        // immutable thread-safe object
        menu_event = menu->event;

        release_mutex(menu_mutex, menu);
    }

    if(!furi_create("menu", menu_mutex)) {
        printf("[menu_task] cannot create the menu record\n");
        furiac_exit(NULL);
    }

    furiac_ready();

    while(1) {
        MenuMessage m = menu_event_next(menu_event);

        Menu* menu = acquire_mutex_block(menu_mutex);

        if(!menu->current && m.type != MenuMessageTypeOk) {
        } else if(m.type == MenuMessageTypeUp) {
            menu_up(menu);
        } else if(m.type == MenuMessageTypeDown) {
            menu_down(menu);
        } else if(m.type == MenuMessageTypeOk) {
            menu_ok(menu);
        } else if(m.type == MenuMessageTypeLeft) {
            menu_back(menu);
        } else if(m.type == MenuMessageTypeRight) {
            menu_ok(menu);
        } else if(m.type == MenuMessageTypeBack) {
            menu_back(menu);
        } else if(m.type == MenuMessageTypeIdle) {
            menu_exit(menu);
        } else {
            // TODO: fail somehow?
        }

        release_mutex(menu_mutex, menu);
    }
}
