#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>
#include "ducky_script.h"

#define SCRIPT_STATE_ERROR (-1)
#define SCRIPT_STATE_END (-2)
#define SCRIPT_STATE_NEXT_LINE (-3)
#define SCRIPT_STATE_CMD_UNKNOWN (-4)
#define SCRIPT_STATE_STRING_START (-5)
#define SCRIPT_STATE_WAIT_FOR_BTN (-6)

uint16_t ducky_get_keycode(BadBtScript* bad_bt, const char* param, bool accept_chars);

uint32_t ducky_get_command_len(const char* line);

bool ducky_is_line_end(const char chr);

uint16_t ducky_get_keycode_by_name(const char* param);

bool ducky_get_number(const char* param, uint32_t* val);

void ducky_numlock_on(BadBtScript* bad_bt);

bool ducky_numpad_press(BadBtScript* bad_bt, const char num);

bool ducky_altchar(BadBtScript* bad_bt, const char* charcode);

bool ducky_altstring(BadBtScript* bad_bt, const char* param);

bool ducky_string(BadBtScript* bad_bt, const char* param);

int32_t ducky_execute_cmd(BadBtScript* bad_bt, const char* line);

int32_t ducky_error(BadBtScript* bad_bt, const char* text, ...);

#ifdef __cplusplus
}
#endif
