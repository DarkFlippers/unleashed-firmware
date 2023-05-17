#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/toolbox/args.h>
#include <furi_hal_bt_hid.h>
#include <bt/bt_service/bt.h>
#include <storage/storage.h>
#include "ducky_script.h"
#include "ducky_script_i.h"
#include <dolphin/dolphin.h>
#include <toolbox/hex.h>
#include "../bad_bt_app.h"

const uint8_t BAD_BT_BOUND_MAC_ADDRESS[BAD_BT_MAC_ADDRESS_LEN] =
    {0x41, 0x4a, 0xef, 0xb6, 0xa9, 0xd4};
const uint8_t BAD_BT_EMPTY_MAC_ADDRESS[BAD_BT_MAC_ADDRESS_LEN] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define TAG "BadBT"
#define WORKER_TAG TAG "Worker"

#define BADBT_ASCII_TO_KEY(script, x) \
    (((uint8_t)x < 128) ? (script->layout[(uint8_t)x]) : HID_KEYBOARD_NONE)

/**
 * Delays for waiting between HID key press and key release
*/
const uint8_t bt_hid_delays[LevelRssiNum] = {
    30, // LevelRssi122_100
    25, // LevelRssi99_80
    20, // LevelRssi79_60
    17, // LevelRssi59_40
    14, // LevelRssi39_0
};

uint8_t bt_timeout = 0;

static LevelRssiRange bt_remote_rssi_range(Bt* bt) {
    uint8_t rssi;

    if(!bt_remote_rssi(bt, &rssi)) return LevelRssiError;

    if(rssi <= 39)
        return LevelRssi39_0;
    else if(rssi <= 59)
        return LevelRssi59_40;
    else if(rssi <= 79)
        return LevelRssi79_60;
    else if(rssi <= 99)
        return LevelRssi99_80;
    else if(rssi <= 122)
        return LevelRssi122_100;

    return LevelRssiError;
}

static inline void update_bt_timeout(Bt* bt) {
    LevelRssiRange r = bt_remote_rssi_range(bt);
    if(r < LevelRssiNum) {
        bt_timeout = bt_hid_delays[r];
        FURI_LOG_D(WORKER_TAG, "BLE Key timeout : %u", bt_timeout);
    }
}

typedef enum {
    WorkerEvtToggle = (1 << 0),
    WorkerEvtEnd = (1 << 1),
    WorkerEvtConnect = (1 << 2),
    WorkerEvtDisconnect = (1 << 3),
} WorkerEvtFlags;

static const char ducky_cmd_id[] = {"ID"};
static const char ducky_cmd_bt_id[] = {"BT_ID"};

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
    return ((chr == ' ') || (chr == '\0') || (chr == '\r') || (chr == '\n'));
}

uint16_t ducky_get_keycode(BadBtScript* bad_bt, const char* param, bool accept_chars) {
    uint16_t keycode = ducky_get_keycode_by_name(param);
    if(keycode != HID_KEYBOARD_NONE) {
        return keycode;
    }

    if((accept_chars) && (strlen(param) > 0)) {
        return (BADBT_ASCII_TO_KEY(bad_bt, param[0]) & 0xFF);
    }
    return 0;
}

bool ducky_get_number(const char* param, uint32_t* val) {
    uint32_t value = 0;
    if(sscanf(param, "%lu", &value) == 1) {
        *val = value;
        return true;
    }
    return false;
}

void ducky_numlock_on(BadBtScript* bad_bt) {
    UNUSED(bad_bt);
    if((furi_hal_bt_hid_get_led_state() & HID_KB_LED_NUM) == 0) {
        furi_hal_bt_hid_kb_press(HID_KEYBOARD_LOCK_NUM_LOCK);
        furi_delay_ms(bt_timeout);
        furi_hal_bt_hid_kb_release(HID_KEYBOARD_LOCK_NUM_LOCK);
    }
}

