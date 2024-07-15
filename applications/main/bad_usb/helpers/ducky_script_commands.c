#include <furi_hal.h>
#include "ducky_script.h"
#include "ducky_script_i.h"

typedef int32_t (*DuckyCmdCallback)(BadUsbScript* bad_usb, const char* line, int32_t param);

typedef struct {
    char* name;
    DuckyCmdCallback callback;
    int32_t param;
} DuckyCmd;

static int32_t ducky_fnc_delay(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint32_t delay_val = 0;
    bool state = ducky_get_number(line, &delay_val);
    if((state) && (delay_val > 0)) {
        return (int32_t)delay_val;
    }

    return ducky_error(bad_usb, "Invalid number %s", line);
}

static int32_t ducky_fnc_defdelay(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &bad_usb->defdelay);
    if(!state) {
        return ducky_error(bad_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_strdelay(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &bad_usb->stringdelay);
    if(!state) {
        return ducky_error(bad_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_defstrdelay(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &bad_usb->defstringdelay);
    if(!state) {
        return ducky_error(bad_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_string(BadUsbScript* bad_usb, const char* line, int32_t param) {
    line = &line[ducky_get_command_len(line) + 1];
    furi_string_set_str(bad_usb->string_print, line);
    if(param == 1) {
        furi_string_cat(bad_usb->string_print, "\n");
    }

    if(bad_usb->stringdelay == 0 &&
       bad_usb->defstringdelay == 0) { // stringdelay not set - run command immediately
        bool state = ducky_string(bad_usb, furi_string_get_cstr(bad_usb->string_print));
        if(!state) {
            return ducky_error(bad_usb, "Invalid string %s", line);
        }
    } else { // stringdelay is set - run command in thread to keep handling external events
        return SCRIPT_STATE_STRING_START;
    }

    return 0;
}

static int32_t ducky_fnc_repeat(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &bad_usb->repeat_cnt);
    if((!state) || (bad_usb->repeat_cnt == 0)) {
        return ducky_error(bad_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_sysrq(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(bad_usb, line, true);
    bad_usb->hid->kb_press(bad_usb->hid_inst, KEY_MOD_LEFT_ALT | HID_KEYBOARD_PRINT_SCREEN);
    bad_usb->hid->kb_press(bad_usb->hid_inst, key);
    bad_usb->hid->release_all(bad_usb->hid_inst);
    return 0;
}

static int32_t ducky_fnc_altchar(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    ducky_numlock_on(bad_usb);
    bool state = ducky_altchar(bad_usb, line);
    if(!state) {
        return ducky_error(bad_usb, "Invalid altchar %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_altstring(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    ducky_numlock_on(bad_usb);
    bool state = ducky_altstring(bad_usb, line);
    if(!state) {
        return ducky_error(bad_usb, "Invalid altstring %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_hold(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(bad_usb, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(bad_usb, "No keycode defined for %s", line);
    }
    bad_usb->key_hold_nb++;
    if(bad_usb->key_hold_nb > (HID_KB_MAX_KEYS - 1)) {
        return ducky_error(bad_usb, "Too many keys are hold");
    }
    bad_usb->hid->kb_press(bad_usb->hid_inst, key);
    return 0;
}

static int32_t ducky_fnc_release(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(bad_usb, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(bad_usb, "No keycode defined for %s", line);
    }
    if(bad_usb->key_hold_nb == 0) {
        return ducky_error(bad_usb, "No keys are hold");
    }
    bad_usb->key_hold_nb--;
    bad_usb->hid->kb_release(bad_usb->hid_inst, key);
    return 0;
}

static int32_t ducky_fnc_media(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_media_keycode_by_name(line);
    if(key == HID_CONSUMER_UNASSIGNED) {
        return ducky_error(bad_usb, "No keycode defined for %s", line);
    }
    bad_usb->hid->consumer_press(bad_usb->hid_inst, key);
    bad_usb->hid->consumer_release(bad_usb->hid_inst, key);
    return 0;
}

static int32_t ducky_fnc_globe(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(bad_usb, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(bad_usb, "No keycode defined for %s", line);
    }

    bad_usb->hid->consumer_press(bad_usb->hid_inst, HID_CONSUMER_FN_GLOBE);
    bad_usb->hid->kb_press(bad_usb->hid_inst, key);
    bad_usb->hid->kb_release(bad_usb->hid_inst, key);
    bad_usb->hid->consumer_release(bad_usb->hid_inst, HID_CONSUMER_FN_GLOBE);
    return 0;
}

static int32_t ducky_fnc_waitforbutton(BadUsbScript* bad_usb, const char* line, int32_t param) {
    UNUSED(param);
    UNUSED(bad_usb);
    UNUSED(line);

    return SCRIPT_STATE_WAIT_FOR_BTN;
}

static const DuckyCmd ducky_commands[] = {
    {"REM", NULL, -1},
    {"ID", NULL, -1},
    {"DELAY", ducky_fnc_delay, -1},
    {"STRING", ducky_fnc_string, 0},
    {"STRINGLN", ducky_fnc_string, 1},
    {"DEFAULT_DELAY", ducky_fnc_defdelay, -1},
    {"DEFAULTDELAY", ducky_fnc_defdelay, -1},
    {"STRINGDELAY", ducky_fnc_strdelay, -1},
    {"STRING_DELAY", ducky_fnc_strdelay, -1},
    {"DEFAULT_STRING_DELAY", ducky_fnc_defstrdelay, -1},
    {"DEFAULTSTRINGDELAY", ducky_fnc_defstrdelay, -1},
    {"REPEAT", ducky_fnc_repeat, -1},
    {"SYSRQ", ducky_fnc_sysrq, -1},
    {"ALTCHAR", ducky_fnc_altchar, -1},
    {"ALTSTRING", ducky_fnc_altstring, -1},
    {"ALTCODE", ducky_fnc_altstring, -1},
    {"HOLD", ducky_fnc_hold, -1},
    {"RELEASE", ducky_fnc_release, -1},
    {"WAIT_FOR_BUTTON_PRESS", ducky_fnc_waitforbutton, -1},
    {"MEDIA", ducky_fnc_media, -1},
    {"GLOBE", ducky_fnc_globe, -1},
};

#define TAG "BadUsb"

#define WORKER_TAG TAG "Worker"

int32_t ducky_execute_cmd(BadUsbScript* bad_usb, const char* line) {
    size_t cmd_word_len = strcspn(line, " ");
    for(size_t i = 0; i < COUNT_OF(ducky_commands); i++) {
        size_t cmd_compare_len = strlen(ducky_commands[i].name);

        if(cmd_compare_len != cmd_word_len) {
            continue;
        }

        if(strncmp(line, ducky_commands[i].name, cmd_compare_len) == 0) {
            if(ducky_commands[i].callback == NULL) {
                return 0;
            } else {
                return (ducky_commands[i].callback)(bad_usb, line, ducky_commands[i].param);
            }
        }
    }

    return SCRIPT_STATE_CMD_UNKNOWN;
}
