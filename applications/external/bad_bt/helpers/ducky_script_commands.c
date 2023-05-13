#include <furi_hal.h>
#include <furi_hal_bt_hid.h>
#include "ducky_script.h"
#include "ducky_script_i.h"

typedef int32_t (*DuckyCmdCallback)(BadBtScript* bad_bt, const char* line, int32_t param);

typedef struct {
    char* name;
    DuckyCmdCallback callback;
    int32_t param;
} DuckyCmd;

static int32_t ducky_fnc_delay(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint32_t delay_val = 0;
    bool state = ducky_get_number(line, &delay_val);
    if((state) && (delay_val > 0)) {
        return (int32_t)delay_val;
    }

    return ducky_error(bad_bt, "Invalid number %s", line);
}

static int32_t ducky_fnc_defdelay(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &bad_bt->defdelay);
    if(!state) {
        return ducky_error(bad_bt, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_strdelay(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &bad_bt->stringdelay);
    if(!state) {
        return ducky_error(bad_bt, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_string(BadBtScript* bad_bt, const char* line, int32_t param) {
    line = &line[ducky_get_command_len(line) + 1];
    furi_string_set_str(bad_bt->string_print, line);
    if(param == 1) {
        furi_string_cat(bad_bt->string_print, "\n");
    }

    if(bad_bt->stringdelay == 0) { // stringdelay not set - run command immidiately
        bool state = ducky_string(bad_bt, furi_string_get_cstr(bad_bt->string_print));
        if(!state) {
            return ducky_error(bad_bt, "Invalid string %s", line);
        }
    } else { // stringdelay is set - run command in thread to keep handling external events
        return SCRIPT_STATE_STRING_START;
    }

    return 0;
}

static int32_t ducky_fnc_repeat(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &bad_bt->repeat_cnt);
    if((!state) || (bad_bt->repeat_cnt == 0)) {
        return ducky_error(bad_bt, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_sysrq(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(bad_bt, line, true);

    furi_hal_bt_hid_kb_press(KEY_MOD_LEFT_ALT | HID_KEYBOARD_PRINT_SCREEN);
    furi_hal_bt_hid_kb_press(key);
    furi_delay_ms(bt_timeout);
    furi_hal_bt_hid_kb_release(key);
    furi_hal_bt_hid_kb_release(KEY_MOD_LEFT_ALT | HID_KEYBOARD_PRINT_SCREEN);
    return 0;
}

static int32_t ducky_fnc_altchar(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    ducky_numlock_on(bad_bt);
    bool state = ducky_altchar(bad_bt, line);
    if(!state) {
        return ducky_error(bad_bt, "Invalid altchar %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_altstring(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    ducky_numlock_on(bad_bt);
    bool state = ducky_altstring(bad_bt, line);
    if(!state) {
        return ducky_error(bad_bt, "Invalid altstring %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_hold(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(bad_bt, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(bad_bt, "No keycode defined for %s", line);
    }
    bad_bt->key_hold_nb++;
    if(bad_bt->key_hold_nb > (HID_KB_MAX_KEYS - 1)) {
        return ducky_error(bad_bt, "Too many keys are hold");
    }
    furi_hal_bt_hid_kb_press(key);

    return 0;
}

static int32_t ducky_fnc_release(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(bad_bt, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(bad_bt, "No keycode defined for %s", line);
    }
    if(bad_bt->key_hold_nb == 0) {
        return ducky_error(bad_bt, "No keys are hold");
    }
    bad_bt->key_hold_nb--;
    furi_hal_bt_hid_kb_release(key);
    return 0;
}

static int32_t ducky_fnc_waitforbutton(BadBtScript* bad_bt, const char* line, int32_t param) {
    UNUSED(param);
    UNUSED(bad_bt);
    UNUSED(line);

    return SCRIPT_STATE_WAIT_FOR_BTN;
}

static const DuckyCmd ducky_commands[] = {
    {"REM", NULL, -1},
    {"ID", NULL, -1},
    {"BT_ID", NULL, -1},
    {"DELAY", ducky_fnc_delay, -1},
    {"STRING", ducky_fnc_string, 0},
    {"STRINGLN", ducky_fnc_string, 1},
    {"DEFAULT_DELAY", ducky_fnc_defdelay, -1},
    {"DEFAULTDELAY", ducky_fnc_defdelay, -1},
    {"STRINGDELAY", ducky_fnc_strdelay, -1},
    {"STRING_DELAY", ducky_fnc_strdelay, -1},
    {"REPEAT", ducky_fnc_repeat, -1},
    {"SYSRQ", ducky_fnc_sysrq, -1},
    {"ALTCHAR", ducky_fnc_altchar, -1},
    {"ALTSTRING", ducky_fnc_altstring, -1},
    {"ALTCODE", ducky_fnc_altstring, -1},
    {"HOLD", ducky_fnc_hold, -1},
    {"RELEASE", ducky_fnc_release, -1},
    {"WAIT_FOR_BUTTON_PRESS", ducky_fnc_waitforbutton, -1},
};

#define TAG "BadBT"
#define WORKER_TAG TAG "Worker"

int32_t ducky_execute_cmd(BadBtScript* bad_bt, const char* line) {
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
                return ((ducky_commands[i].callback)(bad_bt, line, ducky_commands[i].param));
            }
        }
    }

    return SCRIPT_STATE_CMD_UNKNOWN;
}
