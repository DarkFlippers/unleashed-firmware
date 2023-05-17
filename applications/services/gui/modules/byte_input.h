/**
 * @file byte_input.h
 * GUI: ByteInput keyboard view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Byte input anonymous structure  */
typedef struct ByteInput ByteInput;

/** callback that is executed on save button press */
typedef void (*ByteInputCallback)(void* context);

/** callback that is executed when byte buffer is changed */
typedef void (*ByteChangedCallback)(void* context);

/** Allocate and initialize byte input. This byte input is used to enter bytes.
 *
 * @return     ByteInput instance pointer
 */
ByteInput* byte_input_alloc();

/** Deinitialize and free byte input
 *
 * @param      byte_input  Byte input instance
 */
void byte_input_free(ByteInput* byte_input);

/** Get byte input view
 *
 * @param      byte_input  byte input instance
 *
 * @return     View instance that can be used for embedding
 */
View* byte_input_get_view(ByteInput* byte_input);

/** Set byte input result callback
 *
 * @param      byte_input        byte input instance
 * @param      input_callback    input callback fn
 * @param      changed_callback  changed callback fn
 * @param      callback_context  callback context
 * @param      bytes             buffer to use
 * @param      bytes_count       buffer length
 */
void byte_input_set_result_callback(
    ByteInput* byte_input,
    ByteInputCallback input_callback,
    ByteChangedCallback changed_callback,
    void* callback_context,
    uint8_t* bytes,
    uint8_t bytes_count);

/** Set byte input header text
 *
 * @param      byte_input  byte input instance
 * @param      text        text to be shown
 */
void byte_input_set_header_text(ByteInput* byte_input, const char* text);

#ifdef __cplusplus
}
#endif
