#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/toolbox/args.h>
#include <furi_hal_usb_hid.h>
#include <storage/storage.h>
#include "bad_usb_script.h"
#include <dolphin/dolphin.h>

#define TAG "BadUSB"
#define WORKER_TAG TAG "Worker"
#define FILE_BUFFER_LEN 16

#define SCRIPT_STATE_ERROR (-1)
#define SCRIPT_STATE_END (-2)
#define SCRIPT_STATE_NEXT_LINE (-3)

typedef enum {
    WorkerEvtToggle = (1 << 0),
    WorkerEvtEnd = (1 << 1),
    WorkerEvtConnect = (1 << 2),
    WorkerEvtDisconnect = (1 << 3),
} WorkerEvtFlags;

struct BadUsbScript {
    FuriHalUsbHidConfig hid_cfg;
    BadUsbState st;
    FuriString* file_path;
    uint32_t defdelay;
    FuriThread* thread;
    uint8_t file_buf[FILE_BUFFER_LEN + 1];
    uint8_t buf_start;
    uint8_t buf_len;
    bool file_end;
    FuriString* line;

    FuriString* line_prev;
    uint32_t repeat_cnt;
};

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

static const char ducky_cmd_comment[] = {"REM"};
static const char ducky_cmd_id[] = {"ID"};
static const char ducky_cmd_delay[] = {"DELAY "};
static const char ducky_cmd_string[] = {"STRING "};
static const char ducky_cmd_defdelay_1[] = {"DEFAULT_DELAY "};
static const char ducky_cmd_defdelay_2[] = {"DEFAULTDELAY "};
static const char ducky_cmd_repeat[] = {"REPEAT "};
static const char ducky_cmd_sysrq[] = {"SYSRQ "};

static const char ducky_cmd_altchar[] = {"ALTCHAR "};
static const char ducky_cmd_altstr_1[] = {"ALTSTRING "};
static const char ducky_cmd_altstr_2[] = {"ALTCODE "};

static const uint8_t numpad_keys[10] = {
    HID_KEYPAD_0,
    HID_KEYPAD_1,
    HID_KEYPAD_2,
    HID_KEYPAD_3,
    HID_KEYPAD_4,
    HID_KEYPAD_5,
    HID_KEYPAD_6,
    HID_KEYPAD_7,
    HID_KEYPAD_8,
    HID_KEYPAD_9,
};

static bool ducky_get_number(const char* param, uint32_t* val) {
    uint32_t value = 0;
    if(sscanf(param, "%lu", &value) == 1) {
        *val = value;
        return true;
    }
    return false;
}

static uint32_t ducky_get_command_len(const char* line) {
    uint32_t len = strlen(line);
    for(uint32_t i = 0; i < len; i++) {
        if(line[i] == ' ') return i;
    }
    return 0;
}

static bool ducky_is_line_end(const char chr) {
    return ((chr == ' ') || (chr == '\0') || (chr == '\r') || (chr == '\n'));
}

static void ducky_numlock_on() {
    if((furi_hal_hid_get_led_state() & HID_KB_LED_NUM) == 0) {
        furi_hal_hid_kb_press(HID_KEYBOARD_LOCK_NUM_LOCK);
        furi_hal_hid_kb_release(HID_KEYBOARD_LOCK_NUM_LOCK);
    }
}

static bool ducky_numpad_press(const char num) {
    if((num < '0') || (num > '9')) return false;

    uint16_t key = numpad_keys[num - '0'];
    furi_hal_hid_kb_press(key);
    furi_hal_hid_kb_release(key);

    return true;
}

static bool ducky_altchar(const char* charcode) {
    uint8_t i = 0;
    bool state = false;

    FURI_LOG_I(WORKER_TAG, "char %s", charcode);

    furi_hal_hid_kb_press(KEY_MOD_LEFT_ALT);

    while(!ducky_is_line_end(charcode[i])) {
        state = ducky_numpad_press(charcode[i]);
        if(state == false) break;
        i++;
    }

    furi_hal_hid_kb_release(KEY_MOD_LEFT_ALT);
    return state;
}

