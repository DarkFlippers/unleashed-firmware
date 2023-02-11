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
    CONDITIONAL_SEARCH = 0, /**< Search for alarmed device */
    NORMAL_SEARCH = 1, /**< Search all devices */
} OneWireHostSearchMode;

typedef struct OneWireHost OneWireHost;

/**
 * Allocate onewire host bus
 * @param pin
 * @return OneWireHost* 
 */
OneWireHost* onewire_host_alloc(const GpioPin* gpio_pin);

/**
 * Deallocate onewire host bus
 * @param host 
 */
void onewire_host_free(OneWireHost* host);

/**
 * Reset bus
 * @param host 
 * @return bool 
 */
bool onewire_host_reset(OneWireHost* host);

/**
 * Read one bit
 * @param host 
 * @return bool 
 */
bool onewire_host_read_bit(OneWireHost* host);

/**
 * Read one byte
 * @param host 
 * @return uint8_t 
 */
uint8_t onewire_host_read(OneWireHost* host);

/**
 * Read many bytes
 * @param host 
 * @param buffer 
 * @param count 
 */
void onewire_host_read_bytes(OneWireHost* host, uint8_t* buffer, uint16_t count);

/**
 * Write one bit
 * @param host 
 * @param value 
 */
void onewire_host_write_bit(OneWireHost* host, bool value);

/**
 * Write one byte
 * @param host 
 * @param value 
 */
void onewire_host_write(OneWireHost* host, uint8_t value);

/**
 * Skip ROM command
 * @param host 
 */
void onewire_host_skip(OneWireHost* host);

/**
 * Start working with the bus
 * @param host 
 */
void onewire_host_start(OneWireHost* host);

/**
 * Stop working with the bus
 * @param host 
 */
void onewire_host_stop(OneWireHost* host);

/**
 * 
 * @param host 
 */
void onewire_host_reset_search(OneWireHost* host);

/**
 * 
 * @param host 
 * @param family_code 
 */
void onewire_host_target_search(OneWireHost* host, uint8_t family_code);

/**
 * 
 * @param host 
 * @param newAddr 
 * @param mode 
 * @return uint8_t 
 */
uint8_t onewire_host_search(OneWireHost* host, uint8_t* new_addr, OneWireHostSearchMode mode);

#ifdef __cplusplus
}
#endif
