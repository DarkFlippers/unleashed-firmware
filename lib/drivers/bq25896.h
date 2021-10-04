#pragma once

#include <stdbool.h>
#include <stdint.h>

/** Initialize Driver */
void bq25896_init();

/** Send device into shipping mode */
void bq25896_poweroff();

/** Is currently charging */
bool bq25896_is_charging();

/** Enable charging */
void bq25896_enable_charging();

/** Disable charging */
void bq25896_disable_charging();

/** Enable otg */
void bq25896_enable_otg();

/** Disable otg */
void bq25896_disable_otg();

/** Is otg enabled */
bool bq25896_is_otg_enabled();

/** Get VBUS Voltage in mV */
uint16_t bq25896_get_vbus_voltage();

/** Get VSYS Voltage in mV */
uint16_t bq25896_get_vsys_voltage();

/** Get VBAT Voltage in mV */
uint16_t bq25896_get_vbat_voltage();

/** Get VBAT current in mA */
uint16_t bq25896_get_vbat_current();

/** Get NTC voltage in mpct of REGN */
uint32_t bq25896_get_ntc_mpct();