static bool ducky_altstring(const char* param) {
    uint32_t i = 0;
    bool state = false;

    while(param[i] != '\0') {
        if((param[i] < ' ') || (param[i] > '~')) {
            i++;
            continue; // Skip non-printable chars
        }

        char temp_str[4];
        snprintf(temp_str, 4, "%u", param[i]);

        state = ducky_altchar(temp_str);
        if(state == false) break;
        i++;
    }
    return state;
}

static bool ducky_string(const char* param) {
    uint32_t i = 0;
    while(param[i] != '\0') {
        uint16_t keycode = HID_ASCII_TO_KEY(param[i]);
        if(keycode != HID_KEYBOARD_NONE) {
            furi_hal_hid_kb_press(keycode);
            furi_hal_hid_kb_release(keycode);
        }
        i++;
    }
    return true;
}

static uint16_t ducky_get_keycode(const char* param, bool accept_chars) {
    for(size_t i = 0; i < (sizeof(ducky_keys) / sizeof(ducky_keys[0])); i++) {
        size_t key_cmd_len = strlen(ducky_keys[i].name);
        if((strncmp(param, ducky_keys[i].name, key_cmd_len) == 0) &&
           (ducky_is_line_end(param[key_cmd_len]))) {
            return ducky_keys[i].keycode;
        }
    }
    if((accept_chars) && (strlen(param) > 0)) {
        return (HID_ASCII_TO_KEY(param[0]) & 0xFF);
    }
    return 0;
}

