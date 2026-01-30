#pragma once

#include <furi.h>
#include <storage/storage.h>
#include "duck3_hid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Duck3Script Duck3Script;

typedef enum {
    Duck3StateInit,
    Duck3StateNotConnected,
    Duck3StateIdle,
    Duck3StateWillRun,
    Duck3StateRunning,
    Duck3StateDelay,
    Duck3StatePaused,
    Duck3StateDone,
    Duck3StateError,
    Duck3StateFileError,
    Duck3StateScriptError,
    Duck3StateWaitForBtn,
} Duck3WorkerState;

typedef struct {
    Duck3WorkerState state;
    uint32_t line_cur;
    uint32_t line_nb;
    uint32_t delay_remain;
    uint32_t elapsed;
    uint32_t error_line;
    char error[64];
} Duck3State;

// Script lifecycle
Duck3Script* duck3_script_open(
    FuriString* file_path,
    Duck3HidInterface* interface,
    Duck3HidConfig* hid_cfg,
    bool load_id_config);

void duck3_script_close(Duck3Script* script);

// Script control
void duck3_script_start_stop(Duck3Script* script);
void duck3_script_pause_resume(Duck3Script* script);

// Keyboard layout
void duck3_script_set_keyboard_layout(Duck3Script* script, FuriString* layout_path);

// State
Duck3State* duck3_script_get_state(Duck3Script* script);

#ifdef __cplusplus
}
#endif
