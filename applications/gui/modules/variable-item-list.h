#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VariableItemList VariableItemList;
typedef struct VariableItem VariableItem;
typedef void (*VariableItemChangeCallback)(VariableItem* item);

/** Allocate and initialize VariableItemList
 * @return VariableItemList* 
 */
VariableItemList* variable_item_list_alloc();

/** Deinitialize and free VariableItemList
 * @param variable_item_list VariableItemList instance
 */
void variable_item_list_free(VariableItemList* variable_item_list);
View* variable_item_list_get_view(VariableItemList* variable_item_list);

/** Add item to VariableItemList
 * @param variable_item_list VariableItemList instance
 * @param label item name
 * @param values_count item values count
 * @param change_callback called on value change in gui
 * @param context item context
 * @return VariableItem* item instance
 */
VariableItem* variable_item_list_add(
    VariableItemList* variable_item_list,
    const char* label,
    uint8_t values_count,
    VariableItemChangeCallback change_callback,
    void* context);

/** Set item current selected index
 * @param item VariableItem* instance
 * @param current_value_index 
 */
void variable_item_set_current_value_index(VariableItem* item, uint8_t current_value_index);

/** Set item current selected text
 * @param item VariableItem* instance
 * @param current_value_text 
 */
void variable_item_set_current_value_text(VariableItem* item, const char* current_value_text);

/** Get item current selected index
 * @param item VariableItem* instance
 * @return uint8_t current selected index
 */
uint8_t variable_item_get_current_value_index(VariableItem* item);

/** Get item context
 * @param item VariableItem* instance
 * @return void* item context
 */
void* variable_item_get_context(VariableItem* item);

#ifdef __cplusplus
}
#endif