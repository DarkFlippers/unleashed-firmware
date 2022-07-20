/**
 * @file kenrel.h
 * Furi Kernel primitives
 */
#pragma once

#include <core/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Lock kernel, pause process scheduling
 *
 * @return     previous lock state(0 - unlocked, 1 - locked)
 */
int32_t furi_kernel_lock();

/** Unlock kernel, resume process scheduling
 *
 * @return     previous lock state(0 - unlocked, 1 - locked)
 */
int32_t furi_kernel_unlock();

/** Restore kernel lock state
 *
 * @param[in]  lock  The lock state
 *
 * @return     new lock state or error
 */
int32_t furi_kernel_restore_lock(int32_t lock);

/** Get kernel systick frequency
 *
 * @return     systick counts per second
 */
uint32_t furi_kernel_get_tick_frequency();

/** Delay execution
 * 
 * Also keep in mind delay is aliased to scheduler timer intervals.
 *
 * @param[in]  ticks  The ticks count to pause
 */
void furi_delay_tick(uint32_t ticks);

/** Delay until tick
 *
 * @param[in]  ticks  The tick until which kerel should delay task execution
 *
 * @return     The furi status.
 */
FuriStatus furi_delay_until_tick(uint32_t tick);

/** Get current tick counter
 *
 * System uptime, may overflow.
 *
 * @return     Current ticks in milliseconds
 */
uint32_t furi_get_tick(void);

/** Convert milliseconds to ticks
 *
 * @param[in]   milliseconds    time in milliseconds
 * @return      time in ticks
 */
uint32_t furi_ms_to_ticks(uint32_t milliseconds);

/** Delay in milliseconds
 * 
 * This method uses kernel ticks on the inside, which causes delay to be aliased to scheduler timer intervals.
 * Real wait time will be between X+ milliseconds.
 * Special value: 0, will cause task yield.
 * Also if used when kernel is not running will fall back to `furi_delay_us`.
 * 
 * @warning    Cannot be used from ISR
 *
 * @param[in]  milliseconds  milliseconds to wait
 */
void furi_delay_ms(uint32_t milliseconds);

/** Delay in microseconds
 * 
 * Implemented using Cortex DWT counter. Blocking and non aliased.
 *
 * @param[in]  microseconds  microseconds to wait
 */
void furi_delay_us(uint32_t microseconds);

#ifdef __cplusplus
}
#endif