bool ducky_numpad_press(BadBtScript* bad_bt, const char num) {
    UNUSED(bad_bt);
    if((num < '0') || (num > '9')) return false;

    uint16_t key = numpad_keys[num - '0'];
    furi_hal_bt_hid_kb_press(key);
    furi_delay_ms(bt_timeout);
    furi_hal_bt_hid_kb_release(key);

    return true;
}

bool ducky_altchar(BadBtScript* bad_bt, const char* charcode) {
    uint8_t i = 0;
    bool state = false;

    furi_hal_bt_hid_kb_press(KEY_MOD_LEFT_ALT);

    while(!ducky_is_line_end(charcode[i])) {
        state = ducky_numpad_press(bad_bt, charcode[i]);
        if(state == false) break;
        i++;
    }

    furi_hal_bt_hid_kb_release(KEY_MOD_LEFT_ALT);

    return state;
}

bool ducky_altstring(BadBtScript* bad_bt, const char* param) {
    uint32_t i = 0;
    bool state = false;

    while(param[i] != '\0') {
        if((param[i] < ' ') || (param[i] > '~')) {
            i++;
            continue; // Skip non-printable chars
        }

        char temp_str[4];
        snprintf(temp_str, 4, "%u", param[i]);

        state = ducky_altchar(bad_bt, temp_str);
        if(state == false) break;
        i++;
    }
    return state;
}

int32_t ducky_error(BadBtScript* bad_bt, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(bad_bt->st.error, sizeof(bad_bt->st.error), text, args);

    va_end(args);
    return SCRIPT_STATE_ERROR;
}

bool ducky_string(BadBtScript* bad_bt, const char* param) {
    uint32_t i = 0;

    while(param[i] != '\0') {
        if(param[i] != '\n') {
            uint16_t keycode = BADBT_ASCII_TO_KEY(bad_bt, param[i]);
            if(keycode != HID_KEYBOARD_NONE) {
                furi_hal_bt_hid_kb_press(keycode);
                furi_delay_ms(bt_timeout);
                furi_hal_bt_hid_kb_release(keycode);
            }
        } else {
            furi_hal_bt_hid_kb_press(HID_KEYBOARD_RETURN);
            furi_delay_ms(bt_timeout);
            furi_hal_bt_hid_kb_release(HID_KEYBOARD_RETURN);
        }
        i++;
    }
    bad_bt->stringdelay = 0;
    return true;
}

static bool ducky_string_next(BadBtScript* bad_bt) {
    if(bad_bt->string_print_pos >= furi_string_size(bad_bt->string_print)) {
        return true;
    }

    char print_char = furi_string_get_char(bad_bt->string_print, bad_bt->string_print_pos);

    if(print_char != '\n') {
        uint16_t keycode = BADBT_ASCII_TO_KEY(bad_bt, print_char);
        if(keycode != HID_KEYBOARD_NONE) {
            furi_hal_bt_hid_kb_press(keycode);
            furi_delay_ms(bt_timeout);
            furi_hal_bt_hid_kb_release(keycode);
        }
    } else {
        furi_hal_bt_hid_kb_press(HID_KEYBOARD_RETURN);
        furi_delay_ms(bt_timeout);
        furi_hal_bt_hid_kb_release(HID_KEYBOARD_RETURN);
    }

    bad_bt->string_print_pos++;

    return false;
}

static int32_t ducky_parse_line(BadBtScript* bad_bt, FuriString* line) {
    uint32_t line_len = furi_string_size(line);
    const char* line_tmp = furi_string_get_cstr(line);

    if(line_len == 0) {
        return SCRIPT_STATE_NEXT_LINE; // Skip empty lines
    }
    FURI_LOG_D(WORKER_TAG, "line:%s", line_tmp);

    // Ducky Lang Functions
    int32_t cmd_result = ducky_execute_cmd(bad_bt, line_tmp);
    if(cmd_result != SCRIPT_STATE_CMD_UNKNOWN) {
        return cmd_result;
    }

    // Special keys + modifiers
    uint16_t key = ducky_get_keycode(bad_bt, line_tmp, false);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(bad_bt, "No keycode defined for %s", line_tmp);
    }
    if((key & 0xFF00) != 0) {
        // It's a modifier key
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        key |= ducky_get_keycode(bad_bt, line_tmp, true);
    }
    furi_hal_bt_hid_kb_press(key);
    furi_delay_ms(bt_timeout);
    furi_hal_bt_hid_kb_release(key);

    return 0;
}

static bool ducky_set_bt_id(BadBtScript* bad_bt, const char* line) {
    size_t line_len = strlen(line);
    size_t mac_len = BAD_BT_MAC_ADDRESS_LEN * 3;
    if(line_len < mac_len + 1) return false; // MAC + at least 1 char for name

    uint8_t mac[BAD_BT_MAC_ADDRESS_LEN];
    for(size_t i = 0; i < BAD_BT_MAC_ADDRESS_LEN; i++) {
        char a = line[i * 3];
        char b = line[i * 3 + 1];
        if((a < 'A' && a > 'F') || (a < '0' && a > '9') || (b < 'A' && b > 'F') ||
           (b < '0' && b > '9') || !hex_char_to_uint8(a, b, &mac[i])) {
            return false;
        }
    }

    furi_hal_bt_set_profile_adv_name(FuriHalBtProfileHidKeyboard, line + mac_len);
    bt_set_profile_mac_address(bad_bt->bt, mac);
    return true;
}

static bool ducky_script_preload(BadBtScript* bad_bt, File* script_file) {
    uint8_t ret = 0;
    uint32_t line_len = 0;

    furi_string_reset(bad_bt->line);

    do {
        ret = storage_file_read(script_file, bad_bt->file_buf, FILE_BUFFER_LEN);
        for(uint16_t i = 0; i < ret; i++) {
            if(bad_bt->file_buf[i] == '\n' && line_len > 0) {
                bad_bt->st.line_nb++;
                line_len = 0;
            } else {
                if(bad_bt->st.line_nb == 0) { // Save first line
                    furi_string_push_back(bad_bt->line, bad_bt->file_buf[i]);
                }
                line_len++;
            }
        }
        if(storage_file_eof(script_file)) {
            if(line_len > 0) {
                bad_bt->st.line_nb++;
                break;
            }
        }
    } while(ret > 0);

    const char* line_tmp = furi_string_get_cstr(bad_bt->line);
    if(bad_bt->app->switch_mode_thread) {
        furi_thread_join(bad_bt->app->switch_mode_thread);
        furi_thread_free(bad_bt->app->switch_mode_thread);
        bad_bt->app->switch_mode_thread = NULL;
    }
    // Looking for ID or BT_ID command at first line
    bad_bt->set_usb_id = false;
    bad_bt->set_bt_id = false;
    bad_bt->has_usb_id = strncmp(line_tmp, ducky_cmd_id, strlen(ducky_cmd_id)) == 0;
    // TODO: We setting has_usb_id to its value but ignoring it for now and not using anywhere here, may be used in a future to detect script type
    bad_bt->has_bt_id = strncmp(line_tmp, ducky_cmd_bt_id, strlen(ducky_cmd_bt_id)) == 0;
    if(bad_bt->has_bt_id) {
        if(!bad_bt->app->bt_remember) {
            bad_bt->set_bt_id = ducky_set_bt_id(bad_bt, &line_tmp[strlen(ducky_cmd_bt_id) + 1]);
        }
    }

    bad_kb_config_refresh_menu(bad_bt->app);

    if(!bad_bt->set_bt_id) {
        const char* bt_name = bad_bt->app->config.bt_name;
        const uint8_t* bt_mac = bad_bt->app->bt_remember ? (uint8_t*)&BAD_BT_BOUND_MAC_ADDRESS :
                                                           bad_bt->app->config.bt_mac;
        bool reset_name = strncmp(
            bt_name,
            furi_hal_bt_get_profile_adv_name(FuriHalBtProfileHidKeyboard),
            BAD_BT_ADV_NAME_MAX_LEN);
        bool reset_mac = memcmp(
            bt_mac,
            furi_hal_bt_get_profile_mac_addr(FuriHalBtProfileHidKeyboard),
            BAD_BT_MAC_ADDRESS_LEN);
        if(reset_name && reset_mac) {
            furi_hal_bt_set_profile_adv_name(FuriHalBtProfileHidKeyboard, bt_name);
        } else if(reset_name) {
            bt_set_profile_adv_name(bad_bt->bt, bt_name);
        }
        if(reset_mac) {
            bt_set_profile_mac_address(bad_bt->bt, bt_mac);
        }
    }

    storage_file_seek(script_file, 0, true);
    furi_string_reset(bad_bt->line);

    return true;
}

