#pragma once
#include "hid_usage_desktop.h"
#include "hid_usage_button.h"
#include "hid_usage_keyboard.h"
#include "hid_usage_consumer.h"
#include "hid_usage_led.h"

#define HID_KEYBOARD_NONE 0x00
// Remapping the colon key which is shift + ; to comma
#define HID_KEYBOARD_COMMA HID_KEYBOARD_COLON

/** HID keyboard key codes AZERTY */
enum HidKeyboardKeysAZERTY {
    KEY_NONE = 0x00,
    KEY_ERROR_ROLLOVER = 0x01,
    KEY_POST_FAIL = 0x02,
    KEY_ERROR_UNDEFINED = 0x03,

    KEY_ENTER = 0x28,
    KEY_ESC = 0x29,
    KEY_BACKSPACE = 0x2A,
    KEY_TAB = 0x2B,
    KEY_SPACE = 0x2C,

    KEY_A = 0x14,
    KEY_B = 0x05,
    KEY_C = 0x06,
    KEY_D = 0x07,
    KEY_E = 0x08,
    KEY_F = 0x09,
    KEY_G = 0x0A,
    KEY_H = 0x0B,
    KEY_I = 0x0C,
    KEY_J = 0x0D,
    KEY_K = 0x0E,
    KEY_L = 0x0F,
    KEY_M = 0x33,
    KEY_N = 0x11,
    KEY_O = 0x12,
    KEY_P = 0x13,
    KEY_Q = 0x04,
    KEY_R = 0x15,
    KEY_S = 0x16,
    KEY_T = 0x17,
    KEY_U = 0x18,
    KEY_V = 0x19,
    KEY_W = 0x1D,
    KEY_X = 0x1B,
    KEY_Y = 0x1C,
    KEY_Z = 0x1A,

    KEY_1 = 0x1E,
    KEY_2 = 0x1F,
    KEY_3 = 0x20,
    KEY_4 = 0x21,
    KEY_5 = 0x22,
    KEY_6 = 0x23,
    KEY_7 = 0x24,
    KEY_8 = 0x25,
    KEY_9 = 0x26,
    KEY_0 = 0x27,

    KEY_EXCLAMATION = 0x38,
    KEY_DOUBLE_QUOTE = 0x20,
    KEY_DOLLAR = 0x30,
    KEY_U_BACKTICK = 0x34,
    KEY_AND = 0x1E,
    KEY_SINGLE_QUOTE = 0x21,
    KEY_LEFT_PARENTHESIS = 0x22,
    KEY_RIGHT_PARENTHESIS = 0x2D,
    KEY_STAR = 0x31,
    KEY_EQUAL = 0x2E,
    KEY_COMMA = 0x10,
    KEY_DASH = 0x23,
    KEY_SEMI_COLON = 0x36,
    KEY_DOUBLE_POINTS = 0x37,
    KEY_SMALLER = 0x64,
    KEY_UNDERSCORE = 0x25,
    KEY_CIRCUMFLEX = 0x2F,
    KEY_A_BACKTICK = 0x27,
    KEY_E_ACCENT = 0x1F,
    KEY_E_BACKTICK = 0x24,
    KEY_C_CEDILLE = 0x26,

    KEY_CAPS_LOCK = 0xC1,
    KEY_F1 = 0xC2,
    KEY_F2 = 0xC3,
    KEY_F3 = 0xC4,
    KEY_F4 = 0xC5,
    KEY_F5 = 0xC6,
    KEY_F6 = 0xC7,
    KEY_F7 = 0xC8,
    KEY_F8 = 0xC9,
    KEY_F9 = 0xCA,
    KEY_F10 = 0xCB,
    KEY_F11 = 0xCC,
    KEY_F12 = 0xCD,
    KEY_PRINT = 0x63,
    KEY_SCROLL_LOCK = 0x47,
    KEY_PAUSE = 0x48,
    KEY_INSERT = 0xD1,
    KEY_HOME = 0xD2,
    KEY_PAGE_UP = 0xD3,
    KEY_DELETE = 0xD4,
    KEY_END = 0xD5,
    KEY_PAGE_DOWN = 0xD6,
    KEY_RIGHT_ARROW = 0xD7,
    KEY_LEFT_ARROW = 0xD8,
    KEY_DOWN_ARROW = 0xD9,
    KEY_UP_ARROW = 0xDA,
    KEY_NUM_LOCK = 0x53,
    KEYPAD_DIVIDE = 0x54,
    KEYPAD_MULTIPLY = 0x55,
    KEYPAD_SUBTRACT = 0x56,
    KEYPAD_ADD = 0x57,
    KEYPAD_ENTER = 0x58,
    KEYPAD_1 = 0x59,
    KEYPAD_2 = 0x5A,
    KEYPAD_3 = 0x5B,
    KEYPAD_4 = 0x5C,
    KEYPAD_5 = 0x5D,
    KEYPAD_6 = 0x5E,
    KEYPAD_7 = 0x5F,
    KEYPAD_8 = 0x60,
    KEYPAD_9 = 0x61,
    KEYPAD_0 = 0x62,
    KEYPAD_DOT = 0x63,
    KEY_NON_US = 0x64,
    KEY_APPLICATION = 0x65,
};

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

