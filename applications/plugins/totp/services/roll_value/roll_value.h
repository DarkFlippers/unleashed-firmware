#pragma once

#include <stdint.h>

typedef enum { RollOverflowBehaviorStop, RollOverflowBehaviorRoll } TotpRollValueOverflowBehavior;

#define TOTP_ROLL_VALUE_FN_HEADER(type, step_type) \
    void totp_roll_value_##type(                   \
        type* value,                               \
        step_type step,                            \
        type min,                                  \
        type max,                                  \
        TotpRollValueOverflowBehavior overflow_behavior)

TOTP_ROLL_VALUE_FN_HEADER(int8_t, int8_t);
TOTP_ROLL_VALUE_FN_HEADER(uint8_t, int8_t);
TOTP_ROLL_VALUE_FN_HEADER(uint16_t, int16_t);