/**
 * @file digital_signal_i.h
 * @brief DigitalSignal private definitions.
 *
 * This file is an implementation detail. It must not be included in
 * any public API-related headers.
 */
#include <stdint.h>
#include <stdbool.h>

#define DIGITAL_SIGNAL_T_TIM      1562 /**< 15.625 ns *100 */
#define DIGITAL_SIGNAL_T_TIM_DIV2 (DIGITAL_SIGNAL_T_TIM / 2) /**< 15.625 ns / 2 *100 */

/**
 * @brief DigitalSignal structure type.
 */
struct DigitalSignal {
    bool start_level; /**< The level to begin the signal with. */
    uint32_t size; /**< Current period count contained in the instance. */
    uint32_t max_size; /**< Maximum period count this instance can hold. */
    int32_t remainder; /**< Remainder left after converting all periods into timer ticks. */
    uint32_t data[]; /**< The array of time periods. */
};
