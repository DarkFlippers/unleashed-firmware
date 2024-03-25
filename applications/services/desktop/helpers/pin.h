#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../desktop.h"
#include <desktop/desktop_settings.h>

void desktop_pin_lock_error_notify(void);

uint32_t desktop_pin_lock_get_fail_timeout(void);

bool desktop_pin_compare(const PinCode* pin_code1, const PinCode* pin_code2);
