/**
 * @file button_panel.h
 * GUI: ButtonPanel view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Button panel module descriptor */
typedef struct ButtonPanel ButtonPanel;

/** Callback type to call for handling selecting button_panel items */
typedef void (*ButtonItemCallback)(void* context, uint32_t index);

/** Callback type for additional drawings above main button_panel screen */
typedef void (*ButtonPanelDrawCallback)(Canvas* canvas, void* _model);

/** Callback type to intercept input events of button_panel */
typedef bool (*ButtonPanelInputCallback)(InputEvent* event, void* context);

/** Allocate new button_panel module.
 *
 * @return     ButtonPanel instance
 */
ButtonPanel* button_panel_alloc(void);

/** Free button_panel module.
 *
 * @param      button_panel  ButtonPanel instance
 */
void button_panel_free(ButtonPanel* button_panel);

/** Free items from button_panel module. Preallocated matrix stays unchanged.
 *
 * @param      button_panel  ButtonPanel instance
 */
void button_panel_reset(ButtonPanel* button_panel);

/** Reserve space for adding items.
 *
 * One does not simply use button_panel_add_item() without this function. It
 * should be allocated space for it first.
 *
 * @param      button_panel  ButtonPanel instance
 * @param      reserve_x     number of columns in button_panel
 * @param      reserve_y     number of rows in button_panel
 */
void button_panel_reserve(ButtonPanel* button_panel, size_t reserve_x, size_t reserve_y);

/** Add item to button_panel module.
 *
 * Have to set element in bounds of allocated size by X and by Y.
 *
 * @param      button_panel        ButtonPanel instance
 * @param      index               value to pass to callback
 * @param      matrix_place_x      coordinates by x-axis on virtual grid, it
 *                                 is only used for naviagation
 * @param      matrix_place_y      coordinates by y-axis on virtual grid, it
 *                                 is only used for naviagation
 * @param      x                   x-coordinate to draw icon on
 * @param      y                   y-coordinate to draw icon on
 * @param      icon_name           name of the icon to draw
 * @param      icon_name_selected  name of the icon to draw when current
 *                                 element is selected
 * @param      callback            function to call when specific element is
 *                                 selected (pressed Ok on selected item)
 * @param      callback_context    context to pass to callback
 */
void button_panel_add_item(
    ButtonPanel* button_panel,
    uint32_t index,
    uint16_t matrix_place_x,
    uint16_t matrix_place_y,
    uint16_t x,
    uint16_t y,
    const Icon* icon_name,
    const Icon* icon_name_selected,
    ButtonItemCallback callback,
    void* callback_context);

/** Get button_panel view.
 *
 * @param      button_panel  ButtonPanel instance
 *
 * @return     acquired view
 */
View* button_panel_get_view(ButtonPanel* button_panel);

/** Add label to button_panel module.
 *
 * @param      button_panel  ButtonPanel instance
 * @param      x             x-coordinate to place label
 * @param      y             y-coordinate to place label
 * @param      font          font to write label with
 * @param      label_str     string label to write
 */
void button_panel_add_label(
    ButtonPanel* button_panel,
    uint16_t x,
    uint16_t y,
    Font font,
    const char* label_str);

// TODO: [FL-1445] Have to replace callbacks above with additional popup-layer
/** Set popup draw callback for button_panel module.
 *
 * Used to add popup drawings after main draw callback is done.
 *
 * @param      button_panel  ButtonPanel instance
 * @param      callback      callback function to set for draw event
 * @param      context       context to pass to callback
 */
void button_panel_set_popup_draw_callback(
    ButtonPanel* button_panel,
    ButtonPanelDrawCallback callback,
    void* context);

/** Set popup input callback for button_panel module.
 *
 * Used to add popup input callback. It will intercept all input events for
 * current view.
 *
 * @param      button_panel  ButtonPanel instance
 * @param      callback      function to overwrite main input callbacks
 * @param      context       context to pass to callback
 */
void button_panel_set_popup_input_callback(
    ButtonPanel* button_panel,
    ButtonPanelInputCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
