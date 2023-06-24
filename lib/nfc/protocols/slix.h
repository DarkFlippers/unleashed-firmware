#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "nfc_util.h"
#include <furi_hal_nfc.h>

#define NFCV_MANUFACTURER_NXP 0x04

/* ISO15693-3 CUSTOM NXP COMMANDS */
typedef enum {
    NFCV_CMD_NXP_SET_EAS = 0xA2,
    NFCV_CMD_NXP_RESET_EAS = 0xA3,
    NFCV_CMD_NXP_LOCK_EAS = 0xA4,
    NFCV_CMD_NXP_EAS_ALARM = 0xA5,
    NFCV_CMD_NXP_PASSWORD_PROTECT_EAS_AFI = 0xA6,
    NFCV_CMD_NXP_WRITE_EAS_ID = 0xA7,
    NFCV_CMD_NXP_GET_NXP_SYSTEM_INFORMATION = 0xAB,
    NFCV_CMD_NXP_INVENTORY_PAGE_READ = 0xB0,
    NFCV_CMD_NXP_INVENTORY_PAGE_READ_FAST = 0xB1,
    NFCV_CMD_NXP_GET_RANDOM_NUMBER = 0xB2,
    NFCV_CMD_NXP_SET_PASSWORD = 0xB3,
    NFCV_CMD_NXP_WRITE_PASSWORD = 0xB4,
    NFCV_CMD_NXP_64_BIT_PASSWORD_PROTECTION = 0xB5,
    NFCV_CMD_NXP_PROTECT_PAGE = 0xB6,
    NFCV_CMD_NXP_LOCK_PAGE_PROTECTION_CONDITION = 0xB7,
    NFCV_CMD_NXP_DESTROY = 0xB9,
    NFCV_CMD_NXP_ENABLE_PRIVACY = 0xBA,
    NFCV_CMD_NXP_STAY_QUIET_PERSISTENT = 0xBC,
    NFCV_CMD_NXP_READ_SIGNATURE = 0xBD
} SlixCommands;

/* lock bit bits used in SLIX's NXP SYSTEM INFORMATION response */
typedef enum {
    SlixLockBitAfi = 1 << 0,
    SlixLockBitEas = 1 << 1,
    SlixLockBitDsfid = 1 << 2,
    SlixLockBitPpl = 1 << 3,
} SlixLockBits;

/* available passwords */
#define SLIX_PASS_READ 0x01
#define SLIX_PASS_WRITE 0x02
#define SLIX_PASS_PRIVACY 0x04
#define SLIX_PASS_DESTROY 0x08
#define SLIX_PASS_EASAFI 0x10

#define SLIX_PASS_ALL \
    (SLIX_PASS_READ | SLIX_PASS_WRITE | SLIX_PASS_PRIVACY | SLIX_PASS_DESTROY | SLIX_PASS_EASAFI)

bool slix_check_card_type(FuriHalNfcDevData* nfc_data);
bool slix2_check_card_type(FuriHalNfcDevData* nfc_data);
bool slix_s_check_card_type(FuriHalNfcDevData* nfc_data);
bool slix_l_check_card_type(FuriHalNfcDevData* nfc_data);

ReturnCode slix2_read_custom(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data);
ReturnCode slix2_read_signature(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data);
ReturnCode slix2_read_nxp_sysinfo(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data);

ReturnCode slix_get_random(NfcVData* data);
ReturnCode slix_unlock(NfcVData* data, uint32_t password_id);

void slix_prepare(NfcVData* nfcv_data);
void slix_s_prepare(NfcVData* nfcv_data);
void slix_l_prepare(NfcVData* nfcv_data);
void slix2_prepare(NfcVData* nfcv_data);
