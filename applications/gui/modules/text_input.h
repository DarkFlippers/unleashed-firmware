/**
 * @file text_input.h
 * GUI: TextInput keybord view module API
 */

#pragma once

#include <gui/view.h>
#include "validators.h"
#include <m-string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Text input anonymous structure */
typedef struct TextInput TextInput;
typedef void (*TextInputCallback)(void* context);
typedef bool (*TextInputValidatorCallback)(const char* text, string_t error, void* context);

/** Allocate and initialize text input 
 * 
 * This text input is used to enter string
 *
 * @return     TextInput instance
 */
TextInput* text_input_alloc();

/** Deinitialize and free text input
 *
 * @param      text_input  TextInput instance
 */
void text_input_free(TextInput* text_input);

/** Clean text input view Note: this function does not free memory
 *
 * @param      text_input  Text input instance
 */
void text_input_reset(TextInput* text_input);

/** Get text input view
 *
 * @param      text_input  TextInput instance
 *
 * @return     View instance that can be used for embedding
 */
View* text_input_get_view(TextInput* text_input);

/** Set text input result callback
 *
 * @param      text_input          TextInput instance
 * @param      callback            callback fn
 * @param      callback_context    callback context
 * @param      text_buffer         pointer to YOUR text buffer, that we going
 *                                 to modify
 * @param      text_buffer_size    YOUR text buffer size in bytes. Max string
 *                                 length will be text_buffer_size-1.
 * @param      clear_default_text  clear text from text_buffer on first OK
 *                                 event
 */
void text_input_set_result_callback(
    TextInput* text_input,
    TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text);

void text_input_set_validator(
    TextInput* text_input,
    TextInputValidatorCallback callback,
    void* callback_context);

TextInputValidatorCallback text_input_get_validator_callback(TextInput* text_input);

void* text_input_get_validator_callback_context(TextInput* text_input);

/** Set text input header text
 *
 * @param      text_input  TextInput instance
 * @param      text        text to be shown
 */
void text_input_set_header_text(TextInput* text_input, const char* text);

#ifdef __cplusplus
}
#endif