static int32_t ducky_script_execute_next(BadBtScript* bad_bt, File* script_file) {
    int32_t delay_val = 0;

    if(bad_bt->repeat_cnt > 0) {
        bad_bt->repeat_cnt--;
        delay_val = ducky_parse_line(bad_bt, bad_bt->line_prev);
        if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
            return 0;
        } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
            return delay_val;
        } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
            return delay_val;
        } else if(delay_val < 0) { // Script error
            bad_bt->st.error_line = bad_bt->st.line_cur - 1;
            FURI_LOG_E(WORKER_TAG, "Unknown command at line %u", bad_bt->st.line_cur - 1U);
            return SCRIPT_STATE_ERROR;
        } else {
            return (delay_val + bad_bt->defdelay);
        }
    }

    furi_string_set(bad_bt->line_prev, bad_bt->line);
    furi_string_reset(bad_bt->line);

    while(1) {
        if(bad_bt->buf_len == 0) {
            bad_bt->buf_len = storage_file_read(script_file, bad_bt->file_buf, FILE_BUFFER_LEN);
            if(storage_file_eof(script_file)) {
                if((bad_bt->buf_len < FILE_BUFFER_LEN) && (bad_bt->file_end == false)) {
                    bad_bt->file_buf[bad_bt->buf_len] = '\n';
                    bad_bt->buf_len++;
                    bad_bt->file_end = true;
                }
            }

            bad_bt->buf_start = 0;
            if(bad_bt->buf_len == 0) return SCRIPT_STATE_END;
        }
        for(uint8_t i = bad_bt->buf_start; i < (bad_bt->buf_start + bad_bt->buf_len); i++) {
            if(bad_bt->file_buf[i] == '\n' && furi_string_size(bad_bt->line) > 0) {
                bad_bt->st.line_cur++;
                bad_bt->buf_len = bad_bt->buf_len + bad_bt->buf_start - (i + 1);
                bad_bt->buf_start = i + 1;
                furi_string_trim(bad_bt->line);
                delay_val = ducky_parse_line(bad_bt, bad_bt->line);
                if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
                    return 0;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
                    return delay_val;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
                    return delay_val;
                } else if(delay_val < 0) {
                    bad_bt->st.error_line = bad_bt->st.line_cur;
                    FURI_LOG_E(WORKER_TAG, "Unknown command at line %u", bad_bt->st.line_cur);
                    return SCRIPT_STATE_ERROR;
                } else {
                    return (delay_val + bad_bt->defdelay);
                }
            } else {
                furi_string_push_back(bad_bt->line, bad_bt->file_buf[i]);
            }
        }
        bad_bt->buf_len = 0;
        if(bad_bt->file_end) return SCRIPT_STATE_END;
    }

    return 0;
}

