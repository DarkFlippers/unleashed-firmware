#pragma once

/** Initialize clocks */
void furi_hal_clock_init();

/** Switch to HSI clock */
void furi_hal_clock_switch_to_hsi();

/** Switch to PLL clock */
void furi_hal_clock_switch_to_pll();
