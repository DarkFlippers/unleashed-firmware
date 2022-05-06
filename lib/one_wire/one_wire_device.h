/**
 * @file one_wire_device.h
 * 
 * 1-Wire slave library, device interface. Currently it can only emulate ID.
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OneWireSlave OneWireSlave;
typedef struct OneWireDevice OneWireDevice;

/**
 * Allocate onewire device with ID
 * @param id_1 
 * @param id_2 
 * @param id_3 
 * @param id_4 
 * @param id_5 
 * @param id_6 
 * @param id_7 
 * @param id_8 
 * @return OneWireDevice* 
 */
OneWireDevice* onewire_device_alloc(
    uint8_t id_1,
    uint8_t id_2,
    uint8_t id_3,
    uint8_t id_4,
    uint8_t id_5,
    uint8_t id_6,
    uint8_t id_7,
    uint8_t id_8);

/**
 * Deallocate onewire device
 * @param device 
 */
void onewire_device_free(OneWireDevice* device);

/**
 * Send ID report, called from onewire slave
 * @param device 
 */
void onewire_device_send_id(OneWireDevice* device);

/**
 * Attach device to onewire slave bus
 * @param device 
 * @param bus 
 */
void onewire_device_attach(OneWireDevice* device, OneWireSlave* bus);

/**
 * Attach device from onewire slave bus
 * @param device 
 */
void onewire_device_detach(OneWireDevice* device);

/**
 * Get pointer to device id array
 * @param device 
 * @return uint8_t* 
 */
uint8_t* onewire_device_get_id_p(OneWireDevice* device);

#ifdef __cplusplus
}
#endif
