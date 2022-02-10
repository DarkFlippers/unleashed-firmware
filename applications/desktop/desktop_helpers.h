#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "desktop.h"

void desktop_helpers_emit_error_notification();
void desktop_helpers_lock_system(Desktop* desktop, bool hard_lock);
void desktop_helpers_unlock_system(Desktop* desktop);
uint32_t desktop_helpers_get_pin_fail_timeout(uint32_t pin_fails);