static int32_t
    ducky_parse_line(BadUsbScript* bad_usb, FuriString* line, char* error, size_t error_len) {
    uint32_t line_len = furi_string_size(line);
    const char* line_tmp = furi_string_get_cstr(line);
    bool state = false;

    if(line_len == 0) {
        return SCRIPT_STATE_NEXT_LINE; // Skip empty lines
    }

    FURI_LOG_D(WORKER_TAG, "line:%s", line_tmp);

    // General commands
    if(strncmp(line_tmp, ducky_cmd_comment, strlen(ducky_cmd_comment)) == 0) {
        // REM - comment line
        return (0);
    } else if(strncmp(line_tmp, ducky_cmd_id, strlen(ducky_cmd_id)) == 0) {
        // ID - executed in ducky_script_preload
        return (0);
    } else if(strncmp(line_tmp, ducky_cmd_delay, strlen(ducky_cmd_delay)) == 0) {
        // DELAY
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        uint32_t delay_val = 0;
        state = ducky_get_number(line_tmp, &delay_val);
        if((state) && (delay_val > 0)) {
            return (int32_t)delay_val;
        }
        if(error != NULL) {
            snprintf(error, error_len, "Invalid number %s", line_tmp);
        }
        return SCRIPT_STATE_ERROR;
    } else if(
        (strncmp(line_tmp, ducky_cmd_defdelay_1, strlen(ducky_cmd_defdelay_1)) == 0) ||
        (strncmp(line_tmp, ducky_cmd_defdelay_2, strlen(ducky_cmd_defdelay_2)) == 0)) {
        // DEFAULT_DELAY
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        state = ducky_get_number(line_tmp, &bad_usb->defdelay);
        if(!state && error != NULL) {
            snprintf(error, error_len, "Invalid number %s", line_tmp);
        }
        return (state) ? (0) : SCRIPT_STATE_ERROR;
    } else if(strncmp(line_tmp, ducky_cmd_string, strlen(ducky_cmd_string)) == 0) {
        // STRING
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        state = ducky_string(line_tmp);
        if(!state && error != NULL) {
            snprintf(error, error_len, "Invalid string %s", line_tmp);
        }
        return (state) ? (0) : SCRIPT_STATE_ERROR;
    } else if(strncmp(line_tmp, ducky_cmd_altchar, strlen(ducky_cmd_altchar)) == 0) {
        // ALTCHAR
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        ducky_numlock_on();
        state = ducky_altchar(line_tmp);
        if(!state && error != NULL) {
            snprintf(error, error_len, "Invalid altchar %s", line_tmp);
        }
        return (state) ? (0) : SCRIPT_STATE_ERROR;
    } else if(
        (strncmp(line_tmp, ducky_cmd_altstr_1, strlen(ducky_cmd_altstr_1)) == 0) ||
        (strncmp(line_tmp, ducky_cmd_altstr_2, strlen(ducky_cmd_altstr_2)) == 0)) {
        // ALTSTRING
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        ducky_numlock_on();
        state = ducky_altstring(line_tmp);
        if(!state && error != NULL) {
            snprintf(error, error_len, "Invalid altstring %s", line_tmp);
        }
        return (state) ? (0) : SCRIPT_STATE_ERROR;
    } else if(strncmp(line_tmp, ducky_cmd_repeat, strlen(ducky_cmd_repeat)) == 0) {
        // REPEAT
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        state = ducky_get_number(line_tmp, &bad_usb->repeat_cnt);
        if(!state && error != NULL) {
            snprintf(error, error_len, "Invalid number %s", line_tmp);
        }
        return (state) ? (0) : SCRIPT_STATE_ERROR;
    } else if(strncmp(line_tmp, ducky_cmd_sysrq, strlen(ducky_cmd_sysrq)) == 0) {
        // SYSRQ
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        uint16_t key = ducky_get_keycode(line_tmp, true);
        furi_hal_hid_kb_press(KEY_MOD_LEFT_ALT | HID_KEYBOARD_PRINT_SCREEN);
        furi_hal_hid_kb_press(key);
        furi_hal_hid_kb_release_all();
        return (0);
    } else {
        // Special keys + modifiers
        uint16_t key = ducky_get_keycode(line_tmp, false);
        if(key == HID_KEYBOARD_NONE) {
            if(error != NULL) {
                snprintf(error, error_len, "No keycode defined for %s", line_tmp);
            }
            return SCRIPT_STATE_ERROR;
        }
        if((key & 0xFF00) != 0) {
            // It's a modifier key
            line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
            key |= ducky_get_keycode(line_tmp, true);
        }
        furi_hal_hid_kb_press(key);
        furi_hal_hid_kb_release(key);
        return (0);
    }
}

static bool ducky_set_usb_id(BadUsbScript* bad_usb, const char* line) {
    if(sscanf(line, "%lX:%lX", &bad_usb->hid_cfg.vid, &bad_usb->hid_cfg.pid) == 2) {
        bad_usb->hid_cfg.manuf[0] = '\0';
        bad_usb->hid_cfg.product[0] = '\0';

        uint8_t id_len = ducky_get_command_len(line);
        if(!ducky_is_line_end(line[id_len + 1])) {
            sscanf(
                &line[id_len + 1],
                "%31[^\r\n:]:%31[^\r\n]",
                bad_usb->hid_cfg.manuf,
                bad_usb->hid_cfg.product);
        }
        FURI_LOG_D(
            WORKER_TAG,
            "set id: %04lX:%04lX mfr:%s product:%s",
            bad_usb->hid_cfg.vid,
            bad_usb->hid_cfg.pid,
            bad_usb->hid_cfg.manuf,
            bad_usb->hid_cfg.product);
        return true;
    }
    return false;
}

