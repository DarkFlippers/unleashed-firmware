/**
 * @file number_input.h
 * GUI: Integer string keyboard view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number input anonymous structure  */
typedef struct NumberInput NumberInput;

/** Callback to be called on save button press */
typedef void (*NumberInputCallback)(void* context, int32_t number);

/** Allocate and initialize Number input.
 *
 * This Number input is used to enter Numbers (Integers).
 *
 * @return     NumberInput instance pointer
 */
NumberInput* number_input_alloc(void);

/** Deinitialize and free byte input
 *
 * @param      number_input  Number input instance
 */
void number_input_free(NumberInput* number_input);

/** Get byte input view
 *
 * @param      number_input  byte input instance
 *
 * @return     View instance that can be used for embedding
 */
View* number_input_get_view(NumberInput* number_input);

/** Set byte input result callback
 *
 * @param      number_input      byte input instance
 * @param      input_callback    input callback fn
 * @param      callback_context  callback context
 * @param[in]  current_number    The current number
 * @param      min_value         Min number value
 * @param      max_value         Max number value
 */

void number_input_set_result_callback(
    NumberInput* number_input,
    NumberInputCallback input_callback,
    void* callback_context,
    int32_t current_number,
    int32_t min_value,
    int32_t max_value);

/** Set byte input header text
 *
 * @param      number_input  byte input instance
 * @param      text          text to be shown
 */
void number_input_set_header_text(NumberInput* number_input, const char* text);

#ifdef __cplusplus
}
#endif
