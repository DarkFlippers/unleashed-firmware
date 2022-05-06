#pragma once
#include "generic_view_module.h"
#include <gui/modules/text_input.h>

class TextInputVM : public GenericViewModule {
public:
    TextInputVM();
    ~TextInputVM() final;
    View* get_view() final;
    void clean() final;

    /**
     * @brief Set text input result callback
     * 
     * @param callback - callback fn
     * @param callback_context - callback context
     * @param text - text buffer to use
     * @param max_text_length - text buffer length
     * @param clear_default_text - clears given buffer on OK event
     */
    void set_result_callback(
        TextInputCallback callback,
        void* callback_context,
        char* text,
        uint8_t max_text_length,
        bool clear_default_text);

    /** 
     * @brief Set text input header text
     * 
     * @param text - text to be shown
     */
    void set_header_text(const char* text);

    void set_validator(TextInputValidatorCallback callback, void* callback_context);

    void* get_validator_callback_context();

private:
    TextInput* text_input;
};
