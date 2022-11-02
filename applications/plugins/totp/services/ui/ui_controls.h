#pragma once

#include <inttypes.h>
#include <gui/gui.h>

void ui_control_text_box_render(
    Canvas* const canvas,
    int16_t y,
    const char* text,
    bool is_selected);
void ui_control_button_render(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t width,
    uint8_t height,
    const char* text,
    bool is_selected);
void ui_control_select_render(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t width,
    const char* text,
    bool is_selected);
