#pragma once

#include <rfal_nfc.h>
#include <st_errno.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init nfc
 */
ReturnCode api_hal_nfc_init();

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
bool api_hal_nfc_detect(rfalNfcDevice** dev_list, uint8_t* dev_cnt, uint32_t cycles);

#ifdef __cplusplus
}
#endif
