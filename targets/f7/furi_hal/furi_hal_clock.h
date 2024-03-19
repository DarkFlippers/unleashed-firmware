#pragma once

#include <stm32wbxx_ll_rcc.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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
void furi_hal_clock_init_early(void);

/** Early deinitialization */
void furi_hal_clock_deinit_early(void);

/** Initialize clocks */
void furi_hal_clock_init(void);

/** Switch clock from HSE to HSI */
void furi_hal_clock_switch_hse2hsi(void);

/** Switch clock from HSI to HSE */
void furi_hal_clock_switch_hsi2hse(void);

/** Switch clock from HSE to PLL
 *
 * @return     true if changed, false if failed or not possible at this moment
 */
bool furi_hal_clock_switch_hse2pll(void);

/** Switch clock from PLL to HSE
 *
 * @return     true if changed, false if failed or not possible at this moment
 */
bool furi_hal_clock_switch_pll2hse(void);

/** Stop SysTick counter without resetting */
void furi_hal_clock_suspend_tick(void);

/** Continue SysTick counter operation */
void furi_hal_clock_resume_tick(void);

/** Enable clock output on MCO pin
 * 
 * @param      source  MCO clock source
 * @param      div     MCO clock division
*/
void furi_hal_clock_mco_enable(FuriHalClockMcoSourceId source, FuriHalClockMcoDivisorId div);

/** Disable clock output on MCO pin */
void furi_hal_clock_mco_disable(void);

#ifdef __cplusplus
}
#endif
