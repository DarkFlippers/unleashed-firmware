#pragma once

#include <stdint.h>

typedef enum {
    /**
     * @brief Do not change value if it reached constraint
     */
    RollOverflowBehaviorStop,

    /**
     * @brief Set value to opposite constraint value if it reached constraint
     */
    RollOverflowBehaviorRoll
} TotpRollValueOverflowBehavior;

#define TOTP_ROLL_VALUE_FN_HEADER(type, step_type) \
    void totp_roll_value_##type(                   \
        type* value,                               \
        step_type step,                            \
        type min,                                  \
        type max,                                  \
        TotpRollValueOverflowBehavior overflow_behavior)

/**
 * @brief Rolls \c int8_t \p value using \p min and \p max as an value constraints with \p step step.
 *        When value reaches constraint value \p overflow_behavior defines what to do next.
 * @param[in,out] value value to roll
 * @param step step to be used to change value
 * @param min minimal possible value
 * @param max maximum possible value
 * @param overflow_behavior defines what to do when value reaches constraint value
 */
TOTP_ROLL_VALUE_FN_HEADER(int8_t, int8_t);

/**
 * @brief Rolls \c uint8_t \p value using \p min and \p max as an value constraints with \p step step.
 *        When value reaches constraint value \p overflow_behavior defines what to do next.
 * @param[in,out] value value to roll
 * @param step step to be used to change value
 * @param min minimal possible value
 * @param max maximum possible value
 * @param overflow_behavior defines what to do when value reaches constraint value
 */
TOTP_ROLL_VALUE_FN_HEADER(uint8_t, int8_t);

/**
 * @brief Rolls \c uint16_t \p value using \p min and \p max as an value constraints with \p step step.
 *        When value reaches constraint value \p overflow_behavior defines what to do next.
 * @param[in,out] value value to roll
 * @param step step to be used to change value
 * @param min minimal possible value
 * @param max maximum possible value
 * @param overflow_behavior defines what to do when value reaches constraint value
 */
TOTP_ROLL_VALUE_FN_HEADER(uint16_t, int16_t);