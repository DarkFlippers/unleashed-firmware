#pragma once
#include "hid_usage_desktop.h"
#include "hid_usage_button.h"
#include "hid_usage_keyboard.h"
#include "hid_usage_consumer.h"
#include "hid_usage_led.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Max number of simultaneously pressed keys (keyboard) */
#define HID_KB_MAX_KEYS       6
/** Max number of simultaneously pressed keys (consumer control) */
#define HID_CONSUMER_MAX_KEYS 2

/** OS-specific consumer keys, defined as "Reserved" in HID Usage Tables document */
#define HID_CONSUMER_BRIGHTNESS_INCREMENT 0x006F
#define HID_CONSUMER_BRIGHTNESS_DECREMENT 0x0070
#define HID_CONSUMER_FN_GLOBE             0x029D

#define HID_KEYBOARD_NONE 0x00

/** HID keyboard modifier keys */
enum HidKeyboardMods {
    KEY_MOD_LEFT_CTRL = (1 << 8),
    KEY_MOD_LEFT_SHIFT = (1 << 9),
    KEY_MOD_LEFT_ALT = (1 << 10),
    KEY_MOD_LEFT_GUI = (1 << 11),
    KEY_MOD_RIGHT_CTRL = (1 << 12),
    KEY_MOD_RIGHT_SHIFT = (1 << 13),
    KEY_MOD_RIGHT_ALT = (1 << 14),
    KEY_MOD_RIGHT_GUI = (1 << 15),
};

