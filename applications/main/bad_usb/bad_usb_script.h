#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <m-string.h>

typedef struct BadUsbScript BadUsbScript;

typedef enum {
    BadUsbStateInit,
    BadUsbStateNotConnected,
    BadUsbStateIdle,
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
} BadUsbState;

BadUsbScript* bad_usb_script_open(string_t file_path);

void bad_usb_script_close(BadUsbScript* bad_usb);

void bad_usb_script_start(BadUsbScript* bad_usb);

void bad_usb_script_stop(BadUsbScript* bad_usb);

void bad_usb_script_toggle(BadUsbScript* bad_usb);

BadUsbState* bad_usb_script_get_state(BadUsbScript* bad_usb);

#ifdef __cplusplus
}
#endif
