#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/toolbox/args.h>
#include <lib/toolbox/strint.h>
#include <storage/storage.h>
#include "ducky_script.h"
#include "ducky_script_i.h"
#include <dolphin/dolphin.h>

#define TAG "BadBle"

#define WORKER_TAG TAG "Worker"

#define BADUSB_ASCII_TO_KEY(script, x) \
    (((uint8_t)x < 128) ? (script->layout[(uint8_t)x]) : HID_KEYBOARD_NONE)

typedef enum {
    WorkerEvtStartStop = (1 << 0),
    WorkerEvtPauseResume = (1 << 1),
    WorkerEvtEnd = (1 << 2),
    WorkerEvtConnect = (1 << 3),
    WorkerEvtDisconnect = (1 << 4),
} WorkerEvtFlags;

static const char ducky_cmd_id[] = {"ID"};

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

uint32_t ducky_get_command_len(const char* line) {
    uint32_t len = strlen(line);
    for(uint32_t i = 0; i < len; i++) {
        if(line[i] == ' ') return i;
    }
    return 0;
}

bool ducky_is_line_end(const char chr) {
    return (chr == ' ') || (chr == '\0') || (chr == '\r') || (chr == '\n');
}

uint16_t ducky_get_keycode(BadBleScript* bad_ble, const char* param, bool accept_chars) {
    uint16_t keycode = ducky_get_keycode_by_name(param);
    if(keycode != HID_KEYBOARD_NONE) {
        return keycode;
    }

    if((accept_chars) && (strlen(param) > 0)) {
        return BADUSB_ASCII_TO_KEY(bad_ble, param[0]) & 0xFF;
    }
    return 0;
}

bool ducky_get_number(const char* param, uint32_t* val) {
    uint32_t value = 0;
    if(strint_to_uint32(param, NULL, &value, 10) == StrintParseNoError) {
        *val = value;
        return true;
    }
    return false;
}

void ducky_numlock_on(BadBleScript* bad_ble) {
    if((bad_ble->hid->get_led_state(bad_ble->hid_inst) & HID_KB_LED_NUM) == 0) {
        bad_ble->hid->kb_press(bad_ble->hid_inst, HID_KEYBOARD_LOCK_NUM_LOCK);
        bad_ble->hid->kb_release(bad_ble->hid_inst, HID_KEYBOARD_LOCK_NUM_LOCK);
    }
}

bool ducky_numpad_press(BadBleScript* bad_ble, const char num) {
    if((num < '0') || (num > '9')) return false;

    uint16_t key = numpad_keys[num - '0'];
    bad_ble->hid->kb_press(bad_ble->hid_inst, key);
    bad_ble->hid->kb_release(bad_ble->hid_inst, key);

    return true;
}

bool ducky_altchar(BadBleScript* bad_ble, const char* charcode) {
    uint8_t i = 0;
    bool state = false;

    bad_ble->hid->kb_press(bad_ble->hid_inst, KEY_MOD_LEFT_ALT);

    while(!ducky_is_line_end(charcode[i])) {
        state = ducky_numpad_press(bad_ble, charcode[i]);
        if(state == false) break;
        i++;
    }

    bad_ble->hid->kb_release(bad_ble->hid_inst, KEY_MOD_LEFT_ALT);
    return state;
}

bool ducky_altstring(BadBleScript* bad_ble, const char* param) {
    uint32_t i = 0;
    bool state = false;

    while(param[i] != '\0') {
        if((param[i] < ' ') || (param[i] > '~')) {
            i++;
            continue; // Skip non-printable chars
        }

        char temp_str[4];
        snprintf(temp_str, 4, "%u", param[i]);

        state = ducky_altchar(bad_ble, temp_str);
        if(state == false) break;
        i++;
    }
    return state;
}

int32_t ducky_error(BadBleScript* bad_ble, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(bad_ble->st.error, sizeof(bad_ble->st.error), text, args);

    va_end(args);
    return SCRIPT_STATE_ERROR;
}

