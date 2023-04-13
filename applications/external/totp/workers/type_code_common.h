#pragma once
#include <stdint.h>
#include "../types/token_info.h"

typedef bool (*TOTP_AUTOMATION_KEY_HANDLER)(uint16_t key);

void totp_type_code_worker_execute_automation(
    TOTP_AUTOMATION_KEY_HANDLER key_press_fn,
    TOTP_AUTOMATION_KEY_HANDLER key_release_fn,
    const char* code_buffer,
    uint8_t code_buffer_size,
    TokenAutomationFeature features);