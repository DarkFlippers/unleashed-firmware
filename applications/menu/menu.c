#include "menu.h"
#include <cmsis_os.h>
#include <stdio.h>
#include <stdbool.h>

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
    assert(gui);
    gui->add_widget(gui, menu->widget, WidgetLayerFullscreen);

    widget_draw_callback_set(menu->widget, menu_widget_callback, menu_mutex);
    widget_input_callback_set(menu->widget, menu_event_input_callback, menu->event);

    return menu_mutex;
}

void menu_build_main(Menu* menu) {
    assert(menu);
    // Root point
    menu->root = menu_item_alloc_menu(NULL, NULL);

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
    
    Menu* menu = acquire_mutex((ValueMutex*)context, 100); // wait 10 ms to get mutex
    if(menu == NULL) return; // redraw fail

    if(!menu->current) {
        canvas->clear(canvas);
        canvas->set_color(canvas, ColorBlack);
        canvas->set_font(canvas, FontPrimary);
        canvas->draw_str(canvas, 2, 32, "Idle Screen");
    } else {
        MenuItemArray_t* items = menu_item_get_subitems(menu->current);
        canvas->clear(canvas);
        canvas->set_color(canvas, ColorBlack);
        canvas->set_font(canvas, FontSecondary);
        for(size_t i = 0; i < 5; i++) {
            size_t shift_position = i + menu->position + MenuItemArray_size(*items) - 2;
            shift_position = shift_position % (MenuItemArray_size(*items));
            MenuItem* item = *MenuItemArray_get(*items, shift_position);
            canvas->draw_str(canvas, 2, 12 * (i + 1), menu_item_get_label(item));
        }
    }

    release_mutex((ValueMutex*)context, menu);
    
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
    ValueMutex* menu_mutex = menu_init();

    MenuEvent* menu_event = NULL;
    {
        Menu* menu = acquire_mutex_block(menu_mutex);
        assert(menu);

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
