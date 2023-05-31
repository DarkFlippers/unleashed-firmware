#include "gen4.h"

#include <furi_hal_nfc.h>
#include <stdlib.h>

#define TAG "Magic"

#define MAGIC_CMD_PREFIX (0xCF)

#define MAGIC_CMD_GET_CFG (0xC6)
#define MAGIC_CMD_WRITE (0xCD)
#define MAGIC_CMD_READ (0xCE)
#define MAGIC_CMD_SET_CFG (0xF0)
#define MAGIC_CMD_FUSE_CFG (0xF1)
#define MAGIC_CMD_SET_PWD (0xFE)

#define MAGIC_BUFFER_SIZE (40)

const uint8_t MAGIC_DEFAULT_CONFIG[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x78, 0x00, 0x91, 0x02, 0xDA, 0xBC, 0x19, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x04, 0x00, 0x08, 0x00
};

const uint8_t MAGIC_DEFAULT_BLOCK0[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x04, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t MAGIC_EMPTY_BLOCK[16] = { 0 };

const uint8_t MAGIC_DEFAULT_SECTOR_TRAILER[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static bool magic_gen4_is_block_num_trailer(uint8_t n) {
    n++;
    if (n < 32 * 4) {
        return (n % 4 == 0);
    }

    return (n % 16 == 0);
}

bool magic_gen4_get_cfg(uint32_t pwd, uint8_t* config) {
    bool is_valid_config_len = false;
    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        // Start communication
        tx_data[0] = MAGIC_CMD_PREFIX;
        tx_data[1] = (uint8_t)(pwd >> 24);
        tx_data[2] = (uint8_t)(pwd >> 16);
        tx_data[3] = (uint8_t)(pwd >> 8);
        tx_data[4] = (uint8_t)pwd;
        tx_data[5] = MAGIC_CMD_GET_CFG;
        ret = furi_hal_nfc_ll_txrx(
            tx_data,
            6,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_TXRX_DEFAULT,
            furi_hal_nfc_ll_ms2fc(20));
        if(ret != FuriHalNfcReturnOk) break;
        if(rx_len != 30 && rx_len != 32) break;
        memcpy(config, rx_data, rx_len);
        is_valid_config_len = true;
    } while(false);

    return is_valid_config_len;
}

bool magic_gen4_set_cfg(uint32_t pwd, const uint8_t* config, uint8_t config_length, bool fuse) {
    bool write_success = false;
    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        // Start communication
        tx_data[0] = MAGIC_CMD_PREFIX;
        tx_data[1] = (uint8_t)(pwd >> 24);
        tx_data[2] = (uint8_t)(pwd >> 16);
        tx_data[3] = (uint8_t)(pwd >> 8);
        tx_data[4] = (uint8_t)pwd;
        tx_data[5] = fuse ? MAGIC_CMD_FUSE_CFG : MAGIC_CMD_SET_CFG;
        memcpy(tx_data + 6, config, config_length);
        ret = furi_hal_nfc_ll_txrx(
            tx_data,
            6 + config_length,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_TXRX_DEFAULT,
            furi_hal_nfc_ll_ms2fc(20));
        if(ret != FuriHalNfcReturnOk) break;
        if(rx_len != 2) break;
        write_success = true;
    } while(false);

    return write_success;
}

bool magic_gen4_set_pwd(uint32_t old_pwd, uint32_t new_pwd) {
    bool change_success = false;
    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        // Start communication
        tx_data[0] = MAGIC_CMD_PREFIX;
        tx_data[1] = (uint8_t)(old_pwd >> 24);
        tx_data[2] = (uint8_t)(old_pwd >> 16);
        tx_data[3] = (uint8_t)(old_pwd >> 8);
        tx_data[4] = (uint8_t)old_pwd;
        tx_data[5] = MAGIC_CMD_SET_PWD;
        tx_data[6] = (uint8_t)(new_pwd >> 24);
        tx_data[7] = (uint8_t)(new_pwd >> 16);
        tx_data[8] = (uint8_t)(new_pwd >> 8);
        tx_data[9] = (uint8_t)new_pwd;
        ret = furi_hal_nfc_ll_txrx(
            tx_data,
            10,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_TXRX_DEFAULT,
            furi_hal_nfc_ll_ms2fc(20));
        FURI_LOG_I(TAG, "ret %d, len %d", ret, rx_len);
        if(ret != FuriHalNfcReturnOk) break;
        if(rx_len != 2) break;
        change_success = true;
    } while(false);

    return change_success;
}

bool magic_gen4_write_blk(uint32_t pwd, uint8_t block_num, const uint8_t* data) {
    bool write_success = false;
    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        // Start communication
        tx_data[0] = MAGIC_CMD_PREFIX;
        tx_data[1] = (uint8_t)(pwd >> 24);
        tx_data[2] = (uint8_t)(pwd >> 16);
        tx_data[3] = (uint8_t)(pwd >> 8);
        tx_data[4] = (uint8_t)pwd;
        tx_data[5] = MAGIC_CMD_WRITE;
        tx_data[6] = block_num;
        memcpy(tx_data + 7, data, 16);
        ret = furi_hal_nfc_ll_txrx(
            tx_data,
            23,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_TXRX_DEFAULT,
            furi_hal_nfc_ll_ms2fc(200));
        if(ret != FuriHalNfcReturnOk) break;
        if(rx_len != 2) break;
        write_success = true;
    } while(false);

    return write_success;
}

bool magic_gen4_wipe(uint32_t pwd) {
    if(!magic_gen4_set_cfg(pwd, MAGIC_DEFAULT_CONFIG, sizeof(MAGIC_DEFAULT_CONFIG), false)) {
        FURI_LOG_E(TAG, "Set config failed");
        return false;
    }
    if(!magic_gen4_write_blk(pwd, 0, MAGIC_DEFAULT_BLOCK0)) {
        FURI_LOG_E(TAG, "Block 0 write failed");
        return false;
    }
    for(size_t i = 1; i < 64; i++) {
        const uint8_t* block = magic_gen4_is_block_num_trailer(i) ? MAGIC_DEFAULT_SECTOR_TRAILER : MAGIC_EMPTY_BLOCK;
        if(!magic_gen4_write_blk(pwd, i, block)) {
            FURI_LOG_E(TAG, "Block %d write failed", i);
            return false;
        }
    }
    for(size_t i = 65; i < 256; i++) {
        if(!magic_gen4_write_blk(pwd, i, MAGIC_EMPTY_BLOCK)) {
            FURI_LOG_E(TAG, "Block %d write failed", i);
            return false;
        }
    }

    return true;
}