#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DESKTOP_PIN_CODE_MIN_LEN (4)
#define DESKTOP_PIN_CODE_MAX_LEN (10)

typedef struct {
    uint8_t data[DESKTOP_PIN_CODE_MAX_LEN];
    uint8_t length;
} DesktopPinCode;

bool desktop_pin_code_is_set(void);

void desktop_pin_code_set(const DesktopPinCode* pin_code);

void desktop_pin_code_reset(void);

bool desktop_pin_code_check(const DesktopPinCode* pin_code);

bool desktop_pin_code_is_equal(const DesktopPinCode* pin_code1, const DesktopPinCode* pin_code2);

void desktop_pin_lock_error_notify(void);

uint32_t desktop_pin_lock_get_fail_timeout(void);