static void bad_bt_bt_hid_state_callback(BtStatus status, void* context) {
    furi_assert(context);
    BadBtScript* bad_bt = context;
    bool state = (status == BtStatusConnected);

    if(state == true) {
        LevelRssiRange r = bt_remote_rssi_range(bad_bt->bt);
        if(r != LevelRssiError) {
            bt_timeout = bt_hid_delays[r];
        }
        furi_thread_flags_set(furi_thread_get_id(bad_bt->thread), WorkerEvtConnect);
    } else {
        furi_thread_flags_set(furi_thread_get_id(bad_bt->thread), WorkerEvtDisconnect);
    }
}

static uint32_t bad_bt_flags_get(uint32_t flags_mask, uint32_t timeout) {
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

static int32_t bad_bt_worker(void* context) {
    BadBtScript* bad_bt = context;

    BadBtWorkerState worker_state = BadBtStateInit;
    int32_t delay_val = 0;

    FURI_LOG_I(WORKER_TAG, "Init");
    File* script_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    bad_bt->line = furi_string_alloc();
    bad_bt->line_prev = furi_string_alloc();
    bad_bt->string_print = furi_string_alloc();

    bt_set_status_changed_callback(bad_bt->bt, bad_bt_bt_hid_state_callback, bad_bt);

    while(1) {
        if(worker_state == BadBtStateInit) { // State: initialization
            if(storage_file_open(
                   script_file,
                   furi_string_get_cstr(bad_bt->file_path),
                   FSAM_READ,
                   FSOM_OPEN_EXISTING)) {
                if((ducky_script_preload(bad_bt, script_file)) && (bad_bt->st.line_nb > 0)) {
                    if(furi_hal_bt_is_connected()) {
                        worker_state = BadBtStateIdle; // Ready to run
                    } else {
                        worker_state = BadBtStateNotConnected; // Not connected
                    }

                } else {
                    worker_state = BadBtStateScriptError; // Script preload error
                }
            } else {
                FURI_LOG_E(WORKER_TAG, "File open error");
                worker_state = BadBtStateFileError; // File open error
            }
            bad_bt->st.state = worker_state;

        } else if(worker_state == BadBtStateNotConnected) { // State: Not connected
            uint32_t flags = bad_bt_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtToggle, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) {
                worker_state = BadBtStateIdle; // Ready to run
            } else if(flags & WorkerEvtToggle) {
                worker_state = BadBtStateWillRun; // Will run when connected
            }
            bad_bt->st.state = worker_state;

        } else if(worker_state == BadBtStateIdle) { // State: ready to start
            uint32_t flags = bad_bt_flags_get(
                WorkerEvtEnd | WorkerEvtToggle | WorkerEvtDisconnect, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtToggle) { // Start executing script
                delay_val = 0;
                bad_bt->buf_len = 0;
                bad_bt->st.line_cur = 0;
                bad_bt->defdelay = 0;
                bad_bt->stringdelay = 0;
                bad_bt->repeat_cnt = 0;
                bad_bt->key_hold_nb = 0;
                bad_bt->file_end = false;
                storage_file_seek(script_file, 0, true);
                bad_bt_script_set_keyboard_layout(bad_bt, bad_bt->keyboard_layout);
                worker_state = BadBtStateRunning;
            } else if(flags & WorkerEvtDisconnect) {
                worker_state = BadBtStateNotConnected; // Disconnected
            }
            bad_bt->st.state = worker_state;

        } else if(worker_state == BadBtStateWillRun) { // State: start on connection
            uint32_t flags = bad_bt_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtToggle, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) { // Start executing script
                delay_val = 0;
                bad_bt->buf_len = 0;
                bad_bt->st.line_cur = 0;
                bad_bt->defdelay = 0;
                bad_bt->stringdelay = 0;
                bad_bt->repeat_cnt = 0;
                bad_bt->file_end = false;
                storage_file_seek(script_file, 0, true);
                // extra time for PC to recognize Flipper as keyboard
                flags = furi_thread_flags_wait(
                    WorkerEvtEnd | WorkerEvtDisconnect | WorkerEvtToggle,
                    FuriFlagWaitAny | FuriFlagNoClear,
                    1500);
                if(flags == (unsigned)FuriFlagErrorTimeout) {
                    // If nothing happened - start script execution
                    worker_state = BadBtStateRunning;
                } else if(flags & WorkerEvtToggle) {
                    worker_state = BadBtStateIdle;
                    furi_thread_flags_clear(WorkerEvtToggle);
                }

                update_bt_timeout(bad_bt->bt);

                bad_bt_script_set_keyboard_layout(bad_bt, bad_bt->keyboard_layout);
            } else if(flags & WorkerEvtToggle) { // Cancel scheduled execution
                worker_state = BadBtStateNotConnected;
            }
            bad_bt->st.state = worker_state;

        } else if(worker_state == BadBtStateRunning) { // State: running
            uint16_t delay_cur = (delay_val > 1000) ? (1000) : (delay_val);
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtToggle | WorkerEvtDisconnect, FuriFlagWaitAny, delay_cur);

            delay_val -= delay_cur;
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtToggle) {
                    worker_state = BadBtStateIdle; // Stop executing script

                    furi_hal_bt_hid_kb_release_all();

                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadBtStateNotConnected; // Disconnected

                    furi_hal_bt_hid_kb_release_all();
                }
                bad_bt->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                if(delay_val > 0) {
                    bad_bt->st.delay_remain--;
                    continue;
                }
                bad_bt->st.state = BadBtStateRunning;
                delay_val = ducky_script_execute_next(bad_bt, script_file);
                if(delay_val == SCRIPT_STATE_ERROR) { // Script error
                    delay_val = 0;
                    worker_state = BadBtStateScriptError;
                    bad_bt->st.state = worker_state;

                    furi_hal_bt_hid_kb_release_all();

                } else if(delay_val == SCRIPT_STATE_END) { // End of script
                    delay_val = 0;
                    worker_state = BadBtStateIdle;
                    bad_bt->st.state = BadBtStateDone;

                    furi_hal_bt_hid_kb_release_all();

                    continue;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Start printing string with delays
                    delay_val = bad_bt->defdelay;
                    bad_bt->string_print_pos = 0;
                    worker_state = BadBtStateStringDelay;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // set state to wait for user input
                    worker_state = BadBtStateWaitForBtn;
                    bad_bt->st.state = BadBtStateWaitForBtn; // Show long delays
                } else if(delay_val > 1000) {
                    bad_bt->st.state = BadBtStateDelay; // Show long delays
                    bad_bt->st.delay_remain = delay_val / 1000;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(worker_state == BadBtStateWaitForBtn) { // State: Wait for button Press
            uint16_t delay_cur = (delay_val > 1000) ? (1000) : (delay_val);
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtToggle | WorkerEvtDisconnect, FuriFlagWaitAny, delay_cur);
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtToggle) {
                    delay_val = 0;
                    worker_state = BadBtStateRunning;
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadBtStateNotConnected; // Disconnected
                    furi_hal_hid_kb_release_all();
                }
                bad_bt->st.state = worker_state;
                continue;
            }
        } else if(worker_state == BadBtStateStringDelay) { // State: print string with delays
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtToggle | WorkerEvtDisconnect,
                FuriFlagWaitAny,
                bad_bt->stringdelay);

            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtToggle) {
                    worker_state = BadBtStateIdle; // Stop executing script

                    furi_hal_bt_hid_kb_release_all();

                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadBtStateNotConnected; // Disconnected

                    furi_hal_bt_hid_kb_release_all();
                }
                bad_bt->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                bool string_end = ducky_string_next(bad_bt);
                if(string_end) {
                    bad_bt->stringdelay = 0;
                    worker_state = BadBtStateRunning;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(
            (worker_state == BadBtStateFileError) ||
            (worker_state == BadBtStateScriptError)) { // State: error
            uint32_t flags =
                bad_bt_flags_get(WorkerEvtEnd, FuriWaitForever); // Waiting for exit command

            if(flags & WorkerEvtEnd) {
                break;
            }
        }

        update_bt_timeout(bad_bt->bt);
    }

    bt_set_status_changed_callback(bad_bt->bt, NULL, NULL);

    storage_file_close(script_file);
    storage_file_free(script_file);
    furi_string_free(bad_bt->line);
    furi_string_free(bad_bt->line_prev);
    furi_string_free(bad_bt->string_print);

    FURI_LOG_I(WORKER_TAG, "End");

    return 0;
}

