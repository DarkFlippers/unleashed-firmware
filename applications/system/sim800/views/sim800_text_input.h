/**
 * @file sim800_text_input.h
 * @brief Text input with a second, symbol keyboard.
 *
 * Ported from the WiFi Marauder companion app, with a symbol layout aimed at this app: the
 * stock firmware keyboard has no "+", "*", "#", "=" or quote, which numbers, USSD codes and
 * AT commands all need.
 */
#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Sim800TextInput Sim800TextInput;
typedef void (*Sim800TextInputCallback)(void* context);

Sim800TextInput* sim800_text_input_alloc(void);
void sim800_text_input_free(Sim800TextInput* text_input);
void sim800_text_input_reset(Sim800TextInput* text_input);
View* sim800_text_input_get_view(Sim800TextInput* text_input);

/** @param text_buffer_size  caller's buffer size; the longest string is one less. */
void sim800_text_input_set_result_callback(
    Sim800TextInput* text_input,
    Sim800TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text);

void sim800_text_input_set_minimum_length(Sim800TextInput* text_input, size_t minimum_length);
void sim800_text_input_set_header_text(Sim800TextInput* text_input, const char* text);

#ifdef __cplusplus
}
#endif
