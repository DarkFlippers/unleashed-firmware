#pragma once

#include <gui/view.h>
#include "nfc_maker_validators.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Text input anonymous structure */
typedef struct NFCMaker_TextInput NFCMaker_TextInput;
typedef void (*NFCMaker_TextInputCallback)(void* context);
typedef bool (
    *NFCMaker_TextInputValidatorCallback)(const char* text, FuriString* error, void* context);

/** Allocate and initialize text input 
 * 
 * This text input is used to enter string
 *
 * @return     NFCMaker_TextInput instance
 */
NFCMaker_TextInput* nfc_maker_text_input_alloc();

/** Deinitialize and free text input
 *
 * @param      text_input  NFCMaker_TextInput instance
 */
void nfc_maker_text_input_free(NFCMaker_TextInput* text_input);

/** Clean text input view Note: this function does not free memory
 *
 * @param      text_input  Text input instance
 */
void nfc_maker_text_input_reset(NFCMaker_TextInput* text_input);

/** Get text input view
 *
 * @param      text_input  NFCMaker_TextInput instance
 *
 * @return     View instance that can be used for embedding
 */
View* nfc_maker_text_input_get_view(NFCMaker_TextInput* text_input);

/** Set text input result callback
 *
 * @param      text_input          NFCMaker_TextInput instance
 * @param      callback            callback fn
 * @param      callback_context    callback context
 * @param      text_buffer         pointer to YOUR text buffer, that we going
 *                                 to modify
 * @param      text_buffer_size    YOUR text buffer size in bytes. Max string
 *                                 length will be text_buffer_size-1.
 * @param      clear_default_text  clear text from text_buffer on first OK
 *                                 event
 */
void nfc_maker_text_input_set_result_callback(
    NFCMaker_TextInput* text_input,
    NFCMaker_TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text);

void nfc_maker_text_input_set_validator(
    NFCMaker_TextInput* text_input,
    NFCMaker_TextInputValidatorCallback callback,
    void* callback_context);

void nfc_maker_text_input_set_minimum_length(NFCMaker_TextInput* text_input, size_t minimum_length);

NFCMaker_TextInputValidatorCallback
    nfc_maker_text_input_get_validator_callback(NFCMaker_TextInput* text_input);

void* nfc_maker_text_input_get_validator_callback_context(NFCMaker_TextInput* text_input);

/** Set text input header text
 *
 * @param      text_input  NFCMaker_TextInput instance
 * @param      text        text to be shown
 */
void nfc_maker_text_input_set_header_text(NFCMaker_TextInput* text_input, const char* text);

#ifdef __cplusplus
}
#endif