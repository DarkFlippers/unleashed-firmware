#include <furi-hal-bt.h>
#include <ble.h>
#include <stm32wbxx.h>
#include <shci.h>
#include <cmsis_os2.h>

#include <furi.h>

#define TAG "FuriHalBt"

osMutexId_t furi_hal_bt_core2_mtx = NULL;

void furi_hal_bt_init() {
    furi_hal_bt_core2_mtx = osMutexNew(NULL);
    furi_assert(furi_hal_bt_core2_mtx);

    // Explicitly tell that we are in charge of CLK48 domain
    HAL_HSEM_FastTake(CFG_HW_CLK48_CONFIG_SEMID);

    // Start Core2
    ble_glue_init();
}

void furi_hal_bt_lock_core2() {
    furi_assert(furi_hal_bt_core2_mtx);
    furi_check(osMutexAcquire(furi_hal_bt_core2_mtx, osWaitForever) == osOK);
}

void furi_hal_bt_unlock_core2() {
    furi_assert(furi_hal_bt_core2_mtx);
    furi_check(osMutexRelease(furi_hal_bt_core2_mtx) == osOK);
}

bool furi_hal_bt_start_core2() {
    furi_assert(furi_hal_bt_core2_mtx);

    osMutexAcquire(furi_hal_bt_core2_mtx, osWaitForever);
    // Explicitly tell that we are in charge of CLK48 domain
    HAL_HSEM_FastTake(CFG_HW_CLK48_CONFIG_SEMID);
    // Start Core2
    bool ret = ble_glue_start();
    osMutexRelease(furi_hal_bt_core2_mtx);

    return ret;
}

bool furi_hal_bt_init_app(BleEventCallback event_cb, void* context) {
    furi_assert(event_cb);
    return gap_init(event_cb, context);
}

void furi_hal_bt_start_advertising() {
    if(gap_get_state() == GapStateIdle) {
        gap_start_advertising();
    }
}

void furi_hal_bt_stop_advertising() {
    if(furi_hal_bt_is_active()) {
        gap_stop_advertising();
        while(furi_hal_bt_is_active()) {
            osDelay(1);
        }
    }
}

void furi_hal_bt_set_data_event_callbacks(uint16_t buff_size, SerialSvcDataReceivedCallback on_received_cb, SerialSvcDataSentCallback on_sent_cb, void* context) {
    serial_svc_set_callbacks(buff_size, on_received_cb, on_sent_cb, context);
}

void furi_hal_bt_notify_buffer_is_empty() {
    serial_svc_notify_buffer_is_empty();
}

bool furi_hal_bt_tx(uint8_t* data, uint16_t size) {
    if(size > FURI_HAL_BT_PACKET_SIZE_MAX) {
        return false;
    }
    return serial_svc_update_tx(data, size);
}

void furi_hal_bt_get_key_storage_buff(uint8_t** key_buff_addr, uint16_t* key_buff_size) {
    ble_app_get_key_storage_buff(key_buff_addr, key_buff_size);
}

void furi_hal_bt_set_key_storage_change_callback(BleGlueKeyStorageChangedCallback callback, void* context) {
    furi_assert(callback);
    ble_glue_set_key_storage_changed_callback(callback, context);
}

void furi_hal_bt_nvm_sram_sem_acquire() {
    while(HAL_HSEM_FastTake(CFG_HW_BLE_NVM_SRAM_SEMID) != HAL_OK) {
        osDelay(1);
    }
}

void furi_hal_bt_nvm_sram_sem_release() {
    HAL_HSEM_Release(CFG_HW_BLE_NVM_SRAM_SEMID, 0);
}

void furi_hal_bt_dump_state(string_t buffer) {
    if (furi_hal_bt_is_alive()) {
        uint8_t HCI_Version;
        uint16_t HCI_Revision;
        uint8_t LMP_PAL_Version;
        uint16_t Manufacturer_Name;
        uint16_t LMP_PAL_Subversion;

        tBleStatus ret = hci_read_local_version_information(
            &HCI_Version, &HCI_Revision, &LMP_PAL_Version, &Manufacturer_Name, &LMP_PAL_Subversion
        );

        string_cat_printf(buffer,
            "Ret: %d, HCI_Version: %d, HCI_Revision: %d, LMP_PAL_Version: %d, Manufacturer_Name: %d, LMP_PAL_Subversion: %d",
            ret, HCI_Version, HCI_Revision, LMP_PAL_Version, Manufacturer_Name, LMP_PAL_Subversion
        );
    } else {
        string_cat_printf(buffer, "BLE not ready");
    }
}

bool furi_hal_bt_is_alive() {
    return ble_glue_is_alive();
}

bool furi_hal_bt_is_active() {
    return gap_get_state() > GapStateIdle;
}

void furi_hal_bt_start_tone_tx(uint8_t channel, uint8_t power) {
    aci_hal_set_tx_power_level(0, power);
    aci_hal_tone_start(channel, 0);
}

void furi_hal_bt_stop_tone_tx() {
    aci_hal_tone_stop();
}

void furi_hal_bt_start_packet_tx(uint8_t channel, uint8_t pattern, uint8_t datarate) {
    hci_le_enhanced_transmitter_test(channel, 0x25, pattern, datarate);
}

void furi_hal_bt_start_packet_rx(uint8_t channel, uint8_t datarate) {
    hci_le_enhanced_receiver_test(channel, datarate, 0);
}

uint16_t furi_hal_bt_stop_packet_test() {
    uint16_t num_of_packets = 0;
    hci_le_test_end(&num_of_packets);
    return num_of_packets;
}

void furi_hal_bt_start_rx(uint8_t channel) {
    aci_hal_rx_start(channel);
}

float furi_hal_bt_get_rssi() {
    float val;
    uint8_t rssi_raw[3];

    if (aci_hal_read_raw_rssi(rssi_raw) != BLE_STATUS_SUCCESS) {
        return 0.0f;
    }

    // Some ST magic with rssi
    uint8_t agc = rssi_raw[2] & 0xFF;
    int rssi = (((int)rssi_raw[1] << 8) & 0xFF00) + (rssi_raw[0] & 0xFF);
    if(rssi == 0 || agc > 11) {
        val = -127.0;
    } else {
        val = agc * 6.0f - 127.0f;
        while(rssi > 30) {
            val += 6.0;
            rssi >>=1;
        }
        val += (417 * rssi + 18080) >> 10;
    }
    return val;
}

uint32_t furi_hal_bt_get_transmitted_packets() {
    uint32_t packets = 0;
    aci_hal_le_tx_test_packet_number(&packets);
    return packets;
}

void furi_hal_bt_stop_rx() {
    aci_hal_rx_stop();
}
