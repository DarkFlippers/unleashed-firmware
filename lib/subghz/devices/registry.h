#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzDevice SubGhzDevice;

void subghz_device_registry_init(void);

void subghz_device_registry_deinit(void);

bool subghz_device_registry_is_valid(void);

/**
 * Registration by name SubGhzDevice.
 * @param name SubGhzDevice name
 * @return SubGhzDevice* pointer to a SubGhzDevice instance
 */
const SubGhzDevice* subghz_device_registry_get_by_name(const char* name);

/**
 * Registration subghzdevice by index in array SubGhzDevice.
 * @param index SubGhzDevice by index in array
 * @return SubGhzDevice* pointer to a SubGhzDevice instance
 */
const SubGhzDevice* subghz_device_registry_get_by_index(size_t index);

/**
 * Getting the number of registered subghzdevices.
 * @param subghz_device SubGhzDeviceRegistry
 * @return Number of subghzdevices
 */
size_t subghz_device_registry_count(void);

#ifdef __cplusplus
}
#endif
