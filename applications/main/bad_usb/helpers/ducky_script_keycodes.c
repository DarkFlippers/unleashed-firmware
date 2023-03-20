#include <furi_hal.h>
#include <furi_hal_usb_hid.h>
#include "ducky_script_i.h"

typedef struct {
    char* name;
    uint16_t keycode;
} DuckyKey;

static const DuckyKey ducky_keys[] = {
    {"CTRL-ALT", KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_ALT},
    {"CTRL-SHIFT", KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT},
    {"ALT-SHIFT", KEY_MOD_LEFT_ALT | KEY_MOD_LEFT_SHIFT},
    {"ALT-GUI", KEY_MOD_LEFT_ALT | KEY_MOD_LEFT_GUI},
    {"GUI-SHIFT", KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT},
    {"GUI-CTRL", KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_CTRL},

    {"CTRL", KEY_MOD_LEFT_CTRL},
    {"CONTROL", KEY_MOD_LEFT_CTRL},
    {"SHIFT", KEY_MOD_LEFT_SHIFT},
    {"ALT", KEY_MOD_LEFT_ALT},
    {"GUI", KEY_MOD_LEFT_GUI},
    {"WINDOWS", KEY_MOD_LEFT_GUI},

    {"DOWNARROW", HID_KEYBOARD_DOWN_ARROW},
    {"DOWN", HID_KEYBOARD_DOWN_ARROW},
    {"LEFTARROW", HID_KEYBOARD_LEFT_ARROW},
    {"LEFT", HID_KEYBOARD_LEFT_ARROW},
    {"RIGHTARROW", HID_KEYBOARD_RIGHT_ARROW},
    {"RIGHT", HID_KEYBOARD_RIGHT_ARROW},
    {"UPARROW", HID_KEYBOARD_UP_ARROW},
    {"UP", HID_KEYBOARD_UP_ARROW},

    {"ENTER", HID_KEYBOARD_RETURN},
    {"BREAK", HID_KEYBOARD_PAUSE},
    {"PAUSE", HID_KEYBOARD_PAUSE},
    {"CAPSLOCK", HID_KEYBOARD_CAPS_LOCK},
    {"DELETE", HID_KEYBOARD_DELETE_FORWARD},
    {"BACKSPACE", HID_KEYBOARD_DELETE},
    {"END", HID_KEYBOARD_END},
    {"ESC", HID_KEYBOARD_ESCAPE},
    {"ESCAPE", HID_KEYBOARD_ESCAPE},
    {"HOME", HID_KEYBOARD_HOME},
    {"INSERT", HID_KEYBOARD_INSERT},
    {"NUMLOCK", HID_KEYPAD_NUMLOCK},
    {"PAGEUP", HID_KEYBOARD_PAGE_UP},
    {"PAGEDOWN", HID_KEYBOARD_PAGE_DOWN},
    {"PRINTSCREEN", HID_KEYBOARD_PRINT_SCREEN},
    {"SCROLLLOCK", HID_KEYBOARD_SCROLL_LOCK},
    {"SPACE", HID_KEYBOARD_SPACEBAR},
    {"TAB", HID_KEYBOARD_TAB},
    {"MENU", HID_KEYBOARD_APPLICATION},
    {"APP", HID_KEYBOARD_APPLICATION},

    {"F1", HID_KEYBOARD_F1},
    {"F2", HID_KEYBOARD_F2},
    {"F3", HID_KEYBOARD_F3},
    {"F4", HID_KEYBOARD_F4},
    {"F5", HID_KEYBOARD_F5},
    {"F6", HID_KEYBOARD_F6},
    {"F7", HID_KEYBOARD_F7},
    {"F8", HID_KEYBOARD_F8},
    {"F9", HID_KEYBOARD_F9},
    {"F10", HID_KEYBOARD_F10},
    {"F11", HID_KEYBOARD_F11},
    {"F12", HID_KEYBOARD_F12},
};

uint16_t ducky_get_keycode_by_name(const char* param) {
    for(size_t i = 0; i < COUNT_OF(ducky_keys); i++) {
        size_t key_cmd_len = strlen(ducky_keys[i].name);
        if((strncmp(param, ducky_keys[i].name, key_cmd_len) == 0) &&
           (ducky_is_line_end(param[key_cmd_len]))) {
            return ducky_keys[i].keycode;
        }
    }

    return HID_KEYBOARD_NONE;
}
