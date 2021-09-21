#include <furi-hal-bt.h>
#include <app_entry.h>
#include <ble.h>
#include <stm32wbxx.h>
#include <shci.h>
#include <cmsis_os2.h>
#include <gap.h>

void furi_hal_bt_init() {
    // Explicitly tell that we are in charge of CLK48 domain
    HAL_HSEM_FastTake(CFG_HW_CLK48_CONFIG_SEMID);
    // Start Core2, init HCI and start GAP/GATT
    APPE_Init();
}

bool furi_hal_bt_init_app() {
    return gap_init();
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

void furi_hal_bt_dump_state(string_t buffer) {
    BleGlueStatus status = APPE_Status();
    if (status == BleGlueStatusStarted) {
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
    BleGlueStatus status = APPE_Status();
    return (status == BleGlueStatusBroken) || (status == BleGlueStatusStarted);
}

bool furi_hal_bt_is_active() {
    return gap_get_state() > GapStateIdle;
}

bool furi_hal_bt_wait_startup() {
    uint16_t counter = 0;
    while (!(APPE_Status() == BleGlueStatusStarted || APPE_Status() == BleGlueStatusBroken)) {
        osDelay(10);
        counter++;
        if (counter > 1000) {
            return false;
        }
    }
    return true;
}

bool furi_hal_bt_lock_flash(bool erase_flag) {
    if (!furi_hal_bt_wait_startup()) {
        return false;
    }
    
    while (HAL_HSEM_FastTake(CFG_HW_FLASH_SEMID) != HAL_OK) {
        osDelay(1);
    }

    HAL_FLASH_Unlock();

    if(erase_flag) SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_ON);

    while(LL_FLASH_IsActiveFlag_OperationSuspended()) {
        osDelay(1);
    };

    __disable_irq();

    return true;
}

void furi_hal_bt_unlock_flash(bool erase_flag) {
    __enable_irq();

    if(erase_flag) SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_OFF);

    HAL_FLASH_Lock();

    HAL_HSEM_Release(CFG_HW_FLASH_SEMID, HSEM_CPU1_COREID);
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