bool ducky_string(BadBleScript* bad_ble, const char* param) {
    uint32_t i = 0;

    while(param[i] != '\0') {
        if(param[i] != '\n') {
            uint16_t keycode = BADUSB_ASCII_TO_KEY(bad_ble, param[i]);
            if(keycode != HID_KEYBOARD_NONE) {
                bad_ble->hid->kb_press(bad_ble->hid_inst, keycode);
                bad_ble->hid->kb_release(bad_ble->hid_inst, keycode);
            }
        } else {
            bad_ble->hid->kb_press(bad_ble->hid_inst, HID_KEYBOARD_RETURN);
            bad_ble->hid->kb_release(bad_ble->hid_inst, HID_KEYBOARD_RETURN);
        }
        i++;
    }
    bad_ble->stringdelay = 0;
    return true;
}

static bool ducky_string_next(BadBleScript* bad_ble) {
    if(bad_ble->string_print_pos >= furi_string_size(bad_ble->string_print)) {
        return true;
    }

    char print_char = furi_string_get_char(bad_ble->string_print, bad_ble->string_print_pos);

    if(print_char != '\n') {
        uint16_t keycode = BADUSB_ASCII_TO_KEY(bad_ble, print_char);
        if(keycode != HID_KEYBOARD_NONE) {
            bad_ble->hid->kb_press(bad_ble->hid_inst, keycode);
            bad_ble->hid->kb_release(bad_ble->hid_inst, keycode);
        }
    } else {
        bad_ble->hid->kb_press(bad_ble->hid_inst, HID_KEYBOARD_RETURN);
        bad_ble->hid->kb_release(bad_ble->hid_inst, HID_KEYBOARD_RETURN);
    }

    bad_ble->string_print_pos++;

    return false;
}

static int32_t ducky_parse_line(BadBleScript* bad_ble, FuriString* line) {
    uint32_t line_len = furi_string_size(line);
    const char* line_tmp = furi_string_get_cstr(line);

    if(line_len == 0) {
        return SCRIPT_STATE_NEXT_LINE; // Skip empty lines
    }
    FURI_LOG_D(WORKER_TAG, "line:%s", line_tmp);

    // Ducky Lang Functions
    int32_t cmd_result = ducky_execute_cmd(bad_ble, line_tmp);
    if(cmd_result != SCRIPT_STATE_CMD_UNKNOWN) {
        return cmd_result;
    }

    // Special keys + modifiers
    uint16_t key = ducky_get_keycode(bad_ble, line_tmp, false);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(bad_ble, "No keycode defined for %s", line_tmp);
    }
    if((key & 0xFF00) != 0) {
        // It's a modifier key
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        key |= ducky_get_keycode(bad_ble, line_tmp, true);
    }
    bad_ble->hid->kb_press(bad_ble->hid_inst, key);
    bad_ble->hid->kb_release(bad_ble->hid_inst, key);
    return 0;
}

static bool ducky_set_usb_id(BadBleScript* bad_ble, const char* line) {
    if(sscanf(line, "%lX:%lX", &bad_ble->hid_cfg.vid, &bad_ble->hid_cfg.pid) == 2) {
        bad_ble->hid_cfg.manuf[0] = '\0';
        bad_ble->hid_cfg.product[0] = '\0';

        uint8_t id_len = ducky_get_command_len(line);
        if(!ducky_is_line_end(line[id_len + 1])) {
            sscanf(
                &line[id_len + 1],
                "%31[^\r\n:]:%31[^\r\n]",
                bad_ble->hid_cfg.manuf,
                bad_ble->hid_cfg.product);
        }
        FURI_LOG_D(
            WORKER_TAG,
            "set id: %04lX:%04lX mfr:%s product:%s",
            bad_ble->hid_cfg.vid,
            bad_ble->hid_cfg.pid,
            bad_ble->hid_cfg.manuf,
            bad_ble->hid_cfg.product);
        return true;
    }
    return false;
}

static void bad_ble_hid_state_callback(bool state, void* context) {
    furi_assert(context);
    BadBleScript* bad_ble = context;

    if(state == true) {
        furi_thread_flags_set(furi_thread_get_id(bad_ble->thread), WorkerEvtConnect);
    } else {
        furi_thread_flags_set(furi_thread_get_id(bad_ble->thread), WorkerEvtDisconnect);
    }
}

