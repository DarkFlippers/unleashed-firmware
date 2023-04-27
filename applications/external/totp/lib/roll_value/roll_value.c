#include "roll_value.h"

#define TOTP_ROLL_VALUE_FN(type, step_type)                            \
    TOTP_ROLL_VALUE_FN_HEADER(type, step_type) {                       \
        type v = *value;                                               \
        if(step > 0 && v > max - step) {                               \
            if(overflow_behavior == RollOverflowBehaviorRoll) {        \
                v = min;                                               \
            } else if(overflow_behavior == RollOverflowBehaviorStop) { \
                v = max;                                               \
            }                                                          \
        } else if(step < 0 && v < min - step) {                        \
            if(overflow_behavior == RollOverflowBehaviorRoll) {        \
                v = max;                                               \
            } else if(overflow_behavior == RollOverflowBehaviorStop) { \
                v = min;                                               \
            }                                                          \
        } else {                                                       \
            v += step;                                                 \
        }                                                              \
        *value = v;                                                    \
    }

TOTP_ROLL_VALUE_FN(int8_t, int8_t)

TOTP_ROLL_VALUE_FN(uint8_t, int8_t)

TOTP_ROLL_VALUE_FN(size_t, int16_t);