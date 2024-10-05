#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>

typedef enum {
    BadBleHidInterfaceBle,
} BadBleHidInterface;

typedef struct {
    void* (*init)(FuriHalUsbHidConfig* hid_cfg);
    void (*deinit)(void* inst);
    void (*set_state_callback)(void* inst, HidStateCallback cb, void* context);
    bool (*is_connected)(void* inst);

    bool (*kb_press)(void* inst, uint16_t button);
    bool (*kb_release)(void* inst, uint16_t button);
    bool (*consumer_press)(void* inst, uint16_t button);
    bool (*consumer_release)(void* inst, uint16_t button);
    bool (*release_all)(void* inst);
    uint8_t (*get_led_state)(void* inst);
} BadBleHidApi;

const BadBleHidApi* bad_ble_hid_get_interface(BadBleHidInterface interface);

void bad_ble_hid_ble_remove_pairing(void);

#ifdef __cplusplus
}
#endif
