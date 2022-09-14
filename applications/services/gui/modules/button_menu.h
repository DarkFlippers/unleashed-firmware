/**
 * @file button_menu.h
 * GUI: ButtonMenu view module API
 */

#pragma once

#include <stdint.h>
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ButtonMenu anonymous structure */
typedef struct ButtonMenu ButtonMenu;

/** ButtonMenuItem anonymous structure */
typedef struct ButtonMenuItem ButtonMenuItem;

/** Callback for any button menu actions */
typedef void (*ButtonMenuItemCallback)(void* context, int32_t index, InputType type);

/** Type of button. Difference in drawing buttons. */
typedef enum {
    ButtonMenuItemTypeCommon,
    ButtonMenuItemTypeControl,
} ButtonMenuItemType;

/** Get button menu view
 *
 * @param      button_menu  ButtonMenu instance
 *
 * @return     View instance that can be used for embedding
 */
View* button_menu_get_view(ButtonMenu* button_menu);

/** Clean button menu
 *
 * @param      button_menu  ButtonMenu instance
 */
void button_menu_reset(ButtonMenu* button_menu);

/** Add item to button menu instance
 *
 * @param      button_menu       ButtonMenu instance
 * @param      label             text inside new button
 * @param      index             value to distinct between buttons inside
 *                               ButtonMenuItemCallback
 * @param      callback          The callback
 * @param      type              type of button to create. Differ by button
 *                               drawing. Control buttons have no frames, and
 *                               have more squared borders.
 * @param      callback_context  The callback context
 *
 * @return     pointer to just-created item
 */
ButtonMenuItem* button_menu_add_item(
    ButtonMenu* button_menu,
    const char* label,
    int32_t index,
    ButtonMenuItemCallback callback,
    ButtonMenuItemType type,
    void* callback_context);

/** Allocate and initialize new instance of ButtonMenu model
 *
 * @return     just-created ButtonMenu model
 */
ButtonMenu* button_menu_alloc(void);

/** Free ButtonMenu element
 *
 * @param      button_menu  ButtonMenu instance
 */
void button_menu_free(ButtonMenu* button_menu);

/** Set ButtonMenu header on top of canvas
 *
 * @param      button_menu  ButtonMenu instance
 * @param      header       header on the top of button menu
 */
void button_menu_set_header(ButtonMenu* button_menu, const char* header);

/** Set selected item
 *
 * @param      button_menu  ButtonMenu instance
 * @param      index        index of ButtonMenu to be selected
 */
void button_menu_set_selected_item(ButtonMenu* button_menu, uint32_t index);

#ifdef __cplusplus
}
#endif
