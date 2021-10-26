/**
 * @file code_input.h
 * GUI: CodeInput keyboard view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Code input anonymous structure  */
typedef struct CodeInput CodeInput;

/** callback that is executed when entered code matches ext buffer */
typedef void (*CodeInputOkCallback)(void* context);

/** callback that is executed when entered code does not matches ext buffer */
typedef void (*CodeInputFailCallback)(void* context);

/** Allocate and initialize code input. This code input is used to enter codes.
 *
 * @return     CodeInput instance pointer
 */
CodeInput* code_input_alloc();

/** Deinitialize and free code input
 *
 * @param      code_input  Code input instance
 */
void code_input_free(CodeInput* code_input);

/** Get code input view
 *
 * @param      code_input  code input instance
 *
 * @return     View instance that can be used for embedding
 */
View* code_input_get_view(CodeInput* code_input);

/** Set code input result callback
 *
 * @param      code_input        code input instance
 * @param      ok_callback    ok callback fn
 * @param      fail_callback  fail callback fn
 * @param      callback_context  callback context
 * @param      buffer       buffer to use
 * @param      buffer_length       buffer length
 * @param      update  set true to update buffer 
 */
void code_input_set_result_callback(
    CodeInput* code_input,
    CodeInputOkCallback ok_callback,
    CodeInputFailCallback fail_callback,
    void* callback_context,
    uint8_t* buffer,
    uint8_t* buffer_length,
    bool update);

/** Set code input header text
 *
 * @param      code_input  code input instance
 * @param      text        text to be shown
 */
void code_input_set_header_text(CodeInput* code_input, const char* text);

/** Compare two buffers
 *
 * @param      in       buffer to compare to source
 * @param      len_in   length of input buffer
 * @param      src      source buffer
 * @param      len_src  length of insourceput buffer
 * @return     true if buffers match
 */

bool code_input_compare(uint8_t* in, size_t len_in, uint8_t* src, size_t len_src);

/** Push input into the end of array
 *
 * @param      buffer   buffer
 * @param      length   length of buffer
 * @param      key      input key
 * @return     new length of input buffer
 */
size_t code_input_push(uint8_t* buffer, size_t length, InputKey key);

#ifdef __cplusplus
}
#endif