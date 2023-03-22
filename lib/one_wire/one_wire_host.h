/**
 * @file one_wire_host.h
 * 
 * 1-Wire host (master) library
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OneWireHostSearchModeConditional = 0, /**< Search for alarmed device */
    OneWireHostSearchModeNormal = 1, /**< Search for all devices */
} OneWireHostSearchMode;

typedef struct OneWireHost OneWireHost;

/**
 * Allocate OneWireHost instance
 * @param [in] gpio_pin connection pin
 * @return pointer to OneWireHost instance
 */
OneWireHost* onewire_host_alloc(const GpioPin* gpio_pin);

/**
 * Destroy OneWireHost instance, free resources
 * @param [in] host pointer to OneWireHost instance
 */
void onewire_host_free(OneWireHost* host);

/**
 * Reset the 1-Wire bus
 * @param [in] host pointer to OneWireHost instance
 * @return true if presence was detected, false otherwise
 */
bool onewire_host_reset(OneWireHost* host);

/**
 * Read one bit
 * @param [in] host pointer to OneWireHost instance
 * @return received bit value
 */
bool onewire_host_read_bit(OneWireHost* host);

/**
 * Read one byte
 * @param [in] host pointer to OneWireHost instance
 * @return received byte value
 */
uint8_t onewire_host_read(OneWireHost* host);

/**
 * Read one or more bytes
 * @param [in] host pointer to OneWireHost instance
 * @param [out] buffer received data buffer
 * @param [in] count number of bytes to read
 */
void onewire_host_read_bytes(OneWireHost* host, uint8_t* buffer, uint16_t count);

/**
 * Write one bit
 * @param [in] host pointer to OneWireHost instance
 * @param value bit value to write
 */
void onewire_host_write_bit(OneWireHost* host, bool value);

/**
 * Write one byte
 * @param [in] host pointer to OneWireHost instance
 * @param value byte value to write
 */
void onewire_host_write(OneWireHost* host, uint8_t value);

/**
 * Write one or more bytes
 * @param [in] host pointer to OneWireHost instance
 * @param [in] buffer pointer to the data to write
 * @param [in] count size of the data to write
 */
void onewire_host_write_bytes(OneWireHost* host, const uint8_t* buffer, uint16_t count);

/**
 * Start working with the bus
 * @param [in] host pointer to OneWireHost instance
 */
void onewire_host_start(OneWireHost* host);

/**
 * Stop working with the bus
 * @param [in] host pointer to OneWireHost instance
 */
void onewire_host_stop(OneWireHost* host);

/**
 * Reset previous search results
 * @param [in] host pointer to OneWireHost instance
 */
void onewire_host_reset_search(OneWireHost* host);

/**
 * Set the family code to search for
 * @param [in] host pointer to OneWireHost instance
 * @param [in] family_code device family code
 */
void onewire_host_target_search(OneWireHost* host, uint8_t family_code);

/**
 * Search for devices on the 1-Wire bus
 * @param [in] host pointer to OneWireHost instance
 * @param [out] new_addr pointer to the buffer to contain the unique ROM of the found device
 * @param [in] mode search mode
 * @return true on success, false otherwise
 */
bool onewire_host_search(OneWireHost* host, uint8_t* new_addr, OneWireHostSearchMode mode);

/**
 * Enable overdrive mode
 * @param [in] host pointer to OneWireHost instance
 * @param [in] set true to turn overdrive on, false to turn it off
 */
void onewire_host_set_overdrive(OneWireHost* host, bool set);

#ifdef __cplusplus
}
#endif
