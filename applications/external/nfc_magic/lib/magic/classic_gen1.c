#include "classic_gen1.h"

#include <furi_hal_nfc.h>

#define TAG "Magic"

#define MAGIC_CMD_WUPA (0x40)
#define MAGIC_CMD_ACCESS (0x43)

#define MAGIC_MIFARE_READ_CMD (0x30)
#define MAGIC_MIFARE_WRITE_CMD (0xA0)

#define MAGIC_ACK (0x0A)

#define MAGIC_BUFFER_SIZE (32)

bool magic_gen1_wupa() {
    bool magic_activated = false;
    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        // Start communication
        tx_data[0] = MAGIC_CMD_WUPA;
        ret = furi_hal_nfc_ll_txrx_bits(
            tx_data,
            7,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_TX_MANUAL | FURI_HAL_NFC_LL_TXRX_FLAGS_AGC_ON |
                FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_RX_KEEP,
            furi_hal_nfc_ll_ms2fc(20));
        if(ret != FuriHalNfcReturnIncompleteByte) break;
        if(rx_len != 4) break;
        if(rx_data[0] != MAGIC_ACK) break;
        magic_activated = true;
    } while(false);

    return magic_activated;
}

bool magic_gen1_data_access_cmd() {
    bool write_cmd_success = false;
    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        tx_data[0] = MAGIC_CMD_ACCESS;
        ret = furi_hal_nfc_ll_txrx_bits(
            tx_data,
            8,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_TX_MANUAL | FURI_HAL_NFC_LL_TXRX_FLAGS_AGC_ON |
                FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_RX_KEEP,
            furi_hal_nfc_ll_ms2fc(20));
        if(ret != FuriHalNfcReturnIncompleteByte) break;
        if(rx_len != 4) break;
        if(rx_data[0] != MAGIC_ACK) break;

        write_cmd_success = true;
    } while(false);

    return write_cmd_success;
}

bool magic_gen1_read_block(uint8_t block_num, MfClassicBlock* data) {
    furi_assert(data);

    bool read_success = false;

    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        tx_data[0] = MAGIC_MIFARE_READ_CMD;
        tx_data[1] = block_num;
        ret = furi_hal_nfc_ll_txrx_bits(
            tx_data,
            2 * 8,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_LL_TXRX_FLAGS_AGC_ON,
            furi_hal_nfc_ll_ms2fc(20));

        if(ret != FuriHalNfcReturnOk) break;
        if(rx_len != 16 * 8) break;
        memcpy(data->value, rx_data, sizeof(data->value));
        read_success = true;
    } while(false);

    return read_success;
}

bool magic_gen1_write_blk(uint8_t block_num, MfClassicBlock* data) {
    furi_assert(data);

    bool write_success = false;
    uint8_t tx_data[MAGIC_BUFFER_SIZE] = {};
    uint8_t rx_data[MAGIC_BUFFER_SIZE] = {};
    uint16_t rx_len = 0;
    FuriHalNfcReturn ret = 0;

    do {
        tx_data[0] = MAGIC_MIFARE_WRITE_CMD;
        tx_data[1] = block_num;
        ret = furi_hal_nfc_ll_txrx_bits(
            tx_data,
            2 * 8,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_LL_TXRX_FLAGS_AGC_ON | FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_RX_KEEP,
            furi_hal_nfc_ll_ms2fc(20));
        if(ret != FuriHalNfcReturnIncompleteByte) break;
        if(rx_len != 4) break;
        if(rx_data[0] != MAGIC_ACK) break;

        memcpy(tx_data, data->value, sizeof(data->value));
        ret = furi_hal_nfc_ll_txrx_bits(
            tx_data,
            16 * 8,
            rx_data,
            sizeof(rx_data),
            &rx_len,
            FURI_HAL_NFC_LL_TXRX_FLAGS_AGC_ON | FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_RX_KEEP,
            furi_hal_nfc_ll_ms2fc(20));
        if(ret != FuriHalNfcReturnIncompleteByte) break;
        if(rx_len != 4) break;
        if(rx_data[0] != MAGIC_ACK) break;

        write_success = true;
    } while(false);

    return write_success;
}
