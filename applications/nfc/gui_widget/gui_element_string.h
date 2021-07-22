#pragma once
#include <stdint.h>
#include <gui/canvas.h>

typedef struct GuiElement GuiElement;

/** Allocate GuiElement element
 * @param x - x coordinate
 * @param y - y coordinate
 * @param horizontal - Align instance
 * @param vertical - Align instance
 * @param font Font instance
 * @return GuiElement instance
 */
GuiElement* gui_string_create(
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);
