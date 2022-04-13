#pragma once

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
