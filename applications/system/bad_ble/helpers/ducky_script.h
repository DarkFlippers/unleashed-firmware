#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>
#include "bad_ble_hid.h"

typedef enum {
    BadBleStateInit,
    BadBleStateNotConnected,
    BadBleStateIdle,
    BadBleStateWillRun,
    BadBleStateRunning,
    BadBleStateDelay,
    BadBleStateStringDelay,
    BadBleStateWaitForBtn,
    BadBleStatePaused,
    BadBleStateDone,
    BadBleStateScriptError,
    BadBleStateFileError,
} BadBleWorkerState;

typedef struct {
    BadBleWorkerState state;
    size_t line_cur;
    size_t line_nb;
    uint32_t delay_remain;
    size_t error_line;
    char error[64];
} BadBleState;

typedef struct BadBleScript BadBleScript;

BadBleScript* bad_ble_script_open(FuriString* file_path, BadBleHidInterface interface);

void bad_ble_script_close(BadBleScript* bad_ble);

void bad_ble_script_set_keyboard_layout(BadBleScript* bad_ble, FuriString* layout_path);

void bad_ble_script_start(BadBleScript* bad_ble);

void bad_ble_script_stop(BadBleScript* bad_ble);

void bad_ble_script_start_stop(BadBleScript* bad_ble);

void bad_ble_script_pause_resume(BadBleScript* bad_ble);

BadBleState* bad_ble_script_get_state(BadBleScript* bad_ble);

#ifdef __cplusplus
}
#endif