static bool ducky_script_preload(BadBleScript* bad_ble, File* script_file) {
    uint8_t ret = 0;
    uint32_t line_len = 0;

    furi_string_reset(bad_ble->line);

    do {
        ret = storage_file_read(script_file, bad_ble->file_buf, FILE_BUFFER_LEN);
        for(uint16_t i = 0; i < ret; i++) {
            if(bad_ble->file_buf[i] == '\n' && line_len > 0) {
                bad_ble->st.line_nb++;
                line_len = 0;
            } else {
                if(bad_ble->st.line_nb == 0) { // Save first line
                    furi_string_push_back(bad_ble->line, bad_ble->file_buf[i]);
                }
                line_len++;
            }
        }
        if(storage_file_eof(script_file)) {
            if(line_len > 0) {
                bad_ble->st.line_nb++;
                break;
            }
        }
    } while(ret > 0);

    const char* line_tmp = furi_string_get_cstr(bad_ble->line);
    bool id_set = false; // Looking for ID command at first line
    if(strncmp(line_tmp, ducky_cmd_id, strlen(ducky_cmd_id)) == 0) {
        id_set = ducky_set_usb_id(bad_ble, &line_tmp[strlen(ducky_cmd_id) + 1]);
    }

    if(id_set) {
        bad_ble->hid_inst = bad_ble->hid->init(&bad_ble->hid_cfg);
    } else {
        bad_ble->hid_inst = bad_ble->hid->init(NULL);
    }
    bad_ble->hid->set_state_callback(bad_ble->hid_inst, bad_ble_hid_state_callback, bad_ble);

    storage_file_seek(script_file, 0, true);
    furi_string_reset(bad_ble->line);

    return true;
}

static int32_t ducky_script_execute_next(BadBleScript* bad_ble, File* script_file) {
    int32_t delay_val = 0;

    if(bad_ble->repeat_cnt > 0) {
        bad_ble->repeat_cnt--;
        delay_val = ducky_parse_line(bad_ble, bad_ble->line_prev);
        if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
            return 0;
        } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
            return delay_val;
        } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
            return delay_val;
        } else if(delay_val < 0) { // Script error
            bad_ble->st.error_line = bad_ble->st.line_cur - 1;
            FURI_LOG_E(WORKER_TAG, "Unknown command at line %zu", bad_ble->st.line_cur - 1U);
            return SCRIPT_STATE_ERROR;
        } else {
            return delay_val + bad_ble->defdelay;
        }
    }

    furi_string_set(bad_ble->line_prev, bad_ble->line);
    furi_string_reset(bad_ble->line);

    while(1) {
        if(bad_ble->buf_len == 0) {
            bad_ble->buf_len = storage_file_read(script_file, bad_ble->file_buf, FILE_BUFFER_LEN);
            if(storage_file_eof(script_file)) {
                if((bad_ble->buf_len < FILE_BUFFER_LEN) && (bad_ble->file_end == false)) {
                    bad_ble->file_buf[bad_ble->buf_len] = '\n';
                    bad_ble->buf_len++;
                    bad_ble->file_end = true;
                }
            }

            bad_ble->buf_start = 0;
            if(bad_ble->buf_len == 0) return SCRIPT_STATE_END;
        }
        for(uint8_t i = bad_ble->buf_start; i < (bad_ble->buf_start + bad_ble->buf_len); i++) {
            if(bad_ble->file_buf[i] == '\n' && furi_string_size(bad_ble->line) > 0) {
                bad_ble->st.line_cur++;
                bad_ble->buf_len = bad_ble->buf_len + bad_ble->buf_start - (i + 1);
                bad_ble->buf_start = i + 1;
                furi_string_trim(bad_ble->line);
                delay_val = ducky_parse_line(bad_ble, bad_ble->line);
                if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
                    return 0;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
                    return delay_val;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
                    return delay_val;
                } else if(delay_val < 0) {
                    bad_ble->st.error_line = bad_ble->st.line_cur;
                    FURI_LOG_E(WORKER_TAG, "Unknown command at line %zu", bad_ble->st.line_cur);
                    return SCRIPT_STATE_ERROR;
                } else {
                    return delay_val + bad_ble->defdelay;
                }
            } else {
                furi_string_push_back(bad_ble->line, bad_ble->file_buf[i]);
            }
        }
        bad_ble->buf_len = 0;
        if(bad_ble->file_end) return SCRIPT_STATE_END;
    }

    return 0;
}

