#pragma once

#include <stdint.h>
#include <m-array.h>
#include <gui/icon.h>

typedef enum {
    MenuItemTypeMenu = 0x00,
    MenuItemTypeFunction = 0x01,
} MenuItemType;

typedef struct MenuItem MenuItem;
typedef void (*MenuItemCallback)(void* context);

ARRAY_DEF(MenuItemArray, MenuItem*, M_PTR_OPLIST);

MenuItem* menu_item_alloc_menu(const char* label, Icon* icon);

MenuItem*
menu_item_alloc_function(const char* label, Icon* icon, MenuItemCallback callback, void* context);

void menu_item_release(MenuItem* menu_item);

MenuItem* menu_item_get_parent(MenuItem* menu_item);

void menu_item_subitem_add(MenuItem* menu_item, MenuItem* sub_item);

MenuItemType menu_item_get_type(MenuItem* menu_item);

void menu_item_set_position(MenuItem* menu_item, size_t position);
size_t menu_item_get_position(MenuItem* menu_item);

void menu_item_set_label(MenuItem* menu_item, const char* label);
const char* menu_item_get_label(MenuItem* menu_item);

void menu_item_set_icon(MenuItem* menu_item, Icon* icon);
Icon* menu_item_get_icon(MenuItem* menu_item);

MenuItemArray_t* menu_item_get_subitems(MenuItem* menu_item);

void menu_item_function_call(MenuItem* menu_item);
