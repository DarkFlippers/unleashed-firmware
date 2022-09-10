/**
 * @file furi_hal_nfc.h
 * NFC HAL API
 */

#pragma once

#include <rfal_nfc.h>
#include <st_errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <lib/nfc/protocols/nfca.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FURI_HAL_NFC_UID_MAX_LEN 10
#define FURI_HAL_NFC_DATA_BUFF_SIZE (512)
#define FURI_HAL_NFC_PARITY_BUFF_SIZE (FURI_HAL_NFC_DATA_BUFF_SIZE / 8)

#define FURI_HAL_NFC_TXRX_DEFAULT                                                    \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_AUTO | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_REMV | \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_REMV | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_AUTO)

#define FURI_HAL_NFC_TX_DEFAULT_RX_NO_CRC                                            \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_AUTO | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_REMV | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_AUTO)

#define FURI_HAL_NFC_TXRX_WITH_PAR                                                     \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_AUTO)

#define FURI_HAL_NFC_TXRX_RAW                                                          \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP | \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_KEEP | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_NONE)

#define FURI_HAL_NFC_TX_RAW_RX_DEFAULT                                                 \
    ((uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_REMV | \
     (uint32_t)RFAL_TXRX_FLAGS_PAR_RX_REMV | (uint32_t)RFAL_TXRX_FLAGS_PAR_TX_NONE)

typedef enum {
    FuriHalNfcTxRxTypeDefault,
    FuriHalNfcTxRxTypeRxNoCrc,
    FuriHalNfcTxRxTypeRxKeepPar,
    FuriHalNfcTxRxTypeRaw,
    FuriHalNfcTxRxTypeRxRaw,
    FuriHalNfcTxRxTransparent,
} FuriHalNfcTxRxType;

typedef bool (*FuriHalNfcEmulateCallback)(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len,
    uint32_t* flags,
    void* context);

typedef enum {
    FuriHalNfcTypeA,
    FuriHalNfcTypeB,
    FuriHalNfcTypeF,
    FuriHalNfcTypeV,
} FuriHalNfcType;

typedef enum {
    FuriHalNfcInterfaceRf,
    FuriHalNfcInterfaceIsoDep,
    FuriHalNfcInterfaceNfcDep,
} FuriHalNfcInterface;

typedef struct {
    FuriHalNfcType type;
    FuriHalNfcInterface interface;
    uint8_t uid_len;
    uint8_t uid[10];
    uint32_t cuid;
    uint8_t atqa[2];
    uint8_t sak;
} FuriHalNfcDevData;

typedef void (
    *FuriHalNfcTxRxSniffCallback)(uint8_t* data, uint16_t bits, bool crc_dropped, void* context);

typedef struct {
    uint8_t tx_data[FURI_HAL_NFC_DATA_BUFF_SIZE];
    uint8_t tx_parity[FURI_HAL_NFC_PARITY_BUFF_SIZE];
    uint16_t tx_bits;
    uint8_t rx_data[FURI_HAL_NFC_DATA_BUFF_SIZE];
    uint8_t rx_parity[FURI_HAL_NFC_PARITY_BUFF_SIZE];
    uint16_t rx_bits;
    FuriHalNfcTxRxType tx_rx_type;
    NfcaSignal* nfca_signal;

    FuriHalNfcTxRxSniffCallback sniff_tx;
    FuriHalNfcTxRxSniffCallback sniff_rx;
    void* sniff_context;
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

void furi_hal_nfc_stop_cmd();

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
bool furi_hal_nfc_detect(FuriHalNfcDevData* nfc_data, uint32_t timeout);

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

/** Start Target Listen mode
 * @note RFAL free implementation
 *
 * @param       nfc_data            FuriHalNfcDevData instance
 */
void furi_hal_nfc_listen_start(FuriHalNfcDevData* nfc_data);

/** Read data in Target Listen mode
 * @note Must be called only after furi_hal_nfc_listen_start()
 *
 * @param       tx_rx               FuriHalNfcTxRxContext instance
 * @param       timeout_ms          timeout im ms
 *
 * @return      true on not empty receive
 */
bool furi_hal_nfc_listen_rx(FuriHalNfcTxRxContext* tx_rx, uint32_t timeout_ms);

/** Set Target in Sleep state */
void furi_hal_nfc_listen_sleep();

/** Emulate NFC-A Target
 * @note RFAL based implementation
 *
 * @param       uid                 NFC-A UID
 * @param       uid_len             NFC-A UID length
 * @param       atqa                NFC-A ATQA
 * @param       sak                 NFC-A SAK
 * @param       callback            FuriHalNfcEmulateCallback instance
 * @param       context             pointer to context for callback
 * @param       timeout             timeout in ms
 *
 * @return      true on success
 */
bool furi_hal_nfc_emulate_nfca(
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t* atqa,
    uint8_t sak,
    FuriHalNfcEmulateCallback callback,
    void* context,
    uint32_t timeout);

/** NFC data exchange
 *
 * @param       tx_rx_ctx   FuriHalNfcTxRxContext instance
 *
 * @return      true on success
 */
bool furi_hal_nfc_tx_rx(FuriHalNfcTxRxContext* tx_rx, uint16_t timeout_ms);

/** NFC data full exhange
 *
 * @param       tx_rx_ctx   FuriHalNfcTxRxContext instance
 *
 * @return      true on success
 */
bool furi_hal_nfc_tx_rx_full(FuriHalNfcTxRxContext* tx_rx);

/** NFC deactivate and start sleep
 */
void furi_hal_nfc_sleep();

void furi_hal_nfc_stop();

#ifdef __cplusplus
}
#endif
