#pragma once

#include <inttypes.h>
#include <gui/gui.h>

/**
 * @brief Renders TextBox control
 * @param canvas canvas to render control at
 * @param y vertical position of a control to be rendered at
 * @param text text to be rendered inside control
 * @param is_selected whether control should be rendered as focused or not
 */
void ui_control_text_box_render(
    Canvas* const canvas,
    int16_t y,
    const char* text,
    bool is_selected);

/**
 * @brief Renders Button control
 * @param canvas canvas to render control at
 * @param x horizontal position of a control to be rendered at
 * @param y vertical position of a control to be rendered at
 * @param width control width
 * @param height control height
 * @param text text to be rendered inside control
 * @param is_selected whether control should be rendered as focused or not
 */
void ui_control_button_render(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t width,
    uint8_t height,
    const char* text,
    bool is_selected);

/**
 * @brief Renders Select control
 * @param canvas canvas to render control at
 * @param x horizontal position of a control to be rendered at
 * @param y vertical position of a control to be rendered at
 * @param width control width
 * @param text text to be rendered inside control
 * @param is_selected whether control should be rendered as focused or not
 */
void ui_control_select_render(
    Canvas* const canvas,
    int16_t x,
    int16_t y,
    uint8_t width,
    const char* text,
    bool is_selected);
