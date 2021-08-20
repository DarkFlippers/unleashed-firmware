#pragma once
#include "widget_elements/widget_element_i.h"

typedef struct Widget Widget;
typedef struct WidgetElement WidgetElement;

/** Allocate Widget that holds Widget Elements
 * @return Widget instance
 */
Widget* widget_alloc();

/** Free Widget
 * @note this function free allocated Widget Elements
 * @param widget Widget instance
 */
void widget_free(Widget* widget);

/** Clear Widget
 * @param widget Widget instance
 */
void widget_clear(Widget* widget);

/** Get Widget view
 * @param widget Widget instance
 * @return View instance
 */
View* widget_get_view(Widget* widget);

/** Add String Element
 * @param widget Widget instance
 * @param x - x coordinate
 * @param y - y coordinate
 * @param horizontal - Align instance
 * @param vertical - Align instance
 * @param font Font instance
 */
void widget_add_string_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Add Button Element
 * @param widget Widget instance
 * @param button_type GuiButtonType instance
 * @param text text on allocated button
 * @param callback ButtonCallback instance
 * @param context pointer to context
 */
void widget_add_button_element(
    Widget* widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context);

/** Add Icon Element
 * @param widget Widget instance
 * @param x top left x coordinate
 * @param y top left y coordinate
 * @param icon Icon instance
 */
void widget_add_icon_element(Widget* widget, uint8_t x, uint8_t y, const Icon* icon);

/** Add Frame Element
 * @param widget Widget instance
 * @param x top left x coordinate
 * @param y top left y coordinate
 * @param width frame width
 * @param height frame height
 * @param radius frame radius
 */
void widget_add_frame_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius);
