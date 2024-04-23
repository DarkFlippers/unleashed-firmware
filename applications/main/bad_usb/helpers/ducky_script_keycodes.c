#include <furi_hal.h>
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
    {"F13", HID_KEYBOARD_F13},
    {"F14", HID_KEYBOARD_F14},
    {"F15", HID_KEYBOARD_F15},
    {"F16", HID_KEYBOARD_F16},
    {"F17", HID_KEYBOARD_F17},
    {"F18", HID_KEYBOARD_F18},
    {"F19", HID_KEYBOARD_F19},
    {"F20", HID_KEYBOARD_F20},
    {"F21", HID_KEYBOARD_F21},
    {"F22", HID_KEYBOARD_F22},
    {"F23", HID_KEYBOARD_F23},
    {"F24", HID_KEYBOARD_F24},
};

static const DuckyKey ducky_media_keys[] = {
    {"POWER", HID_CONSUMER_POWER},
    {"REBOOT", HID_CONSUMER_RESET},
    {"SLEEP", HID_CONSUMER_SLEEP},
    {"LOGOFF", HID_CONSUMER_AL_LOGOFF},

    {"EXIT", HID_CONSUMER_AC_EXIT},
    {"HOME", HID_CONSUMER_AC_HOME},
    {"BACK", HID_CONSUMER_AC_BACK},
    {"FORWARD", HID_CONSUMER_AC_FORWARD},
    {"REFRESH", HID_CONSUMER_AC_REFRESH},

    {"SNAPSHOT", HID_CONSUMER_SNAPSHOT},

    {"PLAY", HID_CONSUMER_PLAY},
    {"PAUSE", HID_CONSUMER_PAUSE},
    {"PLAY_PAUSE", HID_CONSUMER_PLAY_PAUSE},
    {"NEXT_TRACK", HID_CONSUMER_SCAN_NEXT_TRACK},
    {"PREV_TRACK", HID_CONSUMER_SCAN_PREVIOUS_TRACK},
    {"STOP", HID_CONSUMER_STOP},
    {"EJECT", HID_CONSUMER_EJECT},

    {"MUTE", HID_CONSUMER_MUTE},
    {"VOLUME_UP", HID_CONSUMER_VOLUME_INCREMENT},
    {"VOLUME_DOWN", HID_CONSUMER_VOLUME_DECREMENT},

    {"FN", HID_CONSUMER_FN_GLOBE},
    {"BRIGHT_UP", HID_CONSUMER_BRIGHTNESS_INCREMENT},
    {"BRIGHT_DOWN", HID_CONSUMER_BRIGHTNESS_DECREMENT},
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

uint16_t ducky_get_media_keycode_by_name(const char* param) {
    for(size_t i = 0; i < COUNT_OF(ducky_media_keys); i++) {
        size_t key_cmd_len = strlen(ducky_media_keys[i].name);
        if((strncmp(param, ducky_media_keys[i].name, key_cmd_len) == 0) &&
           (ducky_is_line_end(param[key_cmd_len]))) {
            return ducky_media_keys[i].keycode;
        }
    }

    return HID_CONSUMER_UNASSIGNED;
}
