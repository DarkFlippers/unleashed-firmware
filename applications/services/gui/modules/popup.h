/**
 * @file popup.h
 * GUI: Popup view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Popup anonymous structure */
typedef struct Popup Popup;

/** Popup result callback type
 * @warning    comes from GUI thread
 */
typedef void (*PopupCallback)(void* context);

/** Allocate and initialize popup
 *
 * This popup used to ask simple questions like Yes/
 *
 * @return     Popup instance
 */
Popup* popup_alloc(void);

/** Deinitialize and free popup
 *
 * @param      popup  Popup instance
 */
void popup_free(Popup* popup);

/** Get popup view
 *
 * @param      popup  Popup instance
 *
 * @return     View instance that can be used for embedding
 */
View* popup_get_view(Popup* popup);

/** Set popup header text
 *
 * @param      popup     Popup instance
 * @param      callback  PopupCallback
 */
void popup_set_callback(Popup* popup, PopupCallback callback);

/** Set popup context
 *
 * @param      popup    Popup instance
 * @param      context  context pointer, will be passed to result callback
 */
void popup_set_context(Popup* popup, void* context);

/** Set popup header text
 *
 * If text is null, popup header will not be rendered
 *
 * @param      popup       Popup instance
 * @param      text        text to be shown, can be multiline
 * @param      x           x position
 * @param      y           y position
 * @param      horizontal  horizontal alignment
 * @param      vertical    vertical alignment
 */
void popup_set_header(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/** Set popup text
 *
 * If text is null, popup text will not be rendered
 *
 * @param      popup       Popup instance
 * @param      text        text to be shown, can be multiline
 * @param      x           x position
 * @param      y           y position
 * @param      horizontal  horizontal alignment
 * @param      vertical    vertical alignment
 */
void popup_set_text(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/** Set popup icon
 *
 * If icon position is negative, popup icon will not be rendered
 *
 * @param      popup  Popup instance
 * @param      x      x position
 * @param      y      y position
 * @param      icon   pointer to Icon data
 */
void popup_set_icon(Popup* popup, uint8_t x, uint8_t y, const Icon* icon);

/** Set popup timeout
 *
 * @param      popup          Popup instance
 * @param      timeout_in_ms  popup timeout value in milliseconds
 */
void popup_set_timeout(Popup* popup, uint32_t timeout_in_ms);

/** Enable popup timeout
 *
 * @param      popup  Popup instance
 */
void popup_enable_timeout(Popup* popup);

/** Disable popup timeout
 *
 * @param      popup  Popup instance
 */
void popup_disable_timeout(Popup* popup);

/** Reset popup instance state
 *
 * @param       popup Popup instance
 */
void popup_reset(Popup* popup);

#ifdef __cplusplus
}
#endif
