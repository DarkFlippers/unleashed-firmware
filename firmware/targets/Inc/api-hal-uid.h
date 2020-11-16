#pragma once

#include <stdint.h>
#include <stddef.h>

/* Get platform UID size in bytes */
size_t api_hal_uid_size();

/* Get const pointer to UID */
const uint8_t* api_hal_uid();
