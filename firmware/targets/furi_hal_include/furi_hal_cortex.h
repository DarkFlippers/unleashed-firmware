/**
 * @file furi_hal_cortex.h
 * ARM Cortex HAL
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Cortex timer provides high precision low level expiring timer */
typedef struct {
    uint32_t start;
    uint32_t value;
} FuriHalCortexTimer;

/** Early init stage for cortex
 */
void furi_hal_cortex_init_early();

/** Microseconds delay
 *
 * @param[in]  microseconds  The microseconds to wait
 */
void furi_hal_cortex_delay_us(uint32_t microseconds);

/** Get instructions per microsecond count
 *
 * @return     instructions per microsecond count
 */
uint32_t furi_hal_cortex_instructions_per_microsecond();

/** Get Timer
 *
 * @param[in]  timeout_us  The expire timeout in us
 *
 * @return     The FuriHalCortexTimer
 */
FuriHalCortexTimer furi_hal_cortex_timer_get(uint32_t timeout_us);

/** Check if timer expired
 *
 * @param[in]  cortex_timer  The FuriHalCortexTimer
 *
 * @return     true if expired
 */
bool furi_hal_cortex_timer_is_expired(FuriHalCortexTimer cortex_timer);

/** Wait for timer expire
 *
 * @param[in]  cortex_timer  The FuriHalCortexTimer
 */
void furi_hal_cortex_timer_wait(FuriHalCortexTimer cortex_timer);

#ifdef __cplusplus
}
#endif