/** ASCII to keycode conversion table AZERTY */
static const uint16_t hid_asciimap_azerty[] = {
    KEY_NONE, // NUL
    KEY_NONE, // SOH
    KEY_NONE, // STX
    KEY_NONE, // ETX
    KEY_NONE, // EOT
    KEY_NONE, // ENQ
    KEY_NONE, // ACK
    KEY_NONE, // BEL
    KEY_BACKSPACE, // BS   Backspace
    KEY_TAB, // TAB  Tab
    KEY_ENTER, // LF   Enter
    KEY_NONE, // VT
    KEY_NONE, // FF
    KEY_NONE, // CR
    KEY_NONE, // SO
    KEY_NONE, // SI
    KEY_NONE, // DEL
    KEY_NONE, // DC1
    KEY_NONE, // DC2
    KEY_NONE, // DC3
    KEY_NONE, // DC4
    KEY_NONE, // NAK
    KEY_NONE, // SYN
    KEY_NONE, // ETB
    KEY_NONE, // CAN
    KEY_NONE, // EM
    KEY_NONE, // SUB
    KEY_NONE, // ESC
    KEY_NONE, // FS
    KEY_NONE, // GS
    KEY_NONE, // RS
    KEY_NONE, // US
    KEY_SPACE, // ' ' Space
    KEY_EXCLAMATION, // !
    KEY_DOUBLE_QUOTE, // "
    KEY_DOUBLE_QUOTE | KEY_MOD_RIGHT_ALT, // #
    KEY_DOLLAR, // $
    KEY_U_BACKTICK | KEY_MOD_LEFT_SHIFT, // %
    KEY_AND, // &
    KEY_SINGLE_QUOTE, // '
    KEY_LEFT_PARENTHESIS, // (
    KEY_RIGHT_PARENTHESIS, // )
    KEY_STAR, // *
    KEY_EQUAL | KEY_MOD_LEFT_SHIFT, // +
    KEY_COMMA, // ,
    KEY_DASH, // -
    KEY_SEMI_COLON | KEY_MOD_LEFT_SHIFT, // .
    KEY_DOUBLE_POINTS | KEY_MOD_LEFT_SHIFT, // /
    KEY_A_BACKTICK | KEY_MOD_LEFT_SHIFT, // 0
    KEY_AND | KEY_MOD_LEFT_SHIFT, // 1
    KEY_E_ACCENT | KEY_MOD_LEFT_SHIFT, // 2
    KEY_DOUBLE_QUOTE | KEY_MOD_LEFT_SHIFT, // 3
    KEY_SINGLE_QUOTE | KEY_MOD_LEFT_SHIFT, // 4
    KEY_LEFT_PARENTHESIS | KEY_MOD_LEFT_SHIFT, // 5
    KEY_DASH | KEY_MOD_LEFT_SHIFT, // 6
    KEY_E_BACKTICK | KEY_MOD_LEFT_SHIFT, // 7
    KEY_UNDERSCORE | KEY_MOD_LEFT_SHIFT, // 8
    KEY_C_CEDILLE | KEY_MOD_LEFT_SHIFT, // 9
    KEY_DOUBLE_POINTS, // :
    KEY_SEMI_COLON, // ;
    KEY_SMALLER, // <
    KEY_EQUAL, // =
    KEY_SMALLER | KEY_MOD_LEFT_SHIFT, // >
    KEY_COMMA | KEY_MOD_LEFT_SHIFT, // ?
    KEY_A_BACKTICK | KEY_MOD_RIGHT_ALT, // @
    KEY_A | KEY_MOD_LEFT_SHIFT, // A
    KEY_B | KEY_MOD_LEFT_SHIFT, // B
    KEY_C | KEY_MOD_LEFT_SHIFT, // C
    KEY_D | KEY_MOD_LEFT_SHIFT, // D
    KEY_E | KEY_MOD_LEFT_SHIFT, // E
    KEY_F | KEY_MOD_LEFT_SHIFT, // F
    KEY_G | KEY_MOD_LEFT_SHIFT, // G
    KEY_H | KEY_MOD_LEFT_SHIFT, // H
    KEY_I | KEY_MOD_LEFT_SHIFT, // I
    KEY_J | KEY_MOD_LEFT_SHIFT, // J
    KEY_K | KEY_MOD_LEFT_SHIFT, // K
    KEY_L | KEY_MOD_LEFT_SHIFT, // L
    KEY_M | KEY_MOD_LEFT_SHIFT, // M
    KEY_N | KEY_MOD_LEFT_SHIFT, // N
    KEY_O | KEY_MOD_LEFT_SHIFT, // O
    KEY_P | KEY_MOD_LEFT_SHIFT, // P
    KEY_Q | KEY_MOD_LEFT_SHIFT, // Q
    KEY_R | KEY_MOD_LEFT_SHIFT, // R
    KEY_S | KEY_MOD_LEFT_SHIFT, // S
    KEY_T | KEY_MOD_LEFT_SHIFT, // T
    KEY_U | KEY_MOD_LEFT_SHIFT, // U
    KEY_V | KEY_MOD_LEFT_SHIFT, // V
    KEY_W | KEY_MOD_LEFT_SHIFT, // W
    KEY_X | KEY_MOD_LEFT_SHIFT, // X
    KEY_Y | KEY_MOD_LEFT_SHIFT, // Y
    KEY_Z | KEY_MOD_LEFT_SHIFT, // Z
    KEY_LEFT_PARENTHESIS | KEY_MOD_RIGHT_ALT, // [
    KEY_UNDERSCORE | KEY_MOD_RIGHT_ALT, // bslash
    KEY_RIGHT_PARENTHESIS | KEY_MOD_RIGHT_ALT, // ]
    KEY_CIRCUMFLEX, // ^
    KEY_UNDERSCORE, // _
    KEY_E_BACKTICK | KEY_MOD_RIGHT_ALT, // `
    KEY_A, // a
    KEY_B, // b
    KEY_C, // c
    KEY_D, // d
    KEY_E, // e
    KEY_F, // f
    KEY_G, // g
    KEY_H, // h
    KEY_I, // i
    KEY_J, // j
    KEY_K, // k
    KEY_L, // l
    KEY_M, // m
    KEY_N, // n
    KEY_O, // o
    KEY_P, // p
    KEY_Q, // q
    KEY_R, // r
    KEY_S, // s
    KEY_T, // t
    KEY_U, // u
    KEY_V, // v
    KEY_W, // w
    KEY_X, // x
    KEY_Y, // y
    KEY_Z, // z
    KEY_SINGLE_QUOTE | KEY_MOD_RIGHT_ALT, // {
    KEY_DASH | KEY_MOD_RIGHT_ALT, // |
    KEY_EQUAL | KEY_MOD_RIGHT_ALT, // }
    KEY_E_ACCENT | KEY_MOD_RIGHT_ALT, // ~
    KEY_NONE, // DEL
};

static const uint16_t* hid_asciimaps[] = {
    hid_asciimap, 
    hid_asciimap_azerty
};

typedef struct {
    uint32_t vid;
    uint32_t pid;
    char manuf[32];
    char product[32];
} FuriHalUsbHidConfig;

typedef void (*HidStateCallback)(bool state, void* context);

/** ASCII to keycode conversion macro */
#define HID_ASCII_TO_KEY(x,y) (((uint8_t)y < 128) ? (hid_asciimaps[(uint8_t)x][(uint8_t)y]) : HID_KEYBOARD_NONE)

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
bool furi_hal_hid_is_connected();

/** Get USB HID keyboard leds state
 *
 * @return      leds state
 */
uint8_t furi_hal_hid_get_led_state();

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
bool furi_hal_hid_kb_release_all();

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
