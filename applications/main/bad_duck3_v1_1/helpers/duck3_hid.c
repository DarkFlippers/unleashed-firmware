#include "duck3_hid.h"
#include <bt/bt_service/bt.h>
#include <bt/bt_service/bt_i.h>
#include <storage/storage.h>
#include <ble_profile/extra_profiles/hid_profile.h>

#define TAG "Duck3 HID"

#define HID_BT_KEYS_STORAGE_NAME ".bt_hid.keys"

// BLE profile template for extended HID
typedef struct {
    char name[FURI_HAL_BT_ADV_NAME_LENGTH];
    uint8_t mac[GAP_MAC_ADDR_SIZE];
    bool bonding;
    GapPairing pairing;
} BleProfileHidExtParams;

// ============================================================================
// USB HID Implementation
// ============================================================================

static void hid_usb_adjust_config(Duck3HidConfig* hid_cfg) {
    if(hid_cfg->usb.vid == 0) hid_cfg->usb.vid = HID_VID_DEFAULT;
    if(hid_cfg->usb.pid == 0) hid_cfg->usb.pid = HID_PID_DEFAULT;
}

static void* hid_usb_init(Duck3HidConfig* hid_cfg) {
    hid_usb_adjust_config(hid_cfg);
    FuriHalUsbHidConfig usb_cfg = {
        .vid = hid_cfg->usb.vid,
        .pid = hid_cfg->usb.pid,
    };
    strlcpy(usb_cfg.manuf, hid_cfg->usb.manuf, sizeof(usb_cfg.manuf));
    strlcpy(usb_cfg.product, hid_cfg->usb.product, sizeof(usb_cfg.product));
    furi_check(furi_hal_usb_set_config(&usb_hid, &usb_cfg));
    return NULL;
}

static void hid_usb_deinit(void* inst) {
    UNUSED(inst);
    furi_check(furi_hal_usb_set_config(NULL, NULL));
}

static void hid_usb_set_state_callback(void* inst, Duck3HidStateCallback cb, void* context) {
    UNUSED(inst);
    furi_hal_hid_set_state_callback(cb, context);
}

static bool hid_usb_is_connected(void* inst) {
    UNUSED(inst);
    return furi_hal_hid_is_connected();
}

static bool hid_usb_kb_press(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_kb_press(button);
}

static bool hid_usb_kb_release(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_kb_release(button);
}

static bool hid_usb_mouse_press(void* inst, uint8_t button) {
    UNUSED(inst);
    return furi_hal_hid_mouse_press(button);
}

static bool hid_usb_mouse_release(void* inst, uint8_t button) {
    UNUSED(inst);
    return furi_hal_hid_mouse_release(button);
}

static bool hid_usb_mouse_scroll(void* inst, int8_t delta) {
    UNUSED(inst);
    return furi_hal_hid_mouse_scroll(delta);
}

static bool hid_usb_mouse_move(void* inst, int8_t dx, int8_t dy) {
    UNUSED(inst);
    return furi_hal_hid_mouse_move(dx, dy);
}

static bool hid_usb_consumer_press(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_consumer_key_press(button);
}

static bool hid_usb_consumer_release(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_consumer_key_release(button);
}

static bool hid_usb_release_all(void* inst) {
    UNUSED(inst);
    bool state = furi_hal_hid_kb_release_all();
    state &= furi_hal_hid_consumer_key_release_all();
    state &= furi_hal_hid_mouse_release(0);
    return state;
}

static uint8_t hid_usb_get_led_state(void* inst) {
    UNUSED(inst);
    return furi_hal_hid_get_led_state();
}

static const Duck3HidApi hid_api_usb = {
    .adjust_config = hid_usb_adjust_config,
    .init = hid_usb_init,
    .deinit = hid_usb_deinit,
    .set_state_callback = hid_usb_set_state_callback,
    .is_connected = hid_usb_is_connected,
    .kb_press = hid_usb_kb_press,
    .kb_release = hid_usb_kb_release,
    .mouse_press = hid_usb_mouse_press,
    .mouse_release = hid_usb_mouse_release,
    .mouse_scroll = hid_usb_mouse_scroll,
    .mouse_move = hid_usb_mouse_move,
    .consumer_press = hid_usb_consumer_press,
    .consumer_release = hid_usb_consumer_release,
    .release_all = hid_usb_release_all,
    .get_led_state = hid_usb_get_led_state,
};

// ============================================================================
// BLE HID Implementation
// ============================================================================

typedef struct {
    Bt* bt;
    FuriHalBleProfileBase* profile;
    Duck3HidStateCallback state_callback;
    void* callback_context;
    bool is_connected;
} BleHidInstance;

static void hid_ble_connection_status_callback(BtStatus status, void* context) {
    furi_assert(context);
    BleHidInstance* ble_hid = context;
    ble_hid->is_connected = (status == BtStatusConnected);
    if(ble_hid->state_callback) {
        ble_hid->state_callback(ble_hid->is_connected, ble_hid->callback_context);
    }
}

static void hid_ble_adjust_config(Duck3HidConfig* hid_cfg) {
    const uint8_t* normal_mac = furi_hal_version_get_ble_mac();
    uint8_t empty_mac[GAP_MAC_ADDR_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t default_mac[GAP_MAC_ADDR_SIZE] = {0x6c, 0x7a, 0xd8, 0xac, 0x57, 0x72};

    if(memcmp(hid_cfg->ble.mac, empty_mac, sizeof(hid_cfg->ble.mac)) == 0 ||
       memcmp(hid_cfg->ble.mac, normal_mac, sizeof(hid_cfg->ble.mac)) == 0 ||
       memcmp(hid_cfg->ble.mac, default_mac, sizeof(hid_cfg->ble.mac)) == 0) {
        memcpy(hid_cfg->ble.mac, normal_mac, sizeof(hid_cfg->ble.mac));
        hid_cfg->ble.mac[2]++;
        uint16_t duck3_mac_xor = 0x0003;
        hid_cfg->ble.mac[0] ^= duck3_mac_xor;
        hid_cfg->ble.mac[1] ^= duck3_mac_xor >> 8;
    }

    if(hid_cfg->ble.name[0] == '\0') {
        const char* duck3_device_name_prefix = "Duck3";
        snprintf(
            hid_cfg->ble.name,
            sizeof(hid_cfg->ble.name),
            "%s %s",
            duck3_device_name_prefix,
            furi_hal_version_get_name_ptr());
    }

    if(hid_cfg->ble.pairing >= GapPairingCount) {
        hid_cfg->ble.pairing = GapPairingPinCodeVerifyYesNo;
    }
}

static void* hid_ble_init(Duck3HidConfig* hid_cfg) {
    BleHidInstance* ble_hid = malloc(sizeof(BleHidInstance));
    ble_hid->bt = furi_record_open(RECORD_BT);
    ble_hid->bt->suppress_pin_screen = true;
    bt_disconnect(ble_hid->bt);

    furi_delay_ms(200);

    bt_keys_storage_set_storage_path(ble_hid->bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    hid_ble_adjust_config(hid_cfg);

    BleProfileHidParams ble_params = {
        .device_name_prefix = hid_cfg->ble.name,
        .mac_xor = 0x0003,
    };

    ble_hid->profile = bt_profile_start(ble_hid->bt, ble_profile_hid, &ble_params);
    furi_check(ble_hid->profile);

    furi_hal_bt_start_advertising();

    bt_set_status_changed_callback(ble_hid->bt, hid_ble_connection_status_callback, ble_hid);

    return ble_hid;
}

static void hid_ble_deinit(void* inst) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);

    bt_set_status_changed_callback(ble_hid->bt, NULL, NULL);
    bt_disconnect(ble_hid->bt);

    furi_delay_ms(200);
    bt_keys_storage_set_default_path(ble_hid->bt);

    furi_check(bt_profile_restore_default(ble_hid->bt));
    ble_hid->bt->suppress_pin_screen = false;
    furi_record_close(RECORD_BT);
    free(ble_hid);
}

static void hid_ble_set_state_callback(void* inst, Duck3HidStateCallback cb, void* context) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    ble_hid->state_callback = cb;
    ble_hid->callback_context = context;
}

static bool hid_ble_is_connected(void* inst) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_hid->is_connected;
}

static bool hid_ble_kb_press(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_kb_press(ble_hid->profile, button);
}

static bool hid_ble_kb_release(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_kb_release(ble_hid->profile, button);
}

static bool hid_ble_mouse_press(void* inst, uint8_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_mouse_press(ble_hid->profile, button);
}

static bool hid_ble_mouse_release(void* inst, uint8_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_mouse_release(ble_hid->profile, button);
}

static bool hid_ble_mouse_scroll(void* inst, int8_t delta) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_mouse_scroll(ble_hid->profile, delta);
}

static bool hid_ble_mouse_move(void* inst, int8_t dx, int8_t dy) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_mouse_move(ble_hid->profile, dx, dy);
}

static bool hid_ble_consumer_press(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_consumer_key_press(ble_hid->profile, button);
}

static bool hid_ble_consumer_release(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_consumer_key_release(ble_hid->profile, button);
}

static bool hid_ble_release_all(void* inst) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    bool state = ble_profile_hid_kb_release_all(ble_hid->profile);
    state &= ble_profile_hid_consumer_key_release_all(ble_hid->profile);
    state &= ble_profile_hid_mouse_release_all(ble_hid->profile);
    return state;
}

static uint8_t hid_ble_get_led_state(void* inst) {
    UNUSED(inst);
    FURI_LOG_W(TAG, "hid_ble_get_led_state not implemented");
    return 0;
}

static const Duck3HidApi hid_api_ble = {
    .adjust_config = hid_ble_adjust_config,
    .init = hid_ble_init,
    .deinit = hid_ble_deinit,
    .set_state_callback = hid_ble_set_state_callback,
    .is_connected = hid_ble_is_connected,
    .kb_press = hid_ble_kb_press,
    .kb_release = hid_ble_kb_release,
    .mouse_press = hid_ble_mouse_press,
    .mouse_release = hid_ble_mouse_release,
    .mouse_scroll = hid_ble_mouse_scroll,
    .mouse_move = hid_ble_mouse_move,
    .consumer_press = hid_ble_consumer_press,
    .consumer_release = hid_ble_consumer_release,
    .release_all = hid_ble_release_all,
    .get_led_state = hid_ble_get_led_state,
};

// ============================================================================
// Public API
// ============================================================================

const Duck3HidApi* duck3_hid_get_interface(Duck3HidInterface interface) {
    if(interface == Duck3HidInterfaceUsb) {
        return &hid_api_usb;
    } else {
        return &hid_api_ble;
    }
}

void duck3_hid_ble_remove_pairing(void) {
    Bt* bt = furi_record_open(RECORD_BT);
    bt_disconnect(bt);

    furi_delay_ms(200);

    furi_hal_bt_stop_advertising();

    bt_keys_storage_set_storage_path(bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));
    bt_forget_bonded_devices(bt);

    furi_delay_ms(200);
    bt_keys_storage_set_default_path(bt);

    furi_check(bt_profile_restore_default(bt));
    furi_record_close(RECORD_BT);
}
