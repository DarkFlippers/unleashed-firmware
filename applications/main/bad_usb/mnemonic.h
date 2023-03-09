#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "bad_usb_script.h"

// A no opperation function
int32_t ducky_fnc_noop(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// DELAY
int32_t ducky_fnc_delay(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// DEFAULTDELAY
int32_t ducky_fnc_defdelay(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// STRINGDELAY
int32_t ducky_fnc_strdelay(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// STRING
int32_t ducky_fnc_string(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// STRINGLN
int32_t ducky_fnc_stringln(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// REPEAT
int32_t ducky_fnc_repeat(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// SYSRQ
int32_t ducky_fnc_sysrq(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// ALTCHAR
int32_t ducky_fnc_altchar(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// ALTSTRING
int32_t ducky_fnc_altstring(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// HOLD
int32_t ducky_fnc_hold(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);
// RELEASE
int32_t ducky_fnc_release(
    BadUsbScript* bad_usb,
    FuriString* line,
    const char* line_tmp,
    char* error,
    size_t error_len);

#ifdef __cplusplus
}
#endif
