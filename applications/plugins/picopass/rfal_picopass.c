#include "rfal_picopass.h"
#include "utils.h"

#define RFAL_PICOPASS_TXRX_FLAGS                                                    \
    (FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_TX_MANUAL | FURI_HAL_NFC_LL_TXRX_FLAGS_AGC_ON | \
     FURI_HAL_NFC_LL_TXRX_FLAGS_PAR_RX_REMV | FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_RX_KEEP)

#define TAG "RFAL_PICOPASS"

typedef struct {
    uint8_t CMD;
    uint8_t CSN[RFAL_PICOPASS_UID_LEN];
} rfalPicoPassSelectReq;

typedef struct {
    uint8_t CMD;
    uint8_t null[4];
    uint8_t mac[4];
} rfalPicoPassCheckReq;

static uint16_t rfalPicoPassUpdateCcitt(uint16_t crcSeed, uint8_t dataByte) {
    uint16_t crc = crcSeed;
    uint8_t dat = dataByte;

    dat ^= (uint8_t)(crc & 0xFFU);
    dat ^= (dat << 4);

    crc = (crc >> 8) ^ (((uint16_t)dat) << 8) ^ (((uint16_t)dat) << 3) ^ (((uint16_t)dat) >> 4);

    return crc;
}

static uint16_t
    rfalPicoPassCalculateCcitt(uint16_t preloadValue, const uint8_t* buf, uint16_t length) {
    uint16_t crc = preloadValue;
    uint16_t index;

    for(index = 0; index < length; index++) {
        crc = rfalPicoPassUpdateCcitt(crc, buf[index]);
    }

    return crc;
}

FuriHalNfcReturn rfalPicoPassPollerInitialize(void) {
    FuriHalNfcReturn ret;

    ret = furi_hal_nfc_ll_set_mode(
        FuriHalNfcModePollPicopass, FuriHalNfcBitrate26p48, FuriHalNfcBitrate26p48);
    if(ret != FuriHalNfcReturnOk) {
        return ret;
    };

    furi_hal_nfc_ll_set_error_handling(FuriHalNfcErrorHandlingNfc);
    furi_hal_nfc_ll_set_guard_time(FURI_HAL_NFC_LL_GT_PICOPASS);
    furi_hal_nfc_ll_set_fdt_listen(FURI_HAL_NFC_LL_FDT_LISTEN_PICOPASS_POLLER);
    furi_hal_nfc_ll_set_fdt_poll(FURI_HAL_NFC_LL_FDT_POLL_PICOPASS_POLLER);

    return FuriHalNfcReturnOk;
}

FuriHalNfcReturn rfalPicoPassPollerCheckPresence(void) {
    FuriHalNfcReturn ret;
    uint8_t txBuf[1] = {RFAL_PICOPASS_CMD_ACTALL};
    uint8_t rxBuf[32] = {0};
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = furi_hal_nfc_ll_ms2fc(20);

    ret = furi_hal_nfc_ll_txrx(txBuf, 1, rxBuf, 32, &recvLen, flags, fwt);
    return ret;
}

FuriHalNfcReturn rfalPicoPassPollerIdentify(rfalPicoPassIdentifyRes* idRes) {
    FuriHalNfcReturn ret;

    uint8_t txBuf[1] = {RFAL_PICOPASS_CMD_IDENTIFY};
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = furi_hal_nfc_ll_ms2fc(20);

    ret = furi_hal_nfc_ll_txrx(
        txBuf,
        sizeof(txBuf),
        (uint8_t*)idRes,
        sizeof(rfalPicoPassIdentifyRes),
        &recvLen,
        flags,
        fwt);
    // printf("identify rx: %d %s\n", recvLen, hex2Str(idRes->CSN, RFAL_PICOPASS_UID_LEN));

    return ret;
}

FuriHalNfcReturn rfalPicoPassPollerSelect(uint8_t* csn, rfalPicoPassSelectRes* selRes) {
    FuriHalNfcReturn ret;

    rfalPicoPassSelectReq selReq;
    selReq.CMD = RFAL_PICOPASS_CMD_SELECT;
    ST_MEMCPY(selReq.CSN, csn, RFAL_PICOPASS_UID_LEN);
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = furi_hal_nfc_ll_ms2fc(20);

    ret = furi_hal_nfc_ll_txrx(
        (uint8_t*)&selReq,
        sizeof(rfalPicoPassSelectReq),
        (uint8_t*)selRes,
        sizeof(rfalPicoPassSelectRes),
        &recvLen,
        flags,
        fwt);
    // printf("select rx: %d %s\n", recvLen, hex2Str(selRes->CSN, RFAL_PICOPASS_UID_LEN));
    if(ret == FuriHalNfcReturnTimeout) {
        return FuriHalNfcReturnOk;
    }

    return ret;
}

FuriHalNfcReturn rfalPicoPassPollerReadCheck(rfalPicoPassReadCheckRes* rcRes) {
    FuriHalNfcReturn ret;
    uint8_t txBuf[2] = {RFAL_PICOPASS_CMD_READCHECK, 0x02};
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = furi_hal_nfc_ll_ms2fc(20);

    ret = furi_hal_nfc_ll_txrx(
        txBuf,
        sizeof(txBuf),
        (uint8_t*)rcRes,
        sizeof(rfalPicoPassReadCheckRes),
        &recvLen,
        flags,
        fwt);
    // printf("readcheck rx: %d %s\n", recvLen, hex2Str(rcRes->CCNR, 8));

    if(ret == FuriHalNfcReturnCrc) {
        return FuriHalNfcReturnOk;
    }

    return ret;
}

FuriHalNfcReturn rfalPicoPassPollerCheck(uint8_t* mac, rfalPicoPassCheckRes* chkRes) {
    FuriHalNfcReturn ret;
    rfalPicoPassCheckReq chkReq;
    chkReq.CMD = RFAL_PICOPASS_CMD_CHECK;
    ST_MEMCPY(chkReq.mac, mac, 4);
    ST_MEMSET(chkReq.null, 0, 4);
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = furi_hal_nfc_ll_ms2fc(20);

    // printf("check tx: %s\n", hex2Str((uint8_t *)&chkReq, sizeof(rfalPicoPassCheckReq)));
    ret = furi_hal_nfc_ll_txrx(
        (uint8_t*)&chkReq,
        sizeof(rfalPicoPassCheckReq),
        (uint8_t*)chkRes,
        sizeof(rfalPicoPassCheckRes),
        &recvLen,
        flags,
        fwt);
    // printf("check rx: %d %s\n", recvLen, hex2Str(chkRes->mac, 4));
    if(ret == FuriHalNfcReturnCrc) {
        return FuriHalNfcReturnOk;
    }

    return ret;
}

FuriHalNfcReturn rfalPicoPassPollerReadBlock(uint8_t blockNum, rfalPicoPassReadBlockRes* readRes) {
    FuriHalNfcReturn ret;

    uint8_t txBuf[4] = {RFAL_PICOPASS_CMD_READ, 0, 0, 0};
    txBuf[1] = blockNum;
    uint16_t crc = rfalPicoPassCalculateCcitt(0xE012, txBuf + 1, 1);
    memcpy(txBuf + 2, &crc, sizeof(uint16_t));

    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = furi_hal_nfc_ll_ms2fc(20);

    ret = furi_hal_nfc_ll_txrx(
        txBuf,
        sizeof(txBuf),
        (uint8_t*)readRes,
        sizeof(rfalPicoPassReadBlockRes),
        &recvLen,
        flags,
        fwt);
    return ret;
}

FuriHalNfcReturn rfalPicoPassPollerWriteBlock(uint8_t blockNum, uint8_t data[8], uint8_t mac[4]) {
    FuriHalNfcReturn ret;

    uint8_t txBuf[14] = {RFAL_PICOPASS_CMD_WRITE, blockNum, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(txBuf + 2, data, RFAL_PICOPASS_MAX_BLOCK_LEN);
    memcpy(txBuf + 10, mac, 4);

    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = furi_hal_nfc_ll_ms2fc(20);
    rfalPicoPassReadBlockRes block;

    ret = furi_hal_nfc_ll_txrx(
        txBuf, sizeof(txBuf), (uint8_t*)&block, sizeof(block), &recvLen, flags, fwt);

    if(ret == FuriHalNfcReturnOk) {
        // TODO: compare response
    }

    return ret;
}
