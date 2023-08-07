#pragma once

#include "../features_config.h"

typedef uint8_t AutomationMethod;

enum AutomationMethods {
    AutomationMethodNone = 0b00,
    AutomationMethodBadUsb = 0b01,
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
    AutomationMethodBadBt = 0b10,
#endif
};
