#pragma once

#include <rfal_nfc.h>
#include <st_errno.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define API_HAL_NFC_UID_MAX_LEN 10

/**
 * Init nfc
 */
void api_hal_nfc_init();

/**
 * Check if nfc worker is busy
 */
bool api_hal_nfc_is_busy();

/**
 * NFC field on
 */
void api_hal_nfc_field_on();

/**
 * NFC field off
 */
void api_hal_nfc_field_off();

/**
 * NFC start sleep
 */
void api_hal_nfc_start_sleep();

/**
 * NFC stop sleep
 */
void api_hal_nfc_exit_sleep();

/**
 * NFC poll
 */
bool api_hal_nfc_detect(rfalNfcDevice** dev_list, uint8_t* dev_cnt, uint32_t timeout, bool deactivate);

/**
 * NFC listen
 */
bool api_hal_nfc_listen(uint8_t* uid, uint8_t uid_len, uint8_t* atqa, uint8_t sak, uint32_t timeout);

/**
 * NFC data exchange
 */
ReturnCode api_hal_nfc_data_exchange(uint8_t* tx_buff, uint16_t tx_len, uint8_t** rx_buff, uint16_t** rx_len, bool deactivate);

/**
 * NFC deactivate and start sleep
 */
void api_hal_nfc_deactivate();

#ifdef __cplusplus
}
#endif