static bool ducky_script_preload(BadUsbScript* bad_usb, File* script_file) {
    uint8_t ret = 0;
    uint32_t line_len = 0;

    furi_string_reset(bad_usb->line);

    do {
        ret = storage_file_read(script_file, bad_usb->file_buf, FILE_BUFFER_LEN);
        for(uint16_t i = 0; i < ret; i++) {
            if(bad_usb->file_buf[i] == '\n' && line_len > 0) {
                bad_usb->st.line_nb++;
                line_len = 0;
            } else {
                if(bad_usb->st.line_nb == 0) { // Save first line
                    furi_string_push_back(bad_usb->line, bad_usb->file_buf[i]);
                }
                line_len++;
            }
        }
        if(storage_file_eof(script_file)) {
            if(line_len > 0) {
                bad_usb->st.line_nb++;
                break;
            }
        }
    } while(ret > 0);

    const char* line_tmp = furi_string_get_cstr(bad_usb->line);
    bool id_set = false; // Looking for ID command at first line
    if(strncmp(line_tmp, ducky_cmd_id, strlen(ducky_cmd_id)) == 0) {
        id_set = ducky_set_usb_id(bad_usb, &line_tmp[strlen(ducky_cmd_id) + 1]);
    }

    if(id_set) {
        furi_check(furi_hal_usb_set_config(&usb_hid, &bad_usb->hid_cfg));
    } else {
        furi_check(furi_hal_usb_set_config(&usb_hid, NULL));
    }

    storage_file_seek(script_file, 0, true);
    furi_string_reset(bad_usb->line);

    return true;
}

static int32_t ducky_script_execute_next(BadUsbScript* bad_usb, File* script_file) {
    int32_t delay_val = 0;

    if(bad_usb->repeat_cnt > 0) {
        bad_usb->repeat_cnt--;
        delay_val = ducky_parse_line(
            bad_usb, bad_usb->line_prev, bad_usb->st.error, sizeof(bad_usb->st.error));
        if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
            return 0;
        } else if(delay_val < 0) { // Script error
            bad_usb->st.error_line = bad_usb->st.line_cur - 1;
            FURI_LOG_E(WORKER_TAG, "Unknown command at line %u", bad_usb->st.line_cur - 1U);
            return SCRIPT_STATE_ERROR;
        } else {
            return (delay_val + bad_usb->defdelay);
        }
    }

    furi_string_set(bad_usb->line_prev, bad_usb->line);
    furi_string_reset(bad_usb->line);

    while(1) {
        if(bad_usb->buf_len == 0) {
            bad_usb->buf_len = storage_file_read(script_file, bad_usb->file_buf, FILE_BUFFER_LEN);
            if(storage_file_eof(script_file)) {
                if((bad_usb->buf_len < FILE_BUFFER_LEN) && (bad_usb->file_end == false)) {
                    bad_usb->file_buf[bad_usb->buf_len] = '\n';
                    bad_usb->buf_len++;
                    bad_usb->file_end = true;
                }
            }

            bad_usb->buf_start = 0;
            if(bad_usb->buf_len == 0) return SCRIPT_STATE_END;
        }
        for(uint8_t i = bad_usb->buf_start; i < (bad_usb->buf_start + bad_usb->buf_len); i++) {
            if(bad_usb->file_buf[i] == '\n' && furi_string_size(bad_usb->line) > 0) {
                bad_usb->st.line_cur++;
                bad_usb->buf_len = bad_usb->buf_len + bad_usb->buf_start - (i + 1);
                bad_usb->buf_start = i + 1;
                furi_string_trim(bad_usb->line);
                delay_val = ducky_parse_line(
                    bad_usb, bad_usb->line, bad_usb->st.error, sizeof(bad_usb->st.error));
                if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
                    return 0;
                } else if(delay_val < 0) {
                    bad_usb->st.error_line = bad_usb->st.line_cur;
                    FURI_LOG_E(WORKER_TAG, "Unknown command at line %u", bad_usb->st.line_cur);
                    return SCRIPT_STATE_ERROR;
                } else {
                    return (delay_val + bad_usb->defdelay);
                }
            } else {
                furi_string_push_back(bad_usb->line, bad_usb->file_buf[i]);
            }
        }
        bad_usb->buf_len = 0;
        if(bad_usb->file_end) return SCRIPT_STATE_END;
    }

    return 0;
}

