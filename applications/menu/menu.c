#include "menu.h"
#include <cmsis_os.h>
#include <stdio.h>
#include <stdbool.h>

#include <flipper.h>
#include <flipper_v2.h>
#include <gui/gui.h>

#include "menu_event.h"
#include "menu_item.h"

struct Menu {
    MenuEvent* event;

    // GUI
    Widget* widget;

    // State
    MenuItem* root;
    MenuItem* settings;
    MenuItem* current;
    uint32_t position;
};

void menu_widget_callback(CanvasApi* canvas, void* context);

Menu* menu_init() {
    Menu* menu = furi_alloc(sizeof(Menu));

    // Event dispatcher
    menu->event = menu_event_alloc();

    // Allocate and configure widget
    menu->widget = widget_alloc();
    widget_draw_callback_set(menu->widget, menu_widget_callback, menu);
    widget_input_callback_set(menu->widget, menu_event_input_callback, menu->event);

    // Open GUI and register fullscreen widget
    GuiApi* gui = furi_open("gui");
    assert(gui);
    gui->add_widget(gui, menu->widget, WidgetLayerFullscreen);

    return menu;
}

void menu_build_main(Menu* menu) {
    assert(menu);
    // Root point
    menu->root = menu_item_alloc_menu(NULL, NULL);

    menu_item_add(menu, menu_item_alloc_function("Sub 1 gHz", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("125 kHz RFID", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Infrared", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("I-Button", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("USB", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Bluetooth", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("GPIO / HW", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("U2F", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Tamagotchi", NULL, NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Plugins", NULL, NULL, NULL));

    menu->settings = menu_item_alloc_menu("Setting", NULL);
    menu_item_subitem_add(menu->settings, menu_item_alloc_function("one", NULL, NULL, NULL));
    menu_item_subitem_add(menu->settings, menu_item_alloc_function("two", NULL, NULL, NULL));
    menu_item_subitem_add(menu->settings, menu_item_alloc_function("three", NULL, NULL, NULL));

    menu_item_add(menu, menu->settings);
}

void menu_item_add(Menu* menu, MenuItem* item) {
    menu_item_subitem_add(menu->root, item);
}

void menu_settings_item_add(Menu* menu, MenuItem* item) {
    menu_item_subitem_add(menu->settings, item);
}

void menu_widget_callback(CanvasApi* canvas, void* context) {
    assert(canvas);
    assert(context);

    Menu* menu = context;

    menu_event_lock(menu->event);

    if(!menu->current) {
        canvas->clear(canvas);
        canvas->set_color(canvas, ColorBlack);
        canvas->set_font(canvas, canvas->fonts->primary);
        canvas->draw_str(canvas, 2, 32, "Idle Screen");
    } else {
        MenuItemArray_t* items = menu_item_get_subitems(menu->current);
        canvas->clear(canvas);
        canvas->set_color(canvas, ColorBlack);
        canvas->set_font(canvas, canvas->fonts->secondary);
        for(size_t i = 0; i < 5; i++) {
            size_t shift_position = i + menu->position + MenuItemArray_size(*items) - 2;
            shift_position = shift_position % (MenuItemArray_size(*items));
            MenuItem* item = *MenuItemArray_get(*items, shift_position);
            canvas->draw_str(canvas, 2, 12 * (i + 1), menu_item_get_label(item));
        }
    }

    menu_event_unlock(menu->event);
}

void menu_update(Menu* menu) {
    assert(menu);

    menu_event_activity_notify(menu->event);
    widget_update(menu->widget);
}

void menu_up(Menu* menu) {
    assert(menu);

    MenuItemArray_t* items = menu_item_get_subitems(menu->current);
    if(menu->position == 0) menu->position = MenuItemArray_size(*items);
    menu->position--;
    menu_update(menu);
}

void menu_down(Menu* menu) {
    assert(menu);

    MenuItemArray_t* items = menu_item_get_subitems(menu->current);
    menu->position++;
    menu->position = menu->position % MenuItemArray_size(*items);
    menu_update(menu);
}

void menu_ok(Menu* menu) {
    assert(menu);

    if(!menu->current) {
        menu->current = menu->root;
        menu_update(menu);
        return;
    }

    MenuItemArray_t* items = menu_item_get_subitems(menu->current);
    MenuItem* item = *MenuItemArray_get(*items, menu->position);
    MenuItemType type = menu_item_get_type(item);

    if(type == MenuItemTypeMenu) {
        menu->current = item;
        menu->position = 0;
        menu_update(menu);
    } else if(type == MenuItemTypeFunction) {
        menu_item_function_call(item);
    }
}

void menu_back(Menu* menu) {
    assert(menu);
    MenuItem* parent = menu_item_get_parent(menu->current);
    if(parent) {
        menu->current = parent;
        menu->position = 0;
        menu_update(menu);
    } else {
        menu_exit(menu);
    }
}

void menu_exit(Menu* menu) {
    assert(menu);
    menu->position = 0;
    menu->current = NULL;
    menu_update(menu);
}

void menu_task(void* p) {
    Menu* menu = menu_init();
    menu_build_main(menu);

    if(!furi_create_deprecated("menu", menu, sizeof(menu))) {
        printf("[menu_task] cannot create the menu record\n");
        furiac_exit(NULL);
    }

    furiac_ready();

    while(1) {
        MenuMessage m = menu_event_next(menu->event);

        if(!menu->current && m.type != MenuMessageTypeOk) {
            continue;
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
    }
}
