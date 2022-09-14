#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Start Hid Keyboard Profile
 */
void furi_hal_bt_hid_start();

/** Stop Hid Keyboard Profile
 */
void furi_hal_bt_hid_stop();

/** Press keyboard button
 *
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_press(uint16_t button);

/** Release keyboard button
 *
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_release(uint16_t button);

/** Release all keyboard buttons
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_release_all();

/** Set mouse movement and send HID report
 *
 * @param      dx  x coordinate delta
 * @param      dy  y coordinate delta
 */
bool furi_hal_bt_hid_mouse_move(int8_t dx, int8_t dy);

/** Set mouse button to pressed state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_bt_hid_mouse_press(uint8_t button);

/** Set mouse button to released state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_bt_hid_mouse_release(uint8_t button);

/** Set mouse button to released state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_bt_hid_mouse_release_all();

/** Set mouse wheel position and send HID report
 *
 * @param      delta  number of scroll steps
 */
bool furi_hal_bt_hid_mouse_scroll(int8_t delta);

/** Set the following consumer key to pressed state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_bt_hid_consumer_key_press(uint16_t button);

/** Set the following consumer key to released state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_bt_hid_consumer_key_release(uint16_t button);

/** Set consumer key to released state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_bt_hid_consumer_key_release_all();

#ifdef __cplusplus
}
#endif
