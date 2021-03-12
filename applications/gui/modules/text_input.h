#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Text input anonymous structure */
typedef struct TextInput TextInput;
typedef void (*TextInputCallback)(void* context, char* text);

/* Allocate and initialize text input
 * This text input is used to enter string
 */
TextInput* text_input_alloc();

/* Deinitialize and free text input
 * @param text_input - Text input instance
 */
void text_input_free(TextInput* text_input);

/* Get text input view
 * @param text_input - Text input instance
 * @return View instance that can be used for embedding
 */
View* text_input_get_view(TextInput* text_input);

/* Deinitialize and free text input
 * @param text_input - Text input instance
 * @param callback - callback fn
 * @param callback_context - callback context
 * @param text - text buffer to use
 * @param max_text_length - text buffer length
 */
void text_input_set_result_callback(
    TextInput* text_input,
    TextInputCallback callback,
    void* callback_context,
    char* text,
    uint8_t max_text_length);

/* Set text input header text
 * @param text input - Text input instance
 * @param text - text to be shown
 */
void text_input_set_header_text(TextInput* text_input, const char* text);

#ifdef __cplusplus
}
#endif