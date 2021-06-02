#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Submenu anonymous structure */
typedef struct Submenu Submenu;
typedef struct SubmenuItem SubmenuItem;
typedef void (*SubmenuItemCallback)(void* context, uint32_t index);

/**
 * @brief Allocate and initialize submenu
 * This submenu is used to select one option
 */
Submenu* submenu_alloc();

/**
 * @brief Deinitialize and free submenu
 * @param submenu - Submenu instance
 */
void submenu_free(Submenu* submenu);

/**
 * @brief Get submenu view
 * @param submenu - Submenu instance
 * @return View instance that can be used for embedding
 */
View* submenu_get_view(Submenu* submenu);

/**
 * @brief Add item to submenu
 * @param submenu - Submenu instance
 * @param label - menu item label
 * @param index - menu item index, used for callback, may be the same with other items
 * @param callback - menu item callback
 * @param callback_context - menu item callback context
 * @return SubmenuItem instance that can be used to modify or delete that item
 */
SubmenuItem* submenu_add_item(
    Submenu* submenu,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* callback_context);

/**
 * @brief Remove all items from submenu
 * @param submenu - Submenu instance
 */
void submenu_clean(Submenu* submenu);

/**
 * @brief Set submenu item selector
 * @param submenu
 * @param index
 */
void submenu_set_selected_item(Submenu* submenu, uint32_t index);

/**
 * @brief Set optional header for submenu
 * @param submenu   - submenu entity
 * @param header    - header to set
 */
void submenu_set_header(Submenu* submenu, const char* header);

#ifdef __cplusplus
}
#endif