/** ASCII to keycode conversion table */
static const uint16_t hid_asciimap[] = {
    HID_KEYBOARD_NONE, // NUL
    HID_KEYBOARD_NONE, // SOH
    HID_KEYBOARD_NONE, // STX
    HID_KEYBOARD_NONE, // ETX
    HID_KEYBOARD_NONE, // EOT
    HID_KEYBOARD_NONE, // ENQ
    HID_KEYBOARD_NONE, // ACK
    HID_KEYBOARD_NONE, // BEL
    HID_KEYBOARD_DELETE, // BS   Backspace
    HID_KEYBOARD_TAB, // TAB  Tab
    HID_KEYBOARD_RETURN, // LF   Enter
    HID_KEYBOARD_NONE, // VT
    HID_KEYBOARD_NONE, // FF
    HID_KEYBOARD_NONE, // CR
    HID_KEYBOARD_NONE, // SO
    HID_KEYBOARD_NONE, // SI
    HID_KEYBOARD_NONE, // DEL
    HID_KEYBOARD_NONE, // DC1
    HID_KEYBOARD_NONE, // DC2
    HID_KEYBOARD_NONE, // DC3
    HID_KEYBOARD_NONE, // DC4
    HID_KEYBOARD_NONE, // NAK
    HID_KEYBOARD_NONE, // SYN
    HID_KEYBOARD_NONE, // ETB
    HID_KEYBOARD_NONE, // CAN
    HID_KEYBOARD_NONE, // EM
    HID_KEYBOARD_NONE, // SUB
    HID_KEYBOARD_NONE, // ESC
    HID_KEYBOARD_NONE, // FS
    HID_KEYBOARD_NONE, // GS
    HID_KEYBOARD_NONE, // RS
    HID_KEYBOARD_NONE, // US
    HID_KEYBOARD_SPACEBAR, // ' ' Space
    HID_KEYBOARD_1 | KEY_MOD_LEFT_SHIFT, // !
    HID_KEYBOARD_APOSTROPHE | KEY_MOD_LEFT_SHIFT, // "
    HID_KEYBOARD_3 | KEY_MOD_LEFT_SHIFT, // #
    HID_KEYBOARD_4 | KEY_MOD_LEFT_SHIFT, // $
    HID_KEYBOARD_5 | KEY_MOD_LEFT_SHIFT, // %
    HID_KEYBOARD_7 | KEY_MOD_LEFT_SHIFT, // &
    HID_KEYBOARD_APOSTROPHE, // '
    HID_KEYBOARD_9 | KEY_MOD_LEFT_SHIFT, // (
    HID_KEYBOARD_0 | KEY_MOD_LEFT_SHIFT, // )
    HID_KEYBOARD_8 | KEY_MOD_LEFT_SHIFT, // *
    HID_KEYBOARD_EQUAL_SIGN | KEY_MOD_LEFT_SHIFT, // +
    HID_KEYBOARD_COMMA, // ,
    HID_KEYBOARD_MINUS, // -
    HID_KEYBOARD_DOT, // .
    HID_KEYBOARD_SLASH, // /
    HID_KEYBOARD_0, // 0
    HID_KEYBOARD_1, // 1
    HID_KEYBOARD_2, // 2
    HID_KEYBOARD_3, // 3
    HID_KEYBOARD_4, // 4
    HID_KEYBOARD_5, // 5
    HID_KEYBOARD_6, // 6
    HID_KEYBOARD_7, // 7
    HID_KEYBOARD_8, // 8
    HID_KEYBOARD_9, // 9
    HID_KEYBOARD_SEMICOLON | KEY_MOD_LEFT_SHIFT, // :
    HID_KEYBOARD_SEMICOLON, // ;
    HID_KEYBOARD_COMMA | KEY_MOD_LEFT_SHIFT, // <
    HID_KEYBOARD_EQUAL_SIGN, // =
    HID_KEYBOARD_DOT | KEY_MOD_LEFT_SHIFT, // >
    HID_KEYBOARD_SLASH | KEY_MOD_LEFT_SHIFT, // ?
    HID_KEYBOARD_2 | KEY_MOD_LEFT_SHIFT, // @
    HID_KEYBOARD_A | KEY_MOD_LEFT_SHIFT, // A
    HID_KEYBOARD_B | KEY_MOD_LEFT_SHIFT, // B
    HID_KEYBOARD_C | KEY_MOD_LEFT_SHIFT, // C
    HID_KEYBOARD_D | KEY_MOD_LEFT_SHIFT, // D
    HID_KEYBOARD_E | KEY_MOD_LEFT_SHIFT, // E
    HID_KEYBOARD_F | KEY_MOD_LEFT_SHIFT, // F
    HID_KEYBOARD_G | KEY_MOD_LEFT_SHIFT, // G
    HID_KEYBOARD_H | KEY_MOD_LEFT_SHIFT, // H
    HID_KEYBOARD_I | KEY_MOD_LEFT_SHIFT, // I
    HID_KEYBOARD_J | KEY_MOD_LEFT_SHIFT, // J
    HID_KEYBOARD_K | KEY_MOD_LEFT_SHIFT, // K
    HID_KEYBOARD_L | KEY_MOD_LEFT_SHIFT, // L
    HID_KEYBOARD_M | KEY_MOD_LEFT_SHIFT, // M
    HID_KEYBOARD_N | KEY_MOD_LEFT_SHIFT, // N
    HID_KEYBOARD_O | KEY_MOD_LEFT_SHIFT, // O
    HID_KEYBOARD_P | KEY_MOD_LEFT_SHIFT, // P
    HID_KEYBOARD_Q | KEY_MOD_LEFT_SHIFT, // Q
    HID_KEYBOARD_R | KEY_MOD_LEFT_SHIFT, // R
    HID_KEYBOARD_S | KEY_MOD_LEFT_SHIFT, // S
    HID_KEYBOARD_T | KEY_MOD_LEFT_SHIFT, // T
    HID_KEYBOARD_U | KEY_MOD_LEFT_SHIFT, // U
    HID_KEYBOARD_V | KEY_MOD_LEFT_SHIFT, // V
    HID_KEYBOARD_W | KEY_MOD_LEFT_SHIFT, // W
    HID_KEYBOARD_X | KEY_MOD_LEFT_SHIFT, // X
    HID_KEYBOARD_Y | KEY_MOD_LEFT_SHIFT, // Y
    HID_KEYBOARD_Z | KEY_MOD_LEFT_SHIFT, // Z
    HID_KEYBOARD_OPEN_BRACKET, // [
    HID_KEYBOARD_BACKSLASH, // bslash
    HID_KEYBOARD_CLOSE_BRACKET, // ]
    HID_KEYBOARD_6 | KEY_MOD_LEFT_SHIFT, // ^
    HID_KEYBOARD_MINUS | KEY_MOD_LEFT_SHIFT, // _
    HID_KEYBOARD_GRAVE_ACCENT, // `
    HID_KEYBOARD_A, // a
    HID_KEYBOARD_B, // b
    HID_KEYBOARD_C, // c
    HID_KEYBOARD_D, // d
    HID_KEYBOARD_E, // e
    HID_KEYBOARD_F, // f
    HID_KEYBOARD_G, // g
    HID_KEYBOARD_H, // h
    HID_KEYBOARD_I, // i
    HID_KEYBOARD_J, // j
    HID_KEYBOARD_K, // k
    HID_KEYBOARD_L, // l
    HID_KEYBOARD_M, // m
    HID_KEYBOARD_N, // n
    HID_KEYBOARD_O, // o
    HID_KEYBOARD_P, // p
    HID_KEYBOARD_Q, // q
    HID_KEYBOARD_R, // r
    HID_KEYBOARD_S, // s
    HID_KEYBOARD_T, // t
    HID_KEYBOARD_U, // u
    HID_KEYBOARD_V, // v
    HID_KEYBOARD_W, // w
    HID_KEYBOARD_X, // x
    HID_KEYBOARD_Y, // y
    HID_KEYBOARD_Z, // z
    HID_KEYBOARD_OPEN_BRACKET | KEY_MOD_LEFT_SHIFT, // {
    HID_KEYBOARD_BACKSLASH | KEY_MOD_LEFT_SHIFT, // |
    HID_KEYBOARD_CLOSE_BRACKET | KEY_MOD_LEFT_SHIFT, // }
    HID_KEYBOARD_GRAVE_ACCENT | KEY_MOD_LEFT_SHIFT, // ~
    HID_KEYBOARD_NONE, // DEL
};

