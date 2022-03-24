/**
 * @file furi_hal_nfc.h
 * NFC HAL API
 */

#pragma once

#include <rfal_nfc.h>
#include <st_errno.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FURI_HAL_NFC_UID_MAX_LEN 10
#define FURI_HAL_NFC_DATA_BUFF_SIZE (64)
#define FURI_HAL_NFC_PARITY_BUFF_SIZE (FURI_HAL_NFC_DATA_BUFF_SIZE / 8)

#define FURI_HAL_NFC_TXRX_DEFAULT                                                    \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_AUTO | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_REMV | \
     (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_OFF | (uint32_t)RFAL_TXRX_FLAGS_AGC_ON |       \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_REMV | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_AUTO | \
     (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO)

#define FURI_HAL_NFC_TX_DEFAULT_RX_NO_CRC                                            \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_AUTO | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | \
     (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_OFF | (uint32_t)RFAL_TXRX_FLAGS_AGC_ON |       \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_REMV | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_AUTO | \
     (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO)

#define FURI_HAL_NFC_TXRX_WITH_PAR                                                     \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | \
     (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_OFF | (uint32_t)RFAL_TXRX_FLAGS_AGC_ON |         \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_AUTO |   \
     (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO)

#define FURI_HAL_NFC_TXRX_RAW                                                          \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | \
     (uint32_t)RFAL_TXRX_FLAGS_NFCIP1_OFF | (uint32_t)RFAL_TXRX_FLAGS_AGC_ON |         \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_NONE |   \
     (uint32_t)RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO)

typedef bool (*FuriHalNfcEmulateCallback)(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len,
    uint32_t* flags,
    void* context);

typedef struct {
    uint8_t tx_data[FURI_HAL_NFC_DATA_BUFF_SIZE];
    uint8_t tx_parity[FURI_HAL_NFC_PARITY_BUFF_SIZE];
    uint16_t tx_bits;
    uint8_t rx_data[FURI_HAL_NFC_DATA_BUFF_SIZE];
    uint8_t rx_parity[FURI_HAL_NFC_PARITY_BUFF_SIZE];
    uint16_t rx_bits;
    uint32_t tx_rx_type;
} FuriHalNfcTxRxContext;

/** Init nfc
 */
void furi_hal_nfc_init();

/** Check if nfc worker is busy
 *
 * @return     true if busy
 */
bool furi_hal_nfc_is_busy();

/** NFC field on
 */
void furi_hal_nfc_field_on();

/** NFC field off
 */
void furi_hal_nfc_field_off();

/** NFC start sleep
 */
void furi_hal_nfc_start_sleep();

/** NFC stop sleep
 */
void furi_hal_nfc_exit_sleep();

/** NFC poll
 *
 * @param      dev_list    pointer to rfalNfcDevice buffer
 * @param      dev_cnt     pointer device count
 * @param      timeout     timeout in ms
 * @param      deactivate  deactivate flag
 *
 * @return     true on success
 */
bool furi_hal_nfc_detect(
    rfalNfcDevice** dev_list,
    uint8_t* dev_cnt,
    uint32_t timeout,
    bool deactivate);

/** Activate NFC-A tag
 *
 * @param      timeout      timeout in ms
 * @param      cuid         pointer to 32bit uid
 *
 * @return     true on succeess
 */
bool furi_hal_nfc_activate_nfca(uint32_t timeout, uint32_t* cuid);

/** NFC listen
 *
 * @param      uid                 pointer to uid buffer
 * @param      uid_len             uid length
 * @param      atqa                pointer to atqa
 * @param      sak                 sak
 * @param      activate_after_sak  activate after sak flag
 * @param      timeout             timeout in ms
 *
 * @return     true on success
 */
bool furi_hal_nfc_listen(
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t* atqa,
    uint8_t sak,
    bool activate_after_sak,
    uint32_t timeout);

bool furi_hal_nfc_emulate_nfca(
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t* atqa,
    uint8_t sak,
    FuriHalNfcEmulateCallback callback,
    void* context,
    uint32_t timeout);

/** Get first command from reader after activation in emulation mode
 *
 * @param      rx_buff  pointer to receive buffer
 * @param      rx_len   receive buffer length
 *
 * @return     true on success
 */
bool furi_hal_nfc_get_first_frame(uint8_t** rx_buff, uint16_t** rx_len);

/** NFC data exchange
 *
 * @param      tx_buff     transmit buffer
 * @param      tx_len      transmit buffer length
 * @param      rx_buff     receive buffer
 * @param      rx_len      receive buffer length
 * @param      deactivate  deactivate flag
 *
 * @return     ST ReturnCode
 */
ReturnCode furi_hal_nfc_data_exchange(
    uint8_t* tx_buff,
    uint16_t tx_len,
    uint8_t** rx_buff,
    uint16_t** rx_len,
    bool deactivate);

/** NFC data exchange
 *
 * @param       tx_rx_ctx   FuriHalNfcTxRxContext instance
 *
 * @return      true on success
 */
bool furi_hal_nfc_tx_rx(FuriHalNfcTxRxContext* tx_rx_ctx);

/** NFC deactivate and start sleep
 */
void furi_hal_nfc_deactivate();

void furi_hal_nfc_stop();

#ifdef __cplusplus
}
#endif
