#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32wbxx_ll_rcc.h>

typedef enum {
    FuriHalClockMcoLse,
    FuriHalClockMcoSysclk,
    FuriHalClockMcoMsi100k,
    FuriHalClockMcoMsi200k,
    FuriHalClockMcoMsi400k,
    FuriHalClockMcoMsi800k,
    FuriHalClockMcoMsi1m,
    FuriHalClockMcoMsi2m,
    FuriHalClockMcoMsi4m,
    FuriHalClockMcoMsi8m,
    FuriHalClockMcoMsi16m,
    FuriHalClockMcoMsi24m,
    FuriHalClockMcoMsi32m,
    FuriHalClockMcoMsi48m,
} FuriHalClockMcoSourceId;

typedef enum {
    FuriHalClockMcoDiv1 = LL_RCC_MCO1_DIV_1,
    FuriHalClockMcoDiv2 = LL_RCC_MCO1_DIV_2,
    FuriHalClockMcoDiv4 = LL_RCC_MCO1_DIV_4,
    FuriHalClockMcoDiv8 = LL_RCC_MCO1_DIV_8,
    FuriHalClockMcoDiv16 = LL_RCC_MCO1_DIV_16,
} FuriHalClockMcoDivisorId;

/** Early initialization */
void furi_hal_clock_init_early();

/** Early deinitialization */
void furi_hal_clock_deinit_early();

/** Initialize clocks */
void furi_hal_clock_init();

/** Switch to HSI clock */
void furi_hal_clock_switch_to_hsi();

/** Switch to PLL clock */
void furi_hal_clock_switch_to_pll();

/** Stop SysTick counter without resetting */
void furi_hal_clock_suspend_tick();

/** Continue SysTick counter operation */
void furi_hal_clock_resume_tick();

/** Enable clock output on MCO pin
 * 
 * @param      source  MCO clock source
 * @param      div     MCO clock division
*/
void furi_hal_clock_mco_enable(FuriHalClockMcoSourceId source, FuriHalClockMcoDivisorId div);

/** Disable clock output on MCO pin */
void furi_hal_clock_mco_disable();

#ifdef __cplusplus
}
#endif
