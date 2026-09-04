/**
 * @file menu.h
 * GUI: Menu view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Application id a menu style plugin must declare in its FlipperAppPluginDescriptor */
#define MENU_STYLE_PLUGIN_APP_ID "MenuStyle"

/** Menu style plugin ABI version
 *
 * @note       Bump this on any change to MenuItem, MenuModel or MenuStyle - field order, field
 *             size or the meaning of a field. Nothing else catches a stale style: the SDK version
 *             recorded in a .fal is only compared on its major part, so a plugin built against an
 *             older layout would load and read the model at the wrong offsets. PluginManager
 *             refuses a mismatch of this number, and that is the only guard there is.
 */
#define MENU_STYLE_PLUGIN_API_VERSION 1

/** Menu anonymous structure */
typedef struct Menu Menu;

/** Menu Item Callback */
typedef void (*MenuItemCallback)(void* context, uint32_t index);

/** One menu entry, as added by menu_add_item()
 *
 * @warning    This name is generic and lives in the public SDK namespace: an application that
 *             includes this header cannot also declare a MenuItem of its own.
 */
typedef struct {
    const char* label; /**< Label, not copied - must outlive the menu */
    IconAnimation* icon; /**< Icon, animated while the item is selected */
    uint32_t index; /**< Value handed back to the callback */
    MenuItemCallback callback; /**< Invoked on OK */
    void* callback_context; /**< First argument of the callback */
} MenuItem;

/** Menu style anonymous structure */
typedef struct MenuStyle MenuStyle;

/** Menu view model, passed to both MenuStyle callbacks */
typedef struct {
    MenuItem* items; /**< Item array, count entries long */
    size_t count; /**< Number of items */
    size_t position; /**< Selected item, always < count */
    size_t scroll_counter; /**< 3 Hz ticks since the selection changed, drives label scrolling */
    size_t offset; /**< Scratch owned by the style, cleared when the style changes */
    const MenuStyle* style; /**< Active style, NULL for the built-in list */
} MenuModel;

/** Menu style: how a menu draws itself and how the D-pad moves through it
 *
 * Styles ship as MENU_STYLE_PLUGIN_APP_ID plugins; the loader picks one up from
 * LOADER_MENU_STYLES_PATH and installs it with menu_set_style().
 *
 * Both callbacks run with the view model mutex held, so neither may block or re-enter any menu_*()
 * function. draw() is called from the GUI service thread, navigate() from the thread owning the
 * ViewDispatcher, and scroll_counter is advanced from the FreeRTOS timer daemon.
 */
struct MenuStyle {
    /** Draw the menu
     *
     * Only called when count is non-zero, so dividing by count is safe. The canvas is cleared,
     * full screen, and already oriented for the current hand orientation. A style may change the
     * orientation for its own layout but must not restore it afterwards: canvas_commit() reads the
     * orientation left by the last draw callback to tag the frame for RPC screen streaming, and
     * the GUI re-establishes it at the start of every frame regardless.
     *
     * @param      canvas  Canvas to draw on
     * @param      model   Menu model, of which only offset may be written
     */
    void (*draw)(Canvas* canvas, MenuModel* model);

    /** Move the selection
     *
     * @param      model  Menu model, of which only offset may be written
     * @param      key    One of InputKeyUp, InputKeyDown, InputKeyLeft, InputKeyRight
     * @return     Absolute index of the item to select. Values outside the menu are silently
     *             ignored, so clamp rather than returning a position that does not exist.
     */
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

/** Set menu style
 *
 * Safe to call from another thread while the menu is on screen - the view model mutex orders the
 * swap against the draw callback.
 *
 * @param      menu   Menu instance
 * @param      style  Style to use, or NULL for the built-in list. Kept by pointer, not copied, so
 *                    it must outlive the menu or be replaced before it goes away.
 */
void menu_set_style(Menu* menu, const MenuStyle* style);

#ifdef __cplusplus
}
#endif
