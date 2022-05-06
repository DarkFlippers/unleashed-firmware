#pragma once
#include "generic_view_module.h"
#include <gui/modules/submenu.h>

class SubmenuVM : public GenericViewModule {
public:
    SubmenuVM();
    ~SubmenuVM() final;
    View* get_view() final;
    void clean() final;

    /**
     * @brief Add item to submenu
     * 
     * @param label - menu item label
     * @param index - menu item index, used for callback, may be the same with other items
     * @param callback - menu item callback
     * @param callback_context - menu item callback context
     */
    void add_item(
        const char* label,
        uint32_t index,
        SubmenuItemCallback callback,
        void* callback_context);

    /**
     * @brief Set submenu item selector
     * 
     * @param index index of the item to be selected
     */
    void set_selected_item(uint32_t index);

    /**
     * @brief Set optional header for submenu
     * 
     * @param header header to set
     */
    void set_header(const char* header);

private:
    Submenu* submenu;
};
