#pragma once

#include "cc1101_regs.h"

#include <stdbool.h>
#include <stdint.h>
#include <api-hal-spi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Low level API */

/** Strobe command to the device
 * @param device - pointer to ApiHalSpiDevice
 * @param strobe - command to execute
 * @return device status
 */
CC1101Status cc1101_strobe(const ApiHalSpiDevice* device, uint8_t strobe);

/** Write device register
 * @param device - pointer to ApiHalSpiDevice
 * @param reg - register
 * @param data - data to write
 * @return device status
 */
CC1101Status cc1101_write_reg(const ApiHalSpiDevice* device, uint8_t reg, uint8_t data);

/** Read device register
 * @param device - pointer to ApiHalSpiDevice
 * @param reg - register
 * @param[out] data - pointer to data
 * @return device status
 */
CC1101Status cc1101_read_reg(const ApiHalSpiDevice* device, uint8_t reg, uint8_t* data);

/* High level API */

/** Reset
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_reset(const ApiHalSpiDevice* device);

/** Enable shutdown mode
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_shutdown(const ApiHalSpiDevice* device);

/** Get Partnumber
 * @param device - pointer to ApiHalSpiDevice
 */
uint8_t cc1101_get_partnumber(const ApiHalSpiDevice* device);

/** Get Version
 * @param device - pointer to ApiHalSpiDevice
 */
uint8_t cc1101_get_version(const ApiHalSpiDevice* device);

/** Get raw RSSI value
 * @param device - pointer to ApiHalSpiDevice
 */
uint8_t cc1101_get_rssi(const ApiHalSpiDevice* device);

/** Calibrate oscillator
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_calibrate(const ApiHalSpiDevice* device);

/** Switch to idle
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_switch_to_idle(const ApiHalSpiDevice* device);

/** Switch to RX
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_switch_to_rx(const ApiHalSpiDevice* device);

/** Switch to TX
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_switch_to_tx(const ApiHalSpiDevice* device);

/** Flush RX FIFO
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_flush_rx(const ApiHalSpiDevice* device);

/** Flush TX FIFO
 * @param device - pointer to ApiHalSpiDevice
 */
void cc1101_flush_tx(const ApiHalSpiDevice* device);

/** Set Frequency
 * Is not 100% precise, depends on quartz used
 * @param device - pointer to ApiHalSpiDevice
 * @param value - frequency in herz
 * @return real frequency that were set
 */
uint32_t cc1101_set_frequency(const ApiHalSpiDevice* device, uint32_t value);

/** Get Frequency Step
 * @param device - pointer to ApiHalSpiDevice
 * @return frequency step
 */
uint32_t cc1101_get_frequency_step(const ApiHalSpiDevice* device);

/** Set Frequency Offset
 * Is not 100% precise, depends on quartz used
 * @param device - pointer to ApiHalSpiDevice
 * @param value - frequency offset in herz
 * @return real frequency that were set
 */
uint32_t cc1101_set_frequency_offset(const ApiHalSpiDevice* device, uint32_t value);

/** Get Frequency Offset Step
 * @param device - pointer to ApiHalSpiDevice
 * @return frequency offset step
 */
uint32_t cc1101_get_frequency_offset_step(const ApiHalSpiDevice* device);

/** Set Power Amplifier level table, ramp
 * @param device - pointer to ApiHalSpiDevice
 * @param value - array of power level values
 */
void cc1101_set_pa_table(const ApiHalSpiDevice* device, const uint8_t value[8]);

/** Set Power Amplifier level table, ramp
 * @param device - pointer to ApiHalSpiDevice
 * @param value - array of power level values
 */
void cc1101_set_pa_table(const ApiHalSpiDevice* device, const uint8_t value[8]);

/** Write FIFO
 * @param device - pointer to ApiHalSpiDevice
 * @param data, pointer to byte array
 * @param size, write bytes count
 * @return size, written bytes count
 */
uint8_t cc1101_write_fifo(const ApiHalSpiDevice* device, const uint8_t* data, uint8_t size);

/** Read FIFO
 * @param device - pointer to ApiHalSpiDevice
 * @param data, pointer to byte array
 * @param size, bytes to read from fifo
 * @return size, read bytes count
 */
uint8_t cc1101_read_fifo(const ApiHalSpiDevice* device, uint8_t* data, uint8_t size);

#ifdef __cplusplus
}
#endif