/**
 * @file one_wire_slave.h
 * 
 * 1-Wire slave library.
 */

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OneWireDevice OneWireDevice;
typedef struct OneWireSlave OneWireSlave;

typedef void (*OneWireSlaveResetCallback)(void* context);
typedef void (*OneWireSlaveResultCallback)(void* context);
typedef bool (*OneWireSlaveCommandCallback)(uint8_t command, void* context);

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
 * TODO: description comment
 */
bool onewire_slave_receive_bit(OneWireSlave* bus);

/**
 * TODO: description comment
 */
bool onewire_slave_send_bit(OneWireSlave* bus, bool value);

/**
 * Send data
 * @param bus
 * @param data
 * @param data_size
 * @return bool
 */
bool onewire_slave_send(OneWireSlave* bus, const uint8_t* data, size_t data_size);

/**
 * Receive data
 * @param bus
 * @param data
 * @param data_size
 * @return bool
 */
bool onewire_slave_receive(OneWireSlave* bus, uint8_t* data, size_t data_size);

/**
 * Set a callback to be called on each reset
 * @param bus
 * @param callback
 * @param context
 */
void onewire_slave_set_reset_callback(
    OneWireSlave* bus,
    OneWireSlaveResetCallback callback,
    void* context);

/**
 * Set a callback to be called on each command
 * @param bus
 * @param callback
 * @param context
 */
void onewire_slave_set_command_callback(
    OneWireSlave* bus,
    OneWireSlaveCommandCallback callback,
    void* context);

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
