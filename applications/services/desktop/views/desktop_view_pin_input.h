#pragma once

#include <gui/view.h>

#include "../helpers/pin_code.h"

typedef void (*DesktopViewPinInputCallback)(void*);
typedef void (*DesktopViewPinInputDoneCallback)(const DesktopPinCode* pin_code, void*);
typedef struct DesktopViewPinInput DesktopViewPinInput;

DesktopViewPinInput* desktop_view_pin_input_alloc(void);
void desktop_view_pin_input_free(DesktopViewPinInput*);

void desktop_view_pin_input_set_pin(DesktopViewPinInput* pin_input, const DesktopPinCode* pin_code);
void desktop_view_pin_input_reset_pin(DesktopViewPinInput* pin_input);
void desktop_view_pin_input_hide_pin(DesktopViewPinInput* pin_input, bool pin_hidden);
void desktop_view_pin_input_set_label_button(DesktopViewPinInput* pin_input, const char* label);
void desktop_view_pin_input_set_label_primary(
    DesktopViewPinInput* pin_input,
    uint8_t x,
    uint8_t y,
    const char* label);
void desktop_view_pin_input_set_label_secondary(
    DesktopViewPinInput* pin_input,
    uint8_t x,
    uint8_t y,
    const char* label);
void desktop_view_pin_input_set_label_tertiary(
    DesktopViewPinInput* pin_input,
    uint8_t x,
    uint8_t y,
    const char* label);
void desktop_view_pin_input_set_pin_position(DesktopViewPinInput* pin_input, uint8_t x, uint8_t y);
View* desktop_view_pin_input_get_view(DesktopViewPinInput*);
void desktop_view_pin_input_set_done_callback(
    DesktopViewPinInput* pin_input,
    DesktopViewPinInputDoneCallback callback);
void desktop_view_pin_input_set_back_callback(
    DesktopViewPinInput* pin_input,
    DesktopViewPinInputCallback callback);
void desktop_view_pin_input_set_timeout_callback(
    DesktopViewPinInput* pin_input,
    DesktopViewPinInputCallback callback);
void desktop_view_pin_input_set_context(DesktopViewPinInput* pin_input, void* context);
void desktop_view_pin_input_lock_input(DesktopViewPinInput* pin_input);
void desktop_view_pin_input_unlock_input(DesktopViewPinInput* pin_input);
