/**
 * @file one_wire_slave_i.h
 * 
 * 1-Wire slave library, internal functions
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OneWireDevice OneWireDevice;
typedef struct OneWireSlave OneWireSlave;

/**
 * Send data, called from emulated device
 * @param bus 
 * @param address 
 * @param data_length 
 * @return bool 
 */
bool onewire_slave_send(OneWireSlave* bus, const uint8_t* address, const uint8_t data_length);

/**
 * Receive data, called from emulated device
 * @param bus 
 * @param data 
 * @param data_length 
 * @return bool 
 */
bool onewire_slave_receive(OneWireSlave* bus, uint8_t* data, const uint8_t data_length);

#ifdef __cplusplus
}
#endif
