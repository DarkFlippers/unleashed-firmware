#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>
#include <bt/bt_service/bt.h>
#include <ble_profile/extra_profiles/hid_profile.h>

typedef enum {
    Duck3HidInterfaceUsb,
    Duck3HidInterfaceBle,
    Duck3HidInterfaceMAX,
} Duck3HidInterface;

typedef struct {
    char name[FURI_HAL_BT_ADV_NAME_LENGTH];
    uint8_t mac[GAP_MAC_ADDR_SIZE];
    bool bonding;
    GapPairing pairing;
} Duck3BleConfig;

typedef struct {
    char manuf[HID_MANUF_PRODUCT_NAME_LEN];
    char product[HID_MANUF_PRODUCT_NAME_LEN];
    uint16_t vid;
    uint16_t pid;
} Duck3UsbConfig;

typedef struct {
    Duck3BleConfig ble;
    Duck3UsbConfig usb;
} Duck3HidConfig;

typedef void (*Duck3HidStateCallback)(bool state, void* context);

typedef struct {
    void (*adjust_config)(Duck3HidConfig* hid_cfg);
    void* (*init)(Duck3HidConfig* hid_cfg);
    void (*deinit)(void* inst);
    void (*set_state_callback)(void* inst, Duck3HidStateCallback cb, void* context);
    bool (*is_connected)(void* inst);

    bool (*kb_press)(void* inst, uint16_t button);
    bool (*kb_release)(void* inst, uint16_t button);
    bool (*mouse_press)(void* inst, uint8_t button);
    bool (*mouse_release)(void* inst, uint8_t button);
    bool (*mouse_scroll)(void* inst, int8_t delta);
    bool (*mouse_move)(void* inst, int8_t dx, int8_t dy);
    bool (*consumer_press)(void* inst, uint16_t button);
    bool (*consumer_release)(void* inst, uint16_t button);
    bool (*release_all)(void* inst);
    uint8_t (*get_led_state)(void* inst);
} Duck3HidApi;

const Duck3HidApi* duck3_hid_get_interface(Duck3HidInterface interface);

void duck3_hid_ble_remove_pairing(void);

#ifdef __cplusplus
}
#endif
