#pragma once
#include "generic_view_module.h"
#include <gui/modules/dialog_ex.h>

class DialogExVM : public GenericViewModule {
public:
    DialogExVM();
    ~DialogExVM() final;
    View* get_view() final;
    void clean() final;

    /** 
     * Set dialog result callback
     * @param callback - result callback function
     */
    void set_result_callback(DialogExResultCallback callback);

    /** 
     * Set dialog context
     * @param context - context pointer, will be passed to result callback
     */
    void set_context(void* context);

    /** 
     * Set dialog header text
     * If text is null, dialog header will not be rendered
     * @param text - text to be shown, can be multiline
     * @param x, y - text position
     * @param horizontal, vertical - text aligment
     */
    void set_header(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical);

    /** 
     * Set dialog text
     * If text is null, dialog text will not be rendered
     * @param text - text to be shown, can be multiline
     * @param x, y - text position
     * @param horizontal, vertical - text aligment
     */
    void set_text(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical);

    /** 
     * Set dialog icon
     * If x or y is negative, dialog icon will not be rendered
     * @param x, y - icon position
     * @param name - icon to be shown
     */
    void set_icon(uint8_t x, uint8_t y, const Icon* icon);

    /**
     * Set left button text
     * If text is null, left button will not be rendered and processed
     * @param text - text to be shown
     */
    void set_left_button_text(const char* text);

    /** 
     * Set center button text
     * If text is null, center button will not be rendered and processed
     * @param text - text to be shown
     */
    void set_center_button_text(const char* text);

    /**
     * Set right button text
     * If text is null, right button will not be rendered and processed
     * @param text - text to be shown
     */
    void set_right_button_text(const char* text);

private:
    DialogEx* dialog_ex;
};
