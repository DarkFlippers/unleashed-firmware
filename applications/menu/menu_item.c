#include "menu_item.h"
#include <stdlib.h>
#include <string.h>
#include <furi.h>

struct MenuItem {
    MenuItemType type;

    const char* label;
    Icon* icon;

    size_t position;
    MenuItem* parent;
    void* data;

    // callback related
    MenuItemCallback callback;
    void* callback_context;
};

MenuItem* menu_item_alloc() {
    MenuItem* menu_item = furi_alloc(sizeof(MenuItem));
    return menu_item;
}

MenuItem* menu_item_alloc_menu(const char* label, Icon* icon) {
    MenuItem* menu_item = menu_item_alloc();

    menu_item->type = MenuItemTypeMenu;
    menu_item->label = label;
    menu_item->icon = icon;

    MenuItemArray_t* items = furi_alloc(sizeof(MenuItemArray_t));
    MenuItemArray_init(*items);
    menu_item->data = items;

    return menu_item;
}

MenuItem*
menu_item_alloc_function(const char* label, Icon* icon, MenuItemCallback callback, void* context) {
    MenuItem* menu_item = menu_item_alloc();

    menu_item->type = MenuItemTypeFunction;
    menu_item->label = label;
    menu_item->icon = icon;
    menu_item->callback = callback;
    menu_item->callback_context = context;

    return menu_item;
}

void menu_item_release(MenuItem* menu_item) {
    furi_assert(menu_item);
    if(menu_item->type == MenuItemTypeMenu) {
        //TODO: iterate and release
        free(menu_item->data);
    }
    free(menu_item);
}

MenuItem* menu_item_get_parent(MenuItem* menu_item) {
    furi_assert(menu_item);
    return menu_item->parent;
}

void menu_item_subitem_add(MenuItem* menu_item, MenuItem* sub_item) {
    furi_assert(menu_item);
    furi_check(menu_item->type == MenuItemTypeMenu);
    MenuItemArray_t* items = menu_item->data;
    sub_item->parent = menu_item;
    MenuItemArray_push_back(*items, sub_item);
}

uint8_t menu_item_get_type(MenuItem* menu_item) {
    furi_assert(menu_item);
    return menu_item->type;
}

void menu_item_set_position(MenuItem* menu_item, size_t position) {
    furi_assert(menu_item);
    menu_item->position = position;
}

size_t menu_item_get_position(MenuItem* menu_item) {
    furi_assert(menu_item);
    return menu_item->position;
}

void menu_item_set_label(MenuItem* menu_item, const char* label) {
    furi_assert(menu_item);
    menu_item->label = label;
}

const char* menu_item_get_label(MenuItem* menu_item) {
    furi_assert(menu_item);
    return menu_item->label;
}

void menu_item_set_icon(MenuItem* menu_item, Icon* icon) {
    furi_assert(menu_item);
    menu_item->icon = icon;
}

Icon* menu_item_get_icon(MenuItem* menu_item) {
    furi_assert(menu_item);
    return menu_item->icon;
}

MenuItemArray_t* menu_item_get_subitems(MenuItem* menu_item) {
    furi_assert(menu_item);
    furi_check(menu_item->type == MenuItemTypeMenu);
    return menu_item->data;
}

void menu_item_function_call(MenuItem* menu_item) {
    furi_assert(menu_item);
    furi_check(menu_item->type == MenuItemTypeFunction);
    if(menu_item->callback) menu_item->callback(menu_item->callback_context);
}
