#pragma once
#include <gui/view.h>
#include "gui_element_string.h"
#include "gui_element_button.h"
#include "gui_element_icon.h"

typedef struct GuiWidget GuiWidget;
typedef struct GuiElement GuiElement;

/** Allocate Gui Widget that holds Gui Elements
 * @return GuiWidget instance
 */
GuiWidget* gui_widget_alloc();

/** Free Gui Widget
 * @note this function free Gui Elements
 * @param gui_widget GuiWidget instance
 */
void gui_widget_free(GuiWidget* gui_widget);

/** Clear Gui Widget
 * @param gui_widget GuiWidget instance
 */
void gui_widget_clear(GuiWidget* gui_widget);

/** Get Gui Widget view
 * @param gui_widget GuiWidget instance
 * @return View instance
 */
View* gui_widget_get_view(GuiWidget* gui_widget);

/** Add generic Gui Elements to Gui Widget
 * @param gui_widget GuiWidget instance
 * @param element GuiElement element
 */
void gui_widget_add_element(GuiWidget* gui_widget, GuiElement* element);

/** Add String Element
 * @param gui_widget GuiWidget instance
 * @param x - x coordinate
 * @param y - y coordinate
 * @param horizontal - Align instance
 * @param vertical - Align instance
 * @param font Font instance
 * @return GuiElement instance
 */
void gui_widget_add_string_element(
    GuiWidget* gui_widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Add Button Element
 * @param gui_widget GuiWidget instance
 * @param button_type GuiButtonType instance
 * @param text text on allocated button
 * @param callback ButtonCallback instance
 * @param context pointer to context
 */
void gui_widget_add_button_element(
    GuiWidget* gui_widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context);

/** Add Icon Element
 * @param gui_widget GuiWidget instance
 * @param x - x coordinate
 * @param y - y coordinate
 * @param icon Icon instance
 */
void gui_widget_add_icon_element(GuiWidget* gui_widget, uint8_t x, uint8_t y, const Icon* icon);
