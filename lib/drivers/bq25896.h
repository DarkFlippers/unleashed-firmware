#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi-hal-i2c.h>

/** Initialize Driver */
void bq25896_init(FuriHalI2cBusHandle* handle);

/** Send device into shipping mode */
void bq25896_poweroff(FuriHalI2cBusHandle* handle);

/** Is currently charging */
bool bq25896_is_charging(FuriHalI2cBusHandle* handle);

/** Enable charging */
void bq25896_enable_charging(FuriHalI2cBusHandle* handle);

/** Disable charging */
void bq25896_disable_charging(FuriHalI2cBusHandle* handle);

/** Enable otg */
void bq25896_enable_otg(FuriHalI2cBusHandle* handle);

/** Disable otg */
void bq25896_disable_otg(FuriHalI2cBusHandle* handle);

/** Is otg enabled */
bool bq25896_is_otg_enabled(FuriHalI2cBusHandle* handle);

/** Get VBUS Voltage in mV */
uint16_t bq25896_get_vbus_voltage(FuriHalI2cBusHandle* handle);

/** Get VSYS Voltage in mV */
uint16_t bq25896_get_vsys_voltage(FuriHalI2cBusHandle* handle);

/** Get VBAT Voltage in mV */
uint16_t bq25896_get_vbat_voltage(FuriHalI2cBusHandle* handle);

/** Get VBAT current in mA */
uint16_t bq25896_get_vbat_current(FuriHalI2cBusHandle* handle);

/** Get NTC voltage in mpct of REGN */
uint32_t bq25896_get_ntc_mpct(FuriHalI2cBusHandle* handle);
