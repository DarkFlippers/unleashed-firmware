#pragma once

#include <furi.h>
#include <gui/gui.h>

typedef struct {
    const char* name; //Name of the menu
    bool enabled; //Is the menu item enabled (it will not render, you cannot select it)

    void (*callback)(
        void* state); //Callback for when the activate_menu is called while this menu is selected
} MenuItem;

typedef struct {
    MenuItem* items; //list of menu items
    uint8_t menu_count; //count of menu items (do not change)
    uint8_t current_menu; //currently selected menu item
    uint8_t menu_width; //width of the menu
    bool enabled; //is the menu enabled (it will not render and accept events when disabled)
} Menu;

/**
 * Cleans up the pointers used by the menu
 *
 * @param menu Pointer of the menu to clean up
 */
void free_menu(Menu* menu);

/**
 * Add a new menu item
 *
 * @param menu      Pointer of the menu
 * @param name      Name of the menu item
 * @param callback  Callback called on activation
 */
void add_menu(Menu* menu, const char* name, void (*callback)(void*));

/**
 * Setting menu item to be enabled/disabled
 *
 * @param menu  Pointer of the menu
 * @param index Menu index to set
 * @param state Enabled (true), Disabled(false)
 */
void set_menu_state(Menu* menu, uint8_t index, bool state);

/**
 * Moves selection up or down
 *
 * @param menu      Pointer of the menu
 * @param direction Direction to move -1 down, 1 up
 */
void move_menu(Menu* menu, int8_t direction);

/**
 * Triggers the current menu callback
 *
 * @param menu  Pointer of the menu
 * @param state Usually your application state
 */
void activate_menu(Menu* menu, void* state);

/**
 * Renders the menu at a coordinate (call it in your render function).
 *
 * Keep in mind that Flipper has a 128x64 pixel screen resolution and the coordinate
 * you give is the menu's rectangle top-left corner (arrows not included).
 * The rectangle height is 10 px, the arrows have a 4 pixel height. Space needed is 18px.
 * The width of the menu can be configured in the menu object.
 *
 *
 * @param menu      Pointer of the menu
 * @param canvas    Flippers Canvas pointer
 * @param pos_x     X position to draw
 * @param pos_y     Y position to draw
 */
void render_menu(Menu* menu, Canvas* canvas, uint8_t pos_x, uint8_t pos_y);