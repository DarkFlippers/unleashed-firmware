/**
 * @file one_wire_slave.h
 * 
 * 1-Wire slave library. Currently it can only emulate ID.
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OneWireDevice OneWireDevice;
typedef struct OneWireSlave OneWireSlave;
typedef void (*OneWireSlaveResultCallback)(void* context);

/**
 * Allocate onewire slave
 * @param gpio_pin
 * @return OneWireSlave* 
 */
OneWireSlave* onewire_slave_alloc(const GpioPin* gpio_pin);

/**
 * Free onewire slave
 * @param bus 
 */
void onewire_slave_free(OneWireSlave* bus);

/**
 * Start working with the bus
 * @param bus 
 */
void onewire_slave_start(OneWireSlave* bus);

/**
 * Stop working with the bus
 * @param bus 
 */
void onewire_slave_stop(OneWireSlave* bus);

/**
 * Attach device for emulation
 * @param bus 
 * @param device 
 */
void onewire_slave_attach(OneWireSlave* bus, OneWireDevice* device);

/**
 * Detach device from bus
 * @param bus 
 */
void onewire_slave_detach(OneWireSlave* bus);

/**
 * Set a callback to report emulation success
 * @param bus 
 * @param result_cb 
 * @param context 
 */
void onewire_slave_set_result_callback(
    OneWireSlave* bus,
    OneWireSlaveResultCallback result_cb,
    void* context);

#ifdef __cplusplus
}
#endif
