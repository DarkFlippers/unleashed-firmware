#pragma once
#include "generic_view_module.h"
#include <gui/modules/popup.h>

class PopupVM : public GenericViewModule {
public:
    PopupVM();
    ~PopupVM() final;
    View* get_view() final;
    void clean() final;

    /** 
     * Set popup header text
     * @param text - text to be shown
     */
    void set_callback(PopupCallback callback);

    /** 
     * Set popup context
     * @param context - context pointer, will be passed to result callback
     */
    void set_context(void* context);

    /** 
     * Set popup header text
     * If text is null, popup header will not be rendered
     * @param text - text to be shown, can be multiline
     * @param x, y - text position
     * @param horizontal, vertical - text aligment
     */
    void set_header(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical);

    /** 
     * Set popup text
     * If text is null, popup text will not be rendered
     * @param text - text to be shown, can be multiline
     * @param x, y - text position
     * @param horizontal, vertical - text aligment
     */
    void set_text(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical);

    /** 
     * Set popup icon
     * If icon position is negative, popup icon will not be rendered
     * @param x, y - icon position
     * @param name - icon to be shown
     */
    void set_icon(int8_t x, int8_t y, const Icon* icon);

    /** 
     * Set popup timeout
     * @param timeout_in_ms - popup timeout value in milliseconds
     */
    void set_timeout(uint32_t timeout_in_ms);

    /** 
     * Enable popup timeout
     */
    void enable_timeout();

    /** 
     * Disable popup timeout
     */
    void disable_timeout();

private:
    Popup* popup;
};
