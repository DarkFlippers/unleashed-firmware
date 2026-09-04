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
 * @note       Bump on any change to MenuItem, MenuModel or MenuStyle - field order, field size,
 *             the meaning of a field - or to the preconditions the callbacks below document.
 *             Nothing else catches a stale style: the SDK version in a .fal is compared on its
 *             major part only, and a layout change moves no symbol in api_symbols.csv. Appending
 *             to MenuStyle is the worst case, since the plugin owns that object: firmware reading
 *             a slot the plugin never wrote branches into whatever follows it in .rodata.
 */
#define MENU_STYLE_PLUGIN_API_VERSION 1

/** Menu anonymous structure */
typedef struct Menu Menu;

/** Menu Item Callback */
typedef void (*MenuItemCallback)(void* context, uint32_t index);

/** One menu entry, as added by menu_add_item()
 *
 * @note       An application including this header cannot declare a MenuItem of its own
 */
typedef struct {
    const char* label; /**< Label, not copied - must outlive the menu */
    IconAnimation* icon; /**< Icon, never NULL - menu_add_item() substitutes a default */
    uint32_t index; /**< Value handed back to the callback */
    MenuItemCallback callback; /**< Invoked on OK, may be NULL */
    void* callback_context; /**< First argument of the callback */
} MenuItem;

/** Menu style anonymous structure */
typedef struct MenuStyle MenuStyle;

/** Menu view model, passed to both MenuStyle callbacks */
typedef struct {
    MenuItem* items; /**< Item array, count entries long */
    size_t count; /**< Number of items */
    size_t position; /**< Selected item, < count whenever a style callback runs */
    size_t scroll_counter; /**< Ticks since the selection changed while a style is active */
    size_t offset; /**< Style scratch, shared by draw() and navigate(). Cleared when the style
                        changes or the menu is reset, but not when items are added - check it
                        against count before use */
    const MenuStyle* style; /**< Active style, NULL for the built-in list */
} MenuModel;

/** Menu style: how a menu draws itself and how the D-pad moves through it
 *
 * Styles ship as MENU_STYLE_PLUGIN_APP_ID plugins; the loader picks one up from its plugin
 * directory (LOADER_MENU_STYLES_PATH, declared in loader/loader.h) and installs it here.
 *
 * Both callbacks run with the view model mutex held, on different threads - draw() on the GUI
 * service thread, navigate() on the one running the ViewDispatcher. Neither may call back into
 * menu_*(): that mutex is recursive, so re-entering does not deadlock, it corrupts - menu_reset()
 * frees and menu_add_item() reallocates the very item array the callback is walking. Neither
 * should wait on another service either, since draw() holds up the whole GUI while it runs.
 */
struct MenuStyle {
    /** Draw the menu
     *
     * Called only with a non-empty menu and position < count, so dividing by count and indexing
     * items[position] are both safe. The canvas is cleared, covers the viewport, and is already
     * oriented for the current hand orientation.
     *
     * A style drawing rotated switches the canvas orientation and leaves it switched, because
     * canvas_commit() tags the streamed RPC frame with whatever the last draw callback left, and
     * the orientation is re-established before every viewport draw regardless. That holds only on
     * a fullscreen layer, where nothing is drawn after; the ViewPort, not the style, should
     * really own this, which needs the orientation to become part of MenuStyle.
     *
     * @param      canvas  Canvas to draw on
     * @param      model   Menu model, of which only offset may be written
     */
    void (*draw)(Canvas* canvas, MenuModel* model);

    /** Move the selection
     *
     * Called under the same count and position guarantees as draw(), and only for the four
     * direction keys. All four are consumed whether or not the style acts on them, so a style
     * cannot decline one and let it through.
     *
     * @param      model  Menu model, of which only offset may be written
     * @param      key    InputKeyUp, InputKeyDown, InputKeyLeft or InputKeyRight
     * @return     Absolute index of the item to select, or model->position to stay put. Values
     *             outside the menu are silently ignored, so clamp rather than returning a
     *             position that does not exist.
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
 * @note       safe to call while the menu is on screen
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
 * Safe to call from another thread while the menu is on screen: the view model mutex orders the
 * swap against both style callbacks, so once this returns the old vtable is no longer in use and
 * the plugin holding it can be unloaded.
 *
 * @param      menu   Menu instance
 * @param      style  Style to use, or NULL for the built-in list. Kept by pointer, not copied, so
 *                    it must outlive the menu or be replaced before it goes away.
 */
void menu_set_style(Menu* menu, const MenuStyle* style);

#ifdef __cplusplus
}
#endif
