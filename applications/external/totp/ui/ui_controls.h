#pragma once

#include <inttypes.h>
#include <gui/gui.h>

#define UI_CONTROL_VSCROLL_WIDTH (3)

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

/**
 * @brief Renders vertical scroll bar
 * @param canvas canvas to render control at
 * @param x horizontal position of a control to be rendered at
 * @param y vertical position of a control to be rendered at
 * @param height control height
 * @param position current position
 * @param max_position maximal position
 */
void ui_control_vscroll_render(
    Canvas* const canvas,
    uint8_t x,
    uint8_t y,
    uint8_t height,
    uint8_t position,
    uint8_t max_position);
