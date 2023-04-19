#pragma once

#include <gui/view.h>
#include "wifi_marauder_validators.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Text input anonymous structure */
typedef struct WIFI_TextInput WIFI_TextInput;
typedef void (*WIFI_TextInputCallback)(void* context);
typedef bool (*WIFI_TextInputValidatorCallback)(const char* text, FuriString* error, void* context);

/** Allocate and initialize text input 
 * 
 * This text input is used to enter string
 *
 * @return     WIFI_TextInput instance
 */
WIFI_TextInput* wifi_text_input_alloc();

/** Deinitialize and free text input
 *
 * @param      wifi_text_input  WIFI_TextInput instance
 */
void wifi_text_input_free(WIFI_TextInput* wifi_text_input);

/** Clean text input view Note: this function does not free memory
 *
 * @param      wifi_text_input  Text input instance
 */
void wifi_text_input_reset(WIFI_TextInput* wifi_text_input);

/** Get text input view
 *
 * @param      wifi_text_input  WIFI_TextInput instance
 *
 * @return     View instance that can be used for embedding
 */
View* wifi_text_input_get_view(WIFI_TextInput* wifi_text_input);

/** Set text input result callback
 *
 * @param      wifi_text_input          WIFI_TextInput instance
 * @param      callback            callback fn
 * @param      callback_context    callback context
 * @param      text_buffer         pointer to YOUR text buffer, that we going
 *                                 to modify
 * @param      text_buffer_size    YOUR text buffer size in bytes. Max string
 *                                 length will be text_buffer_size-1.
 * @param      clear_default_text  clear text from text_buffer on first OK
 *                                 event
 */
void wifi_text_input_set_result_callback(
    WIFI_TextInput* wifi_text_input,
    WIFI_TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text);

void wifi_text_input_set_validator(
    WIFI_TextInput* wifi_text_input,
    WIFI_TextInputValidatorCallback callback,
    void* callback_context);

WIFI_TextInputValidatorCallback
    wifi_text_input_get_validator_callback(WIFI_TextInput* wifi_text_input);

void* wifi_text_input_get_validator_callback_context(WIFI_TextInput* wifi_text_input);

/** Set text input header text
 *
 * @param      wifi_text_input  WIFI_TextInput instance
 * @param      text        text to be shown
 */
void wifi_text_input_set_header_text(WIFI_TextInput* wifi_text_input, const char* text);

#ifdef __cplusplus
}
#endif
