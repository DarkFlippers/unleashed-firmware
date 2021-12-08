#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Start Hid Keyboard Profile
 */
void furi_hal_bt_hid_start();

/** Stop Hid Keyboard Profile
 */
void furi_hal_bt_hid_stop();

/** Press key button
 *
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_press(uint16_t button);

/** Release key button
 *
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_release(uint16_t button);

/** Release all key buttons
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_release_all();
