#pragma once

#include "menu/menu_item.h"

typedef struct Menu Menu;
typedef struct MenuItem MenuItem;

// Add menu item to root menu
void menu_item_add(Menu* menu, MenuItem* item);

// Add menu item to settings menu
void menu_settings_item_add(Menu* menu, MenuItem* item);

// Menu controls
void menu_up(Menu* menu);
void menu_down(Menu* menu);
void menu_ok(Menu* menu);
void menu_back(Menu* menu);
void menu_exit(Menu* menu);
