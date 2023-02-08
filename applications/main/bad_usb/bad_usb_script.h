#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

typedef struct BadUsbScript BadUsbScript;

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

BadUsbScript* bad_usb_script_open(FuriString* file_path);

void bad_usb_script_close(BadUsbScript* bad_usb);

void bad_usb_script_set_keyboard_layout(BadUsbScript* bad_usb, FuriString* layout_path);

void bad_usb_script_start(BadUsbScript* bad_usb);

void bad_usb_script_stop(BadUsbScript* bad_usb);

void bad_usb_script_toggle(BadUsbScript* bad_usb);

BadUsbState* bad_usb_script_get_state(BadUsbScript* bad_usb);

#ifdef __cplusplus
}
#endif
