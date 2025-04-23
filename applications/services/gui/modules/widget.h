/**
 * @file widget.h
 * GUI: Widget view module API
 */

#pragma once
#include <gui/view.h>
#include "widget_elements/widget_element.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Widget Widget;
typedef struct WidgetElement WidgetElement;

/** Allocate Widget that holds Widget Elements
 *
 * @return     Widget instance
 */
Widget* widget_alloc(void);

/** Free Widget
 * @note       this function free allocated Widget Elements
 *
 * @param      widget  Widget instance
 */
void widget_free(Widget* widget);

/** Reset Widget
 *
 * @param      widget  Widget instance
 */
void widget_reset(Widget* widget);

/** Get Widget view
 *
 * @param      widget  Widget instance
 *
 * @return     View instance
 */
View* widget_get_view(Widget* widget);

/** Add Multi String Element
 *
 * @param      widget      Widget instance
 * @param      x           x coordinate
 * @param      y           y coordinate
 * @param      horizontal  Align instance
 * @param      vertical    Align instance
 * @param      font        Font instance
 * @param[in]  text        The text
 */
void widget_add_string_multiline_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Add String Element
 *
 * @param      widget      Widget instance
 * @param      x           x coordinate
 * @param      y           y coordinate
 * @param      horizontal  Align instance
 * @param      vertical    Align instance
 * @param      font        Font instance
 * @param[in]  text        The text
 */
void widget_add_string_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Add Text Box Element
 *
 * @param      widget           Widget instance
 * @param      x                x coordinate
 * @param      y                y coordinate
 * @param      width            width to fit text
 * @param      height           height to fit text
 * @param      horizontal       Align instance
 * @param      vertical         Align instance
 * @param[in]  text             Formatted text. The following formats are available:
 *                               "\e#Bold text\e#" - bold font is used
 *                               "\e*Monospaced text\e*" - monospaced font is used
 *                               "\e!Inverted text\e!" - white text on black background
 * @param      strip_to_dots    Strip text to ... if does not fit to width
 */
void widget_add_text_box_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    Align horizontal,
    Align vertical,
    const char* text,
    bool strip_to_dots);

/** Add Text Scroll Element
 *
 * @param      widget           Widget instance
 * @param      x                x coordinate
 * @param      y                y coordinate
 * @param      width            width to fit text
 * @param      height           height to fit text
 * @param[in]  text             Formatted text. Default format: align left, Secondary font.
 *                              The following formats are available:
 *                               "\e#Bold text" - sets bold font before until next '\n' symbol
 *                               "\e*Monospaced text\e*" - sets monospaced font before until next '\n' symbol
 *                               "\ecCenter-aligned text" - sets center horizontal align until the next '\n' symbol
 *                               "\erRight-aligned text" - sets right horizontal align until the next '\n' symbol
 */
void widget_add_text_scroll_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const char* text);

/** Add Button Element
 *
 * @param      widget       Widget instance
 * @param      button_type  GuiButtonType instance
 * @param      text         text on allocated button
 * @param      callback     ButtonCallback instance
 * @param      context      pointer to context
 */
void widget_add_button_element(
    Widget* widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context);

/** Add Icon Element
 *
 * @param      widget  Widget instance
 * @param      x       top left x coordinate
 * @param      y       top left y coordinate
 * @param      icon    Icon instance
 */
void widget_add_icon_element(Widget* widget, uint8_t x, uint8_t y, const Icon* icon);

/** Add Frame Element
  *
  * @param      widget  Widget instance
  * @param      x       top left x coordinate
  * @param      y       top left y coordinate
  * @param      width   frame width
  * @param      height  frame height
  * @param      radius  frame radius
  * 
  * @warning deprecated, use widget_add_rect_element instead
  */
#define widget_add_frame_element(widget, x, y, width, height, radius) \
    widget_add_rect_element((widget), (x), (y), (width), (height), (radius), false)

/** Add Rect Element
 *
 * @param      widget  Widget instance
 * @param      x       top left x coordinate
 * @param      y       top left y coordinate
 * @param      width   rect width
 * @param      height  rect height
 * @param      radius  corner radius
 * @param      fill    whether to fill the box or not
 */
void widget_add_rect_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius,
    bool fill);

/** Add Circle Element
 *
 * @param      widget  Widget instance
 * @param      x       center x coordinate
 * @param      y       center y coordinate
 * @param      radius  circle radius
 * @param      fill    whether to fill the circle or not
 */
void widget_add_circle_element(Widget* widget, uint8_t x, uint8_t y, uint8_t radius, bool fill);

/** Add Line Element
 *
 * @param      widget  Widget instance
 * @param      x1      first x coordinate
 * @param      y1      first y coordinate
 * @param      x2      second x coordinate
 * @param      y2      second y coordinate
 */
void widget_add_line_element(Widget* widget, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

#ifdef __cplusplus
}
#endif
