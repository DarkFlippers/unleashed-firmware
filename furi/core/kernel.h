/**
 * @file kernel.h
 * Furi Kernel primitives
 */
#pragma once

#include <core/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Check if CPU is in IRQ or kernel running and IRQ is masked
 * 
 * Originally this primitive was born as a workaround for FreeRTOS kernel primitives shenanigans with PRIMASK.
 * 
 * Meaningful use cases are:
 * 
 * - When kernel is started and you want to ensure that you are not in IRQ or IRQ is not masked(like in critical section)
 * - When kernel is not started and you want to make sure that you are not in IRQ mode, ignoring PRIMASK.
 * 
 * As you can see there will be edge case when kernel is not started and PRIMASK is not 0 that may cause some funky behavior.
 * Most likely it will happen after kernel primitives being used, but control not yet passed to kernel.
 * It's up to you to figure out if it is safe for your code or not.
 * 
 * @return     true if CPU is in IRQ or kernel running and IRQ is masked
 */
bool furi_kernel_is_irq_or_masked(void);

/** Check if kernel is running
 *
 * @return     true if running, false otherwise
 */
bool furi_kernel_is_running(void);

/** Lock kernel, pause process scheduling
 *
 * @warning This should never be called in interrupt request context.
 *
 * @return     previous lock state(0 - unlocked, 1 - locked)
 */
int32_t furi_kernel_lock(void);

/** Unlock kernel, resume process scheduling
 *
 * @warning This should never be called in interrupt request context.
 *
 * @return     previous lock state(0 - unlocked, 1 - locked)
 */
int32_t furi_kernel_unlock(void);

/** Restore kernel lock state
 *
 * @warning This should never be called in interrupt request context.
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
uint32_t furi_kernel_get_tick_frequency(void);

/** Delay execution
 *
 * @warning This should never be called in interrupt request context.
 *
 * Also keep in mind delay is aliased to scheduler timer intervals.
 *
 * @param[in]  ticks  The ticks count to pause
 */
void furi_delay_tick(uint32_t ticks);

/** Delay until tick
 *
 * @warning This should never be called in interrupt request context.
 *
 * @param[in]  tick  The tick until which kernel should delay task execution
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