static void bad_usb_hid_state_callback(bool state, void* context) {
    furi_assert(context);
    BadUsbScript* bad_usb = context;

    if(state == true)
        furi_thread_flags_set(furi_thread_get_id(bad_usb->thread), WorkerEvtConnect);
    else
        furi_thread_flags_set(furi_thread_get_id(bad_usb->thread), WorkerEvtDisconnect);
}

static int32_t bad_usb_worker(void* context) {
    BadUsbScript* bad_usb = context;

    BadUsbWorkerState worker_state = BadUsbStateInit;
    int32_t delay_val = 0;

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();

    FURI_LOG_I(WORKER_TAG, "Init");
    File* script_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    bad_usb->line = furi_string_alloc();
    bad_usb->line_prev = furi_string_alloc();

    furi_hal_hid_set_state_callback(bad_usb_hid_state_callback, bad_usb);

    while(1) {
        if(worker_state == BadUsbStateInit) { // State: initialization
            if(storage_file_open(
                   script_file,
                   furi_string_get_cstr(bad_usb->file_path),
                   FSAM_READ,
                   FSOM_OPEN_EXISTING)) {
                if((ducky_script_preload(bad_usb, script_file)) && (bad_usb->st.line_nb > 0)) {
                    if(furi_hal_hid_is_connected()) {
                        worker_state = BadUsbStateIdle; // Ready to run
                    } else {
                        worker_state = BadUsbStateNotConnected; // USB not connected
                    }
                } else {
                    worker_state = BadUsbStateScriptError; // Script preload error
                }
            } else {
                FURI_LOG_E(WORKER_TAG, "File open error");
                worker_state = BadUsbStateFileError; // File open error
            }
            bad_usb->st.state = worker_state;

        } else if(worker_state == BadUsbStateNotConnected) { // State: USB not connected
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtToggle,
                FuriFlagWaitAny,
                FuriWaitForever);
            furi_check((flags & FuriFlagError) == 0);
            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) {
                worker_state = BadUsbStateIdle; // Ready to run
            } else if(flags & WorkerEvtToggle) {
                worker_state = BadUsbStateWillRun; // Will run when USB is connected
            }
            bad_usb->st.state = worker_state;

        } else if(worker_state == BadUsbStateIdle) { // State: ready to start
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtToggle | WorkerEvtDisconnect,
                FuriFlagWaitAny,
                FuriWaitForever);
            furi_check((flags & FuriFlagError) == 0);
            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtToggle) { // Start executing script
                DOLPHIN_DEED(DolphinDeedBadUsbPlayScript);
                delay_val = 0;
                bad_usb->buf_len = 0;
                bad_usb->st.line_cur = 0;
                bad_usb->defdelay = 0;
                bad_usb->repeat_cnt = 0;
                bad_usb->file_end = false;
                storage_file_seek(script_file, 0, true);
                worker_state = BadUsbStateRunning;
            } else if(flags & WorkerEvtDisconnect) {
                worker_state = BadUsbStateNotConnected; // USB disconnected
            }
            bad_usb->st.state = worker_state;

        } else if(worker_state == BadUsbStateWillRun) { // State: start on connection
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtToggle,
                FuriFlagWaitAny,
                FuriWaitForever);
            furi_check((flags & FuriFlagError) == 0);
            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) { // Start executing script
                DOLPHIN_DEED(DolphinDeedBadUsbPlayScript);
                delay_val = 0;
                bad_usb->buf_len = 0;
                bad_usb->st.line_cur = 0;
                bad_usb->defdelay = 0;
                bad_usb->repeat_cnt = 0;
                bad_usb->file_end = false;
                storage_file_seek(script_file, 0, true);
                // extra time for PC to recognize Flipper as keyboard
                furi_thread_flags_wait(0, FuriFlagWaitAny, 1500);
                worker_state = BadUsbStateRunning;
            } else if(flags & WorkerEvtToggle) { // Cancel scheduled execution
                worker_state = BadUsbStateNotConnected;
            }
            bad_usb->st.state = worker_state;

        } else if(worker_state == BadUsbStateRunning) { // State: running
            uint16_t delay_cur = (delay_val > 1000) ? (1000) : (delay_val);
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtToggle | WorkerEvtDisconnect, FuriFlagWaitAny, delay_cur);
            delay_val -= delay_cur;
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtToggle) {
                    worker_state = BadUsbStateIdle; // Stop executing script
                    furi_hal_hid_kb_release_all();
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadUsbStateNotConnected; // USB disconnected
                    furi_hal_hid_kb_release_all();
                }
                bad_usb->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                if(delay_val > 0) {
                    bad_usb->st.delay_remain--;
                    continue;
                }
                bad_usb->st.state = BadUsbStateRunning;
                delay_val = ducky_script_execute_next(bad_usb, script_file);
                if(delay_val == SCRIPT_STATE_ERROR) { // Script error
                    delay_val = 0;
                    worker_state = BadUsbStateScriptError;
                    bad_usb->st.state = worker_state;
                } else if(delay_val == SCRIPT_STATE_END) { // End of script
                    delay_val = 0;
                    worker_state = BadUsbStateIdle;
                    bad_usb->st.state = BadUsbStateDone;
                    furi_hal_hid_kb_release_all();
                    continue;
                } else if(delay_val > 1000) {
                    bad_usb->st.state = BadUsbStateDelay; // Show long delays
                    bad_usb->st.delay_remain = delay_val / 1000;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }

        } else if(
            (worker_state == BadUsbStateFileError) ||
            (worker_state == BadUsbStateScriptError)) { // State: error
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd, FuriFlagWaitAny, FuriWaitForever); // Waiting for exit command
            furi_check((flags & FuriFlagError) == 0);
            if(flags & WorkerEvtEnd) {
                break;
            }
        }
    }

    furi_hal_hid_set_state_callback(NULL, NULL);

    furi_hal_usb_set_config(usb_mode_prev, NULL);

    storage_file_close(script_file);
    storage_file_free(script_file);
    furi_string_free(bad_usb->line);
    furi_string_free(bad_usb->line_prev);

    FURI_LOG_I(WORKER_TAG, "End");

    return 0;
}

