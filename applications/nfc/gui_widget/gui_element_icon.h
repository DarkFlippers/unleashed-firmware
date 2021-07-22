#pragma once
#include <stdint.h>
#include <gui/canvas.h>

typedef struct GuiElement GuiElement;

/** Allocate GuiElement element
 * @param x - x coordinate
 * @param y - y coordinate
 * @param icon Icon instance
 * @return GuiElement instance
 */
GuiElement* gui_icon_create(uint8_t x, uint8_t y, const Icon* icon);
