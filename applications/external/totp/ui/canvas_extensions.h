#pragma once

#include <stdint.h>
#include <gui/gui.h>
#include <font_info.h>

void canvas_draw_str_ex(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    const char* text,
    size_t text_length,
    const FONT_INFO* const font);