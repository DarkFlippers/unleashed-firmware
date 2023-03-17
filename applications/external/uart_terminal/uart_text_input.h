#pragma once

#include <gui/view.h>
#include "uart_validators.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Text input anonymous structure */
typedef struct UART_TextInput UART_TextInput;
typedef void (*UART_TextInputCallback)(void* context);
typedef bool (*UART_TextInputValidatorCallback)(const char* text, FuriString* error, void* context);

/** Allocate and initialize text input 
 * 
 * This text input is used to enter string
 *
 * @return     UART_TextInput instance
 */
UART_TextInput* uart_text_input_alloc();

/** Deinitialize and free text input
 *
 * @param      uart_text_input  UART_TextInput instance
 */
void uart_text_input_free(UART_TextInput* uart_text_input);

/** Clean text input view Note: this function does not free memory
 *
 * @param      uart_text_input  Text input instance
 */
void uart_text_input_reset(UART_TextInput* uart_text_input);

/** Get text input view
 *
 * @param      uart_text_input  UART_TextInput instance
 *
 * @return     View instance that can be used for embedding
 */
View* uart_text_input_get_view(UART_TextInput* uart_text_input);

/** Set text input result callback
 *
 * @param      uart_text_input          UART_TextInput instance
 * @param      callback            callback fn
 * @param      callback_context    callback context
 * @param      text_buffer         pointer to YOUR text buffer, that we going
 *                                 to modify
 * @param      text_buffer_size    YOUR text buffer size in bytes. Max string
 *                                 length will be text_buffer_size-1.
 * @param      clear_default_text  clear text from text_buffer on first OK
 *                                 event
 */
void uart_text_input_set_result_callback(
    UART_TextInput* uart_text_input,
    UART_TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text);

void uart_text_input_set_validator(
    UART_TextInput* uart_text_input,
    UART_TextInputValidatorCallback callback,
    void* callback_context);

UART_TextInputValidatorCallback
    uart_text_input_get_validator_callback(UART_TextInput* uart_text_input);

void* uart_text_input_get_validator_callback_context(UART_TextInput* uart_text_input);

/** Set text input header text
 *
 * @param      uart_text_input  UART_TextInput instance
 * @param      text        text to be shown
 */
void uart_text_input_set_header_text(UART_TextInput* uart_text_input, const char* text);

#ifdef __cplusplus
}
#endif
