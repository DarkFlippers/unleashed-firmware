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

typedef bool (*OneWireSlaveResetCallback)(bool is_short, void* context);
typedef bool (*OneWireSlaveCommandCallback)(uint8_t command, void* context);
typedef void (*OneWireSlaveResultCallback)(void* context);

/**
 * Allocate OneWireSlave instance
 * @param [in] gpio_pin connection pin
 * @return pointer to OneWireSlave instance
 */
OneWireSlave* onewire_slave_alloc(const GpioPin* gpio_pin);

/**
 * Destroy OneWireSlave instance, free resources
 * @param [in] bus pointer to OneWireSlave instance
 */
void onewire_slave_free(OneWireSlave* bus);

/**
 * Start working with the bus
 * @param [in] bus pointer to OneWireSlave instance
 */
void onewire_slave_start(OneWireSlave* bus);

/**
 * Stop working with the bus
 * @param [in] bus pointer to OneWireSlave instance
 */
void onewire_slave_stop(OneWireSlave* bus);

/**
 * Receive one bit
 * @param [in] bus pointer to OneWireSlave instance
 * @return received bit value
 */
bool onewire_slave_receive_bit(OneWireSlave* bus);

/**
 * Send one bit
 * @param [in] bus pointer to OneWireSlave instance
 * @param [in] value bit value to send
 * @return true on success, false on failure
 */
bool onewire_slave_send_bit(OneWireSlave* bus, bool value);

/**
 * Send one or more bytes of data
 * @param [in] bus pointer to OneWireSlave instance
 * @param [in] data pointer to the data to send
 * @param [in] data_size size of the data to send
 * @return true on success, false on failure
 */
bool onewire_slave_send(OneWireSlave* bus, const uint8_t* data, size_t data_size);

/**
 * Receive one or more bytes of data
 * @param [in] bus pointer to OneWireSlave instance
 * @param [out] data pointer to the receive buffer
 * @param [in] data_size number of bytes to receive
 * @return true on success, false on failure
 */
bool onewire_slave_receive(OneWireSlave* bus, uint8_t* data, size_t data_size);

/**
 * Enable overdrive mode
 * @param [in] bus pointer to OneWireSlave instance
 * @param [in] set true to turn overdrive on, false to turn it off
 */
void onewire_slave_set_overdrive(OneWireSlave* bus, bool set);

/**
 * Set a callback function to be called on each reset.
 * The return value of the callback determines whether the emulated device
 * supports the short reset (passed as the is_short parameter).
 * In most applications, it should also call onewire_slave_set_overdrive()
 * to set the appropriate speed mode.
 *
 * @param [in] bus pointer to OneWireSlave instance
 * @param [in] callback pointer to a callback function
 * @param [in] context additional parameter to be passed to the callback
 */
void onewire_slave_set_reset_callback(
    OneWireSlave* bus,
    OneWireSlaveResetCallback callback,
    void* context);

/**
 * Set a callback function to be called on each command.
 * The return value of the callback determines whether further operation
 * is possible. As a rule of thumb, return true unless a critical error happened.
 *
 * @param [in] bus pointer to OneWireSlave instance
 * @param [in] callback pointer to a callback function
 * @param [in] context additional parameter to be passed to the callback
 */
void onewire_slave_set_command_callback(
    OneWireSlave* bus,
    OneWireSlaveCommandCallback callback,
    void* context);

/**
 * Set a callback to report emulation success
 * @param [in] bus pointer to OneWireSlave instance
 * @param [in] result_cb pointer to a callback function
 * @param [in] context additional parameter to be passed to the callback
 */
void onewire_slave_set_result_callback(
    OneWireSlave* bus,
    OneWireSlaveResultCallback result_cb,
    void* context);

#ifdef __cplusplus
}
#endif
