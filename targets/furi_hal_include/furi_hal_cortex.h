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
void furi_hal_cortex_init_early(void);

/** Microseconds delay
 *
 * @param[in]  microseconds  The microseconds to wait
 */
void furi_hal_cortex_delay_us(uint32_t microseconds);

/** Get instructions per microsecond count
 *
 * @return     instructions per microsecond count
 */
uint32_t furi_hal_cortex_instructions_per_microsecond(void);

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

typedef enum {
    FuriHalCortexComp0,
    FuriHalCortexComp1,
    FuriHalCortexComp2,
    FuriHalCortexComp3,
} FuriHalCortexComp;

typedef enum {
    FuriHalCortexCompSizeWord = 0b10,
    FuriHalCortexCompSizeHalfWord = 0b01,
    FuriHalCortexCompSizeByte = 0b00,
} FuriHalCortexCompSize;

typedef enum {
    FuriHalCortexCompFunctionPC = 0b100,
    FuriHalCortexCompFunctionRead = 0b101,
    FuriHalCortexCompFunctionWrite = 0b110,
    FuriHalCortexCompFunctionReadWrite = 0b110,
} FuriHalCortexCompFunction;

/** Enable DWT comparator
 * 
 * Allows to programmatically set instruction/data breakpoints.
 * 
 * More details on how it works can be found in armv7m official documentation:
 * https://developer.arm.com/documentation/ddi0403/d/Debug-Architecture/ARMv7-M-Debug/The-Data-Watchpoint-and-Trace-unit/The-DWT-comparators
 * https://developer.arm.com/documentation/ddi0403/d/Debug-Architecture/ARMv7-M-Debug/The-Data-Watchpoint-and-Trace-unit/Comparator-Function-registers--DWT-FUNCTIONn
 *
 * @param[in]  comp      The Comparator
 * @param[in]  function  The Comparator Function to use
 * @param[in]  value     The value
 * @param[in]  mask      The mask
 * @param[in]  size      The size
 */
void furi_hal_cortex_comp_enable(
    FuriHalCortexComp comp,
    FuriHalCortexCompFunction function,
    uint32_t value,
    uint32_t mask,
    FuriHalCortexCompSize size);

/** Reset DWT comparator
 *
 * @param[in]  comp  The Comparator
 */
void furi_hal_cortex_comp_reset(FuriHalCortexComp comp);

#ifdef __cplusplus
}
#endif