static void bad_bt_script_set_default_keyboard_layout(BadBtScript* bad_bt) {
    furi_assert(bad_bt);
    furi_string_set_str(bad_bt->keyboard_layout, "");
    memset(bad_bt->layout, HID_KEYBOARD_NONE, sizeof(bad_bt->layout));
    memcpy(bad_bt->layout, hid_asciimap, MIN(sizeof(hid_asciimap), sizeof(bad_bt->layout)));
}

BadBtScript* bad_bt_script_open(FuriString* file_path, Bt* bt, BadBtApp* app) {
    furi_assert(file_path);

    BadBtScript* bad_bt = malloc(sizeof(BadBtScript));
    bad_bt->app = app;
    bad_bt->file_path = furi_string_alloc();
    furi_string_set(bad_bt->file_path, file_path);
    bad_bt->keyboard_layout = furi_string_alloc();
    bad_bt_script_set_default_keyboard_layout(bad_bt);

    bad_bt->st.state = BadBtStateInit;
    bad_bt->st.error[0] = '\0';

    bad_bt->bt = bt;

    bad_bt->thread = furi_thread_alloc_ex("BadBtWorker", 2048, bad_bt_worker, bad_bt);
    furi_thread_start(bad_bt->thread);
    return bad_bt;
}

void bad_bt_script_close(BadBtScript* bad_bt) {
    furi_assert(bad_bt);
    furi_record_close(RECORD_STORAGE);
    furi_thread_flags_set(furi_thread_get_id(bad_bt->thread), WorkerEvtEnd);
    furi_thread_join(bad_bt->thread);
    furi_thread_free(bad_bt->thread);
    furi_string_free(bad_bt->file_path);
    furi_string_free(bad_bt->keyboard_layout);
    free(bad_bt);
}