BadUsbScript* bad_usb_script_open(FuriString* file_path) {
    furi_assert(file_path);

    BadUsbScript* bad_usb = malloc(sizeof(BadUsbScript));
    bad_usb->file_path = furi_string_alloc();
    furi_string_set(bad_usb->file_path, file_path);

    bad_usb->st.state = BadUsbStateInit;
    bad_usb->st.error[0] = '\0';

    bad_usb->thread = furi_thread_alloc_ex("BadUsbWorker", 2048, bad_usb_worker, bad_usb);
    furi_thread_start(bad_usb->thread);
    return bad_usb;
} //-V773

void bad_usb_script_close(BadUsbScript* bad_usb) {
    furi_assert(bad_usb);
    furi_thread_flags_set(furi_thread_get_id(bad_usb->thread), WorkerEvtEnd);
    furi_thread_join(bad_usb->thread);
    furi_thread_free(bad_usb->thread);
    furi_string_free(bad_usb->file_path);
    free(bad_usb);
}

void bad_usb_script_toggle(BadUsbScript* bad_usb) {
    furi_assert(bad_usb);
    furi_thread_flags_set(furi_thread_get_id(bad_usb->thread), WorkerEvtToggle);
}

BadUsbState* bad_usb_script_get_state(BadUsbScript* bad_usb) {
    furi_assert(bad_usb);
    return &(bad_usb->st);
}
