/**
 * @file digital_signal.h
 * @brief Simple digital signal container for the DigitalSequence library.
 *
 * Each signal is represented by its start level (high or low) and one or more periods.
 * The output will transition to its inverse value on each period boundary.
 *
 * Example: A signal with n periods and HIGH start level.
 *
 * ```
 * ----+        +------+    +- ... -+
 *  t0 |   t1   |  t2  | t3 |       | tn - 1
 *     +--------+      +----+       +--------
 * ```
 *
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// DigitalSignal uses 10 picosecond time units (1 tick = 10 ps).
// Use the macros below to convert the time from other units.

#define DIGITAL_SIGNAL_MS(x) ((x) * 100000000UL)
#define DIGITAL_SIGNAL_US(x) ((x) * 100000UL)
#define DIGITAL_SIGNAL_NS(x) ((x) * 100UL)
#define DIGITAL_SIGNAL_PS(x) ((x) / 10UL)

typedef struct DigitalSignal DigitalSignal;

/**
 * @brief Allocate a DigitalSignal instance with a defined maximum size.
 *
 * @param[in] max_size the maximum number of periods the instance will be able to contain.
 * @returns pointer to the allocated instance.
 */
DigitalSignal* digital_signal_alloc(uint32_t max_size);

/**
 * @brief Delete a previously allocated DigitalSignal instance.
 *
 * @param[in,out] signal pointer to the instance to be deleted.
 */
void digital_signal_free(DigitalSignal* signal);

/**
 * @brief Append one period to the end of the DigitalSignal instance.
 *
 * @param[in,out] signal pointer to a the instance to append to.
 * @param[in] ticks the period length, in 10 picosecond units.
 */
void digital_signal_add_period(DigitalSignal* signal, uint32_t ticks);

/**
 * @brief Append one period to the end of the DigitalSignal instance, with the level specified.
 *
 * If the level is the same as the last level contained in the instance, then it is extened
 * by the given ticks value. Otherwise, the behaviour is identical to digital_signal_add_period().
 *
 * Example 1: add tc with HIGH level
 * ```
 * before:
 * ... ------+
 *       ta  |  tb
 *           +-------
 * after:
 * ... ------+      +-------
 *       ta  |  tb  |  tc
 *           +------+
 * ```
 * Example 2: add tc with LOW level
 * ```
 * before:
 * ... ------+
 *       ta  |  tb
 *           +-------
 * after:
 * ... ------+
 *       ta  |   tb + tc
 *           +--------------
 * ```
 *
 * @param[in,out] signal pointer to the instance to append to.
 * @param[in] ticks the period length, in 10 picosecond units.
 * @param[in] level the level to be set during the period.
 */
void digital_signal_add_period_with_level(DigitalSignal* signal, uint32_t ticks, bool level);

/**
 * @brief Get the current start level contained in the DigitalSignal instance.
 *
 * If not explicitly set with digital_signal_set_start_level(), it defaults to false.
 *
 * @param[in] signal pointer to the instance to be queried.
 * @returns the start level value.
 */
bool digital_signal_get_start_level(const DigitalSignal* signal);

/**
 * @brief Set the start level contained in the DigitalSignal instance.
 *
 * @param[in,out] signal pointer to the instance to be modified.
 * @param[in] level signal level to be set as the start level.
 */
void digital_signal_set_start_level(DigitalSignal* signal, bool level);

/**
 * @brief Get the number of periods currently stored in a DigitalSignal instance.
 *
 * @param[in] signal pointer to the instance to be queried.
 * @return the number of periods stored in the instance.
 */
uint32_t digital_signal_get_size(const DigitalSignal* signal);

#ifdef __cplusplus
}
#endif