static uint32_t bad_ble_flags_get(uint32_t flags_mask, uint32_t timeout) {
    uint32_t flags = furi_thread_flags_get();
    furi_check((flags & FuriFlagError) == 0);
    if(flags == 0) {
        flags = furi_thread_flags_wait(flags_mask, FuriFlagWaitAny, timeout);
        furi_check(((flags & FuriFlagError) == 0) || (flags == (unsigned)FuriFlagErrorTimeout));
    } else {
        uint32_t state = furi_thread_flags_clear(flags);
        furi_check((state & FuriFlagError) == 0);
    }
    return flags;
}

static int32_t bad_ble_worker(void* context) {
    BadBleScript* bad_ble = context;

    BadBleWorkerState worker_state = BadBleStateInit;
    BadBleWorkerState pause_state = BadBleStateRunning;
    int32_t delay_val = 0;

    FURI_LOG_I(WORKER_TAG, "Init");
    File* script_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    bad_ble->line = furi_string_alloc();
    bad_ble->line_prev = furi_string_alloc();
    bad_ble->string_print = furi_string_alloc();

    while(1) {
        if(worker_state == BadBleStateInit) { // State: initialization
            if(storage_file_open(
                   script_file,
                   furi_string_get_cstr(bad_ble->file_path),
                   FSAM_READ,
                   FSOM_OPEN_EXISTING)) {
                if((ducky_script_preload(bad_ble, script_file)) && (bad_ble->st.line_nb > 0)) {
                    if(bad_ble->hid->is_connected(bad_ble->hid_inst)) {
                        worker_state = BadBleStateIdle; // Ready to run
                    } else {
                        worker_state = BadBleStateNotConnected; // USB not connected
                    }
                } else {
                    worker_state = BadBleStateScriptError; // Script preload error
                }
            } else {
                FURI_LOG_E(WORKER_TAG, "File open error");
                worker_state = BadBleStateFileError; // File open error
            }
            bad_ble->st.state = worker_state;

        } else if(worker_state == BadBleStateNotConnected) { // State: USB not connected
            uint32_t flags = bad_ble_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtDisconnect | WorkerEvtStartStop,
                FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) {
                worker_state = BadBleStateIdle; // Ready to run
            } else if(flags & WorkerEvtStartStop) {
                worker_state = BadBleStateWillRun; // Will run when USB is connected
            }
            bad_ble->st.state = worker_state;

        } else if(worker_state == BadBleStateIdle) { // State: ready to start
            uint32_t flags = bad_ble_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtDisconnect, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtStartStop) { // Start executing script
                dolphin_deed(DolphinDeedBadUsbPlayScript);
                delay_val = 0;
                bad_ble->buf_len = 0;
                bad_ble->st.line_cur = 0;
                bad_ble->defdelay = 0;
                bad_ble->stringdelay = 0;
                bad_ble->defstringdelay = 0;
                bad_ble->repeat_cnt = 0;
                bad_ble->key_hold_nb = 0;
                bad_ble->file_end = false;
                storage_file_seek(script_file, 0, true);
                worker_state = BadBleStateRunning;
            } else if(flags & WorkerEvtDisconnect) {
                worker_state = BadBleStateNotConnected; // USB disconnected
            }
            bad_ble->st.state = worker_state;

        } else if(worker_state == BadBleStateWillRun) { // State: start on connection
            uint32_t flags = bad_ble_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtStartStop, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) { // Start executing script
                dolphin_deed(DolphinDeedBadUsbPlayScript);
                delay_val = 0;
                bad_ble->buf_len = 0;
                bad_ble->st.line_cur = 0;
                bad_ble->defdelay = 0;
                bad_ble->stringdelay = 0;
                bad_ble->defstringdelay = 0;
                bad_ble->repeat_cnt = 0;
                bad_ble->file_end = false;
                storage_file_seek(script_file, 0, true);
                // extra time for PC to recognize Flipper as keyboard
                flags = furi_thread_flags_wait(
                    WorkerEvtEnd | WorkerEvtDisconnect | WorkerEvtStartStop,
                    FuriFlagWaitAny | FuriFlagNoClear,
                    1500);
                if(flags == (unsigned)FuriFlagErrorTimeout) {
                    // If nothing happened - start script execution
                    worker_state = BadBleStateRunning;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadBleStateIdle;
                    furi_thread_flags_clear(WorkerEvtStartStop);
                }
            } else if(flags & WorkerEvtStartStop) { // Cancel scheduled execution
                worker_state = BadBleStateNotConnected;
            }
            bad_ble->st.state = worker_state;

        } else if(worker_state == BadBleStateRunning) { // State: running
            uint16_t delay_cur = (delay_val > 1000) ? (1000) : (delay_val);
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriFlagWaitAny,
                delay_cur);

            delay_val -= delay_cur;
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadBleStateIdle; // Stop executing script
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadBleStateNotConnected; // USB disconnected
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    pause_state = BadBleStateRunning;
                    worker_state = BadBleStatePaused; // Pause
                }
                bad_ble->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                if(delay_val > 0) {
                    bad_ble->st.delay_remain--;
                    continue;
                }
                bad_ble->st.state = BadBleStateRunning;
                delay_val = ducky_script_execute_next(bad_ble, script_file);
                if(delay_val == SCRIPT_STATE_ERROR) { // Script error
                    delay_val = 0;
                    worker_state = BadBleStateScriptError;
                    bad_ble->st.state = worker_state;
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                } else if(delay_val == SCRIPT_STATE_END) { // End of script
                    delay_val = 0;
                    worker_state = BadBleStateIdle;
                    bad_ble->st.state = BadBleStateDone;
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                    continue;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Start printing string with delays
                    delay_val = bad_ble->defdelay;
                    bad_ble->string_print_pos = 0;
                    worker_state = BadBleStateStringDelay;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // set state to wait for user input
                    worker_state = BadBleStateWaitForBtn;
                    bad_ble->st.state = BadBleStateWaitForBtn; // Show long delays
                } else if(delay_val > 1000) {
                    bad_ble->st.state = BadBleStateDelay; // Show long delays
                    bad_ble->st.delay_remain = delay_val / 1000;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(worker_state == BadBleStateWaitForBtn) { // State: Wait for button Press
            uint32_t flags = bad_ble_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriWaitForever);
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    delay_val = 0;
                    worker_state = BadBleStateRunning;
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadBleStateNotConnected; // USB disconnected
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                }
                bad_ble->st.state = worker_state;
                continue;
            }
        } else if(worker_state == BadBleStatePaused) { // State: Paused
            uint32_t flags = bad_ble_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriWaitForever);
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadBleStateIdle; // Stop executing script
                    bad_ble->st.state = worker_state;
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadBleStateNotConnected; // USB disconnected
                    bad_ble->st.state = worker_state;
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    if(pause_state == BadBleStateRunning) {
                        if(delay_val > 0) {
                            bad_ble->st.state = BadBleStateDelay;
                            bad_ble->st.delay_remain = delay_val / 1000;
                        } else {
                            bad_ble->st.state = BadBleStateRunning;
                            delay_val = 0;
                        }
                        worker_state = BadBleStateRunning; // Resume
                    } else if(pause_state == BadBleStateStringDelay) {
                        bad_ble->st.state = BadBleStateRunning;
                        worker_state = BadBleStateStringDelay; // Resume
                    }
                }
                continue;
            }
        } else if(worker_state == BadBleStateStringDelay) { // State: print string with delays
            uint32_t delay = (bad_ble->stringdelay == 0) ? bad_ble->defstringdelay :
                                                           bad_ble->stringdelay;
            uint32_t flags = bad_ble_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                delay);

            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadBleStateIdle; // Stop executing script
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadBleStateNotConnected; // USB disconnected
                    bad_ble->hid->release_all(bad_ble->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    pause_state = BadBleStateStringDelay;
                    worker_state = BadBleStatePaused; // Pause
                }
                bad_ble->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                bool string_end = ducky_string_next(bad_ble);
                if(string_end) {
                    bad_ble->stringdelay = 0;
                    worker_state = BadBleStateRunning;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(
            (worker_state == BadBleStateFileError) ||
            (worker_state == BadBleStateScriptError)) { // State: error
            uint32_t flags =
                bad_ble_flags_get(WorkerEvtEnd, FuriWaitForever); // Waiting for exit command

            if(flags & WorkerEvtEnd) {
                break;
            }
        }
    }

    bad_ble->hid->set_state_callback(bad_ble->hid_inst, NULL, NULL);
    bad_ble->hid->deinit(bad_ble->hid_inst);

    storage_file_close(script_file);
    storage_file_free(script_file);
    furi_string_free(bad_ble->line);
    furi_string_free(bad_ble->line_prev);
    furi_string_free(bad_ble->string_print);

    FURI_LOG_I(WORKER_TAG, "End");

    return 0;
}

static void bad_ble_script_set_default_keyboard_layout(BadBleScript* bad_ble) {
    furi_assert(bad_ble);
    memset(bad_ble->layout, HID_KEYBOARD_NONE, sizeof(bad_ble->layout));
    memcpy(bad_ble->layout, hid_asciimap, MIN(sizeof(hid_asciimap), sizeof(bad_ble->layout)));
}

BadBleScript* bad_ble_script_open(FuriString* file_path, BadBleHidInterface interface) {
    furi_assert(file_path);

    BadBleScript* bad_ble = malloc(sizeof(BadBleScript));
    bad_ble->file_path = furi_string_alloc();
    furi_string_set(bad_ble->file_path, file_path);
    bad_ble_script_set_default_keyboard_layout(bad_ble);

    bad_ble->st.state = BadBleStateInit;
    bad_ble->st.error[0] = '\0';
    bad_ble->hid = bad_ble_hid_get_interface(interface);

    bad_ble->thread = furi_thread_alloc_ex("BadBleWorker", 2048, bad_ble_worker, bad_ble);
    furi_thread_start(bad_ble->thread);
    return bad_ble;
} //-V773

void bad_ble_script_close(BadBleScript* bad_ble) {
    furi_assert(bad_ble);
    furi_thread_flags_set(furi_thread_get_id(bad_ble->thread), WorkerEvtEnd);
    furi_thread_join(bad_ble->thread);
    furi_thread_free(bad_ble->thread);
    furi_string_free(bad_ble->file_path);
    free(bad_ble);
}