typedef struct {
    uint32_t vid;
    uint32_t pid;
    char manuf[32];
    char product[32];
} FuriHalUsbHidConfig;

typedef void (*HidStateCallback)(bool state, void* context);

/** ASCII to keycode conversion macro */
#define HID_ASCII_TO_KEY(x) (((uint8_t)x < 128) ? (hid_asciimap[(uint8_t)x]) : HID_KEYBOARD_NONE)

/** HID keyboard leds */
enum HidKeyboardLeds {
    HID_KB_LED_NUM = (1 << 0),
    HID_KB_LED_CAPS = (1 << 1),
    HID_KB_LED_SCROLL = (1 << 2),
};

/** HID mouse buttons */
enum HidMouseButtons {
    HID_MOUSE_BTN_LEFT = (1 << 0),
    HID_MOUSE_BTN_RIGHT = (1 << 1),
    HID_MOUSE_BTN_WHEEL = (1 << 2),
};

/** Get USB HID connection state
 *
 * @return      true / false
 */
bool furi_hal_hid_is_connected(void);

/** Get USB HID keyboard leds state
 *
 * @return      leds state
 */
uint8_t furi_hal_hid_get_led_state(void);

/** Set USB HID connect/disconnect callback
 *
 * @param      cb  callback
 * @param      ctx  callback context
 */
void furi_hal_hid_set_state_callback(HidStateCallback cb, void* ctx);

/** Set the following key to pressed state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_hid_kb_press(uint16_t button);

/** Set the following key to released state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_hid_kb_release(uint16_t button);

/** Clear all pressed keys and send HID report
 *
 */
bool furi_hal_hid_kb_release_all(void);

/** Set mouse movement and send HID report
 *
 * @param      dx  x coordinate delta
 * @param      dy  y coordinate delta
 */
bool furi_hal_hid_mouse_move(int8_t dx, int8_t dy);

/** Set mouse button to pressed state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_hid_mouse_press(uint8_t button);

/** Set mouse button to released state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_hid_mouse_release(uint8_t button);

/** Set mouse wheel position and send HID report
 *
 * @param      delta  number of scroll steps
 */
bool furi_hal_hid_mouse_scroll(int8_t delta);

/** Set the following consumer key to pressed state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_hid_consumer_key_press(uint16_t button);

/** Set the following consumer key to released state and send HID report
 *
 * @param      button  key code
 */
bool furi_hal_hid_consumer_key_release(uint16_t button);

/** Clear all pressed consumer keys and send HID report
 *
 */
bool furi_hal_hid_consumer_key_release_all(void);

#ifdef __cplusplus
}
#endif