void bad_bt_script_set_keyboard_layout(BadBtScript* bad_bt, FuriString* layout_path) {
    furi_assert(bad_bt);

    if((bad_bt->st.state == BadBtStateRunning) || (bad_bt->st.state == BadBtStateDelay)) {
        // do not update keyboard layout while a script is running
        return;
    }

    File* layout_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    if(!furi_string_empty(layout_path)) { //-V1051
        furi_string_set(bad_bt->keyboard_layout, layout_path);
        if(storage_file_open(
               layout_file, furi_string_get_cstr(layout_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            uint16_t layout[128];
            if(storage_file_read(layout_file, layout, sizeof(layout)) == sizeof(layout)) {
                memcpy(bad_bt->layout, layout, sizeof(layout));
            }
        }
        storage_file_close(layout_file);
    } else {
        bad_bt_script_set_default_keyboard_layout(bad_bt);
    }
    storage_file_free(layout_file);
}

void bad_bt_script_toggle(BadBtScript* bad_bt) {
    furi_assert(bad_bt);
    furi_thread_flags_set(furi_thread_get_id(bad_bt->thread), WorkerEvtToggle);
}

BadBtState* bad_bt_script_get_state(BadBtScript* bad_bt) {
    furi_assert(bad_bt);
    return &(bad_bt->st);
}