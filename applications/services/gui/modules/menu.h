/**
 * @file menu.h
 * GUI: Menu view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MENU_STYLE_PLUGIN_APP_ID      "MenuStyle"
#define MENU_STYLE_PLUGIN_API_VERSION 1

/** Menu anonymous structure */
typedef struct Menu Menu;

/** Menu Item Callback */
typedef void (*MenuItemCallback)(void* context, uint32_t index);

typedef struct {
    const char* label;
    IconAnimation* icon;
    uint32_t index;
    MenuItemCallback callback;
    void* callback_context;
} MenuItem;

typedef struct MenuStyle MenuStyle;

typedef struct {
    MenuItem* items;
    size_t count;
    size_t position;
    size_t scroll_counter;
    size_t offset;
    const MenuStyle* style;
} MenuModel;

struct MenuStyle {
    void (*draw)(Canvas* canvas, MenuModel* model);
    size_t (*navigate)(MenuModel* model, InputKey key);
};

/** Menu allocation and initialization
 *
 * @return     Menu instance
 */
Menu* menu_alloc(void);

/** Free menu
 *
 * @param      menu  Menu instance
 */
void menu_free(Menu* menu);

/** Get Menu view
 *
 * @param      menu  Menu instance
 *
 * @return     View instance
 */
View* menu_get_view(Menu* menu);

/** Add item to menu
 *
 * @param      menu      Menu instance
 * @param      label     menu item string label
 * @param      icon      IconAnimation instance
 * @param      index     menu item index
 * @param      callback  MenuItemCallback instance
 * @param      context   pointer to context
 */
void menu_add_item(
    Menu* menu,
    const char* label,
    const Icon* icon,
    uint32_t index,
    MenuItemCallback callback,
    void* context);

/** Clean menu
 * @note       this function does not free menu instance
 *
 * @param      menu  Menu instance
 */
void menu_reset(Menu* menu);

/** Set current menu item
 *
 * @param      menu   Menu instance
 * @param      index  The index
 */
void menu_set_selected_item(Menu* menu, uint32_t index);

void menu_set_style(Menu* menu, const MenuStyle* style);

#ifdef __cplusplus
}
#endif
