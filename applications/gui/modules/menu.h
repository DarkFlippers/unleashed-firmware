#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Menu anonymous structure */
typedef struct Menu Menu;
typedef void (*MenuItemCallback)(void* context, uint32_t index);

/** Menu allocation and initialization
 * @return Menu instance
 */
Menu* menu_alloc();

/** Free menu
 * @param menu - Menu instance
 */
void menu_free(Menu* menu);

/** Get Menu view
 * @param menu - Menu instance
 * @return View instance
 */
View* menu_get_view(Menu* menu);

/** Add item to menu
 * @param menu - Menu instance
 * @param label - menu item string label
 * @param icon - IconAnimation instance
 * @param index - menu item index
 * @param callback - MenuItemCallback instance
 * @param context - pointer to context
 */
void menu_add_item(
    Menu* menu,
    const char* label,
    IconAnimation* icon,
    uint32_t index,
    MenuItemCallback callback,
    void* context);

/** Clean menu
 * Note: this function does not free menu instance
 * @param menu - Menu instance
 */
void menu_clean(Menu* menu);

#ifdef __cplusplus
}
#endif
