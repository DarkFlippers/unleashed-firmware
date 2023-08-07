#pragma once

#include <stdint.h>
#include <gui/gui.h>
#include <font_info.h>

/**
 * @brief Draw string using given font
 * @param canvas canvas to draw string at
 * @param x horizontal position
 * @param y vertical position
 * @param text string to draw
 * @param text_length string length
 * @param font font to be used to draw string
 */
void canvas_draw_str_ex(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    const char* text,
    size_t text_length,
    const FONT_INFO* const font);