#include "bad_usb_hid.h"
#include <extra_profiles/hid_profile.h>
#include <bt/bt_service/bt.h>
#include <storage/storage.h>

#define TAG "BadUSB HID"

#define HID_BT_KEYS_STORAGE_NAME ".bt_hid.keys"

void* hid_usb_init(FuriHalUsbHidConfig* hid_cfg) {
    furi_check(furi_hal_usb_set_config(&usb_hid, hid_cfg));
    return NULL;
}

void hid_usb_deinit(void* inst) {
    UNUSED(inst);
    furi_check(furi_hal_usb_set_config(NULL, NULL));
}

void hid_usb_set_state_callback(void* inst, HidStateCallback cb, void* context) {
    UNUSED(inst);
    furi_hal_hid_set_state_callback(cb, context);
}

bool hid_usb_is_connected(void* inst) {
    UNUSED(inst);
    return furi_hal_hid_is_connected();
}

bool hid_usb_kb_press(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_kb_press(button);
}

bool hid_usb_kb_release(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_kb_release(button);
}

bool hid_usb_consumer_press(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_consumer_key_press(button);
}

bool hid_usb_consumer_release(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_consumer_key_release(button);
}

bool hid_usb_release_all(void* inst) {
    UNUSED(inst);
    bool state = furi_hal_hid_kb_release_all();
    state &= furi_hal_hid_consumer_key_release_all();
    return state;
}

uint8_t hid_usb_get_led_state(void* inst) {
    UNUSED(inst);
    return furi_hal_hid_get_led_state();
}

static const BadUsbHidApi hid_api_usb = {
    .init = hid_usb_init,
    .deinit = hid_usb_deinit,
    .set_state_callback = hid_usb_set_state_callback,
    .is_connected = hid_usb_is_connected,

    .kb_press = hid_usb_kb_press,
    .kb_release = hid_usb_kb_release,
    .consumer_press = hid_usb_consumer_press,
    .consumer_release = hid_usb_consumer_release,
    .release_all = hid_usb_release_all,
    .get_led_state = hid_usb_get_led_state,
};

typedef struct {
    Bt* bt;
    FuriHalBleProfileBase* profile;
    HidStateCallback state_callback;
    void* callback_context;
    bool is_connected;
} BleHidInstance;

static const BleProfileHidParams ble_hid_params = {
    .device_name_prefix = "BadUSB",
    .mac_xor = 0x0002,
};

static void hid_ble_connection_status_callback(BtStatus status, void* context) {
    furi_assert(context);
    BleHidInstance* ble_hid = context;
    ble_hid->is_connected = (status == BtStatusConnected);
    if(ble_hid->state_callback) {
        ble_hid->state_callback(ble_hid->is_connected, ble_hid->callback_context);
    }
}

void* hid_ble_init(FuriHalUsbHidConfig* hid_cfg) {
    UNUSED(hid_cfg);
    BleHidInstance* ble_hid = malloc(sizeof(BleHidInstance));
    ble_hid->bt = furi_record_open(RECORD_BT);
    bt_disconnect(ble_hid->bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    bt_keys_storage_set_storage_path(ble_hid->bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    ble_hid->profile = bt_profile_start(ble_hid->bt, ble_profile_hid, (void*)&ble_hid_params);
    furi_check(ble_hid->profile);

    furi_hal_bt_start_advertising();

    bt_set_status_changed_callback(ble_hid->bt, hid_ble_connection_status_callback, ble_hid);

    return ble_hid;
}

void hid_ble_deinit(void* inst) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);

    bt_set_status_changed_callback(ble_hid->bt, NULL, NULL);
    bt_disconnect(ble_hid->bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);
    bt_keys_storage_set_default_path(ble_hid->bt);

    furi_check(bt_profile_restore_default(ble_hid->bt));
    furi_record_close(RECORD_BT);
    free(ble_hid);
}

void hid_ble_set_state_callback(void* inst, HidStateCallback cb, void* context) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    ble_hid->state_callback = cb;
    ble_hid->callback_context = context;
}

bool hid_ble_is_connected(void* inst) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_hid->is_connected;
}

bool hid_ble_kb_press(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_kb_press(ble_hid->profile, button);
}

bool hid_ble_kb_release(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_kb_release(ble_hid->profile, button);
}

bool hid_ble_consumer_press(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_consumer_key_press(ble_hid->profile, button);
}

bool hid_ble_consumer_release(void* inst, uint16_t button) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    return ble_profile_hid_consumer_key_release(ble_hid->profile, button);
}

bool hid_ble_release_all(void* inst) {
    BleHidInstance* ble_hid = inst;
    furi_assert(ble_hid);
    bool state = ble_profile_hid_kb_release_all(ble_hid->profile);
    state &= ble_profile_hid_consumer_key_release_all(ble_hid->profile);
    return state;
}

uint8_t hid_ble_get_led_state(void* inst) {
    UNUSED(inst);
    FURI_LOG_W(TAG, "hid_ble_get_led_state not implemented");
    return 0;
}

static const BadUsbHidApi hid_api_ble = {
    .init = hid_ble_init,
    .deinit = hid_ble_deinit,
    .set_state_callback = hid_ble_set_state_callback,
    .is_connected = hid_ble_is_connected,

    .kb_press = hid_ble_kb_press,
    .kb_release = hid_ble_kb_release,
    .consumer_press = hid_ble_consumer_press,
    .consumer_release = hid_ble_consumer_release,
    .release_all = hid_ble_release_all,
    .get_led_state = hid_ble_get_led_state,
};

const BadUsbHidApi* bad_usb_hid_get_interface(BadUsbHidInterface interface) {
    if(interface == BadUsbHidInterfaceUsb) {
        return &hid_api_usb;
    } else {
        return &hid_api_ble;
    }
}

void bad_usb_hid_ble_remove_pairing(void) {
    Bt* bt = furi_record_open(RECORD_BT);
    bt_disconnect(bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    furi_hal_bt_stop_advertising();

    bt_keys_storage_set_storage_path(bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));
    bt_forget_bonded_devices(bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);
    bt_keys_storage_set_default_path(bt);

    furi_check(bt_profile_restore_default(bt));
    furi_record_close(RECORD_BT);
}
