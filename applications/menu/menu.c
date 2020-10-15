#include "menu.h"
#include <cmsis_os.h>
#include <stdio.h>
#include <stdbool.h>

#include <flipper.h>
#include <gui/gui.h>
#include <gui/canvas.h>

#include "menu_event.h"
#include "menu_item.h"

struct Menu {
    MenuEvent* event;
    // GUI
    FuriRecordSubscriber* gui_record;
    Widget* widget;
    // State
    MenuItem* root;
    MenuItem* settings;
    MenuItem* current;
    uint32_t position;
};

void menu_widget_callback(Canvas* canvas, void* context);

Menu* menu_alloc() {
    Menu* menu = furi_alloc(sizeof(Menu));

    // Event dispatcher
    menu->event = menu_event_alloc();

    // Allocate and configure widget
    menu->widget = widget_alloc();
    widget_draw_callback_set(menu->widget, menu_widget_callback, menu);
    widget_input_callback_set(menu->widget, menu_event_input_callback, menu->event);

    // Open GUI and register fullscreen widget
    menu->gui_record = furi_open("gui");
    assert(menu->gui_record);

    return menu;
}

void menu_build_main(Menu* menu) {
    assert(menu);
    // Root point
    menu->root = menu_item_alloc_menu(NULL, NULL);

    menu_item_add(menu, menu_item_alloc_function("Sub 1 gHz", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("125 kHz RFID", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Infrared", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("I-Button", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("USB", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Bluetooth", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("GPIO / HW", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("NFC", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("U2F", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Tamagotchi", NULL, NULL));
    menu_item_add(menu, menu_item_alloc_function("Plugins", NULL, NULL));

    menu->settings = menu_item_alloc_menu("Setting", NULL);
    menu_item_subitem_add(menu->settings, menu_item_alloc_function("one", NULL, NULL));
    menu_item_subitem_add(menu->settings, menu_item_alloc_function("two", NULL, NULL));
    menu_item_subitem_add(menu->settings, menu_item_alloc_function("three", NULL, NULL));

    menu_item_add(menu, menu->settings);
}

void menu_item_add(Menu* menu, MenuItem* item) {
    menu_item_subitem_add(menu->root, item);
}

void menu_settings_item_add(Menu* menu, MenuItem* item) {
    menu_item_subitem_add(menu->settings, item);
}

void menu_widget_callback(Canvas* canvas, void* context) {
    assert(canvas);
    assert(context);

    Menu* menu = context;

    menu_event_lock(menu->event);

    if(!menu->current) {
        canvas_clear(canvas);
        canvas_color_set(canvas, COLOR_BLACK);
        canvas_font_set(canvas, CANVAS_FONT_PRIMARY);
        canvas_str_draw(canvas, 2, 32, "Idle Screen");
    } else {
        MenuItemArray_t* items = menu_item_get_subitems(menu->current);
        canvas_clear(canvas);
        canvas_color_set(canvas, COLOR_BLACK);
        canvas_font_set(canvas, CANVAS_FONT_SECONDARY);
        for(size_t i = 0; i < 5; i++) {
            size_t shift_position = i + menu->position + MenuItemArray_size(*items) - 2;
            shift_position = shift_position % (MenuItemArray_size(*items));
            MenuItem* item = *MenuItemArray_get(*items, shift_position);
            canvas_str_draw(canvas, 2, 12 * (i + 1), menu_item_get_label(item));
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
        MenuItemCallback function = menu_item_get_function(item);
        if(function) function();
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
    Menu* menu = menu_alloc();
    menu_build_main(menu);

    // Register widget
    GUI* gui = furi_take(menu->gui_record);
    assert(gui);
    gui_widget_fs_add(gui, menu->widget);
    furi_commit(menu->gui_record);

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