void bad_ble_script_set_keyboard_layout(BadBleScript* bad_ble, FuriString* layout_path) {
    furi_assert(bad_ble);

    if((bad_ble->st.state == BadBleStateRunning) || (bad_ble->st.state == BadBleStateDelay)) {
        // do not update keyboard layout while a script is running
        return;
    }

    File* layout_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    if(!furi_string_empty(layout_path)) { //-V1051
        if(storage_file_open(
               layout_file, furi_string_get_cstr(layout_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            uint16_t layout[128];
            if(storage_file_read(layout_file, layout, sizeof(layout)) == sizeof(layout)) {
                memcpy(bad_ble->layout, layout, sizeof(layout));
            }
        }
        storage_file_close(layout_file);
    } else {
        bad_ble_script_set_default_keyboard_layout(bad_ble);
    }
    storage_file_free(layout_file);
}

void bad_ble_script_start_stop(BadBleScript* bad_ble) {
    furi_assert(bad_ble);
    furi_thread_flags_set(furi_thread_get_id(bad_ble->thread), WorkerEvtStartStop);
}

void bad_ble_script_pause_resume(BadBleScript* bad_ble) {
    furi_assert(bad_ble);
    furi_thread_flags_set(furi_thread_get_id(bad_ble->thread), WorkerEvtPauseResume);
}

BadBleState* bad_ble_script_get_state(BadBleScript* bad_ble) {
    furi_assert(bad_ble);
    return &(bad_ble->st);
}
