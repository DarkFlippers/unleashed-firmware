#pragma once
#include "generic_view_module.h"
#include <gui/modules/byte_input.h>

class ByteInputVM : public GenericViewModule {
public:
    ByteInputVM();
    ~ByteInputVM() final;
    View* get_view() final;
    void clean() final;

    /** 
     * @brief Set byte input result callback
     * 
     * @param input_callback input callback fn
     * @param changed_callback changed callback fn
     * @param callback_context callback context
     * @param bytes buffer to use
     * @param bytes_count buffer length
     */
    void set_result_callback(
        ByteInputCallback input_callback,
        ByteChangedCallback changed_callback,
        void* callback_context,
        uint8_t* bytes,
        uint8_t bytes_count);

    /**
     * @brief Set byte input header text
     * 
     * @param text text to be shown
     */
    void set_header_text(const char* text);

private:
    ByteInput* byte_input;
};
