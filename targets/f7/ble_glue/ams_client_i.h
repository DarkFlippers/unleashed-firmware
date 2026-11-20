#pragma once

#include <furi_ble/ams_client.h>

#ifdef __cplusplus
extern "C" {
#endif

void ams_client_init(void);

void ams_client_deinit(void);

bool ams_client_start(uint16_t connection_handle);

bool ams_client_start_test(uint16_t connection_handle);

#ifdef __cplusplus
}
#endif
