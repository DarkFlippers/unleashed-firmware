#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>

#define FILE_BUFFER_LEN 16

typedef enum {
    BadUsbStateInit,
    BadUsbStateNotConnected,
    BadUsbStateIdle,
    BadUsbStateWillRun,
    BadUsbStateRunning,
    BadUsbStateDelay,
    BadUsbStateDone,
    BadUsbStateScriptError,
    BadUsbStateFileError,
} BadUsbWorkerState;

typedef struct {
    BadUsbWorkerState state;
    uint16_t line_cur;
    uint16_t line_nb;
    uint32_t delay_remain;
    uint16_t error_line;
    char error[64];
} BadUsbState;

typedef struct BadUsbScript {
    FuriHalUsbHidConfig hid_cfg;
    BadUsbState st;
    FuriString* file_path;
    uint32_t defdelay;
    uint16_t layout[128];
    uint32_t stringdelay;
    FuriThread* thread;
    uint8_t file_buf[FILE_BUFFER_LEN + 1];
    uint8_t buf_start;
    uint8_t buf_len;
    bool file_end;
    FuriString* line;

    FuriString* line_prev;
    uint32_t repeat_cnt;
} BadUsbScript;

BadUsbScript* bad_usb_script_open(FuriString* file_path);

void bad_usb_script_close(BadUsbScript* bad_usb);

void bad_usb_script_set_keyboard_layout(BadUsbScript* bad_usb, FuriString* layout_path);

void bad_usb_script_start(BadUsbScript* bad_usb);

void bad_usb_script_stop(BadUsbScript* bad_usb);

void bad_usb_script_toggle(BadUsbScript* bad_usb);

BadUsbState* bad_usb_script_get_state(BadUsbScript* bad_usb);

uint16_t ducky_get_keycode(BadUsbScript* bad_usb, const char* param, bool accept_chars);

uint32_t ducky_get_command_len(const char* line);

bool ducky_is_line_end(const char chr);

#ifdef __cplusplus
}
#endif
