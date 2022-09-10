
#include "rfal_picopass.h"
#include "utils.h"

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

ReturnCode rfalPicoPassPollerInitialize(void) {
    ReturnCode ret;

    EXIT_ON_ERR(ret, rfalSetMode(RFAL_MODE_POLL_PICOPASS, RFAL_BR_26p48, RFAL_BR_26p48));
    rfalSetErrorHandling(RFAL_ERRORHANDLING_NFC);

    rfalSetGT(RFAL_GT_PICOPASS);
    rfalSetFDTListen(RFAL_FDT_LISTEN_PICOPASS_POLLER);
    rfalSetFDTPoll(RFAL_FDT_POLL_PICOPASS_POLLER);

    return ERR_NONE;
}

ReturnCode rfalPicoPassPollerCheckPresence(void) {
    ReturnCode ret;
    uint8_t txBuf[1] = {RFAL_PICOPASS_CMD_ACTALL};
    uint8_t rxBuf[32] = {0};
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = rfalConvMsTo1fc(20);

    ret = rfalTransceiveBlockingTxRx(txBuf, 1, rxBuf, 32, &recvLen, flags, fwt);
    return ret;
}

ReturnCode rfalPicoPassPollerIdentify(rfalPicoPassIdentifyRes* idRes) {
    ReturnCode ret;

    uint8_t txBuf[1] = {RFAL_PICOPASS_CMD_IDENTIFY};
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = rfalConvMsTo1fc(20);

    ret = rfalTransceiveBlockingTxRx(
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

ReturnCode rfalPicoPassPollerSelect(uint8_t* csn, rfalPicoPassSelectRes* selRes) {
    ReturnCode ret;

    rfalPicoPassSelectReq selReq;
    selReq.CMD = RFAL_PICOPASS_CMD_SELECT;
    ST_MEMCPY(selReq.CSN, csn, RFAL_PICOPASS_UID_LEN);
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = rfalConvMsTo1fc(20);

    ret = rfalTransceiveBlockingTxRx(
        (uint8_t*)&selReq,
        sizeof(rfalPicoPassSelectReq),
        (uint8_t*)selRes,
        sizeof(rfalPicoPassSelectRes),
        &recvLen,
        flags,
        fwt);
    // printf("select rx: %d %s\n", recvLen, hex2Str(selRes->CSN, RFAL_PICOPASS_UID_LEN));
    if(ret == ERR_TIMEOUT) {
        return ERR_NONE;
    }

    return ret;
}

ReturnCode rfalPicoPassPollerReadCheck(rfalPicoPassReadCheckRes* rcRes) {
    ReturnCode ret;
    uint8_t txBuf[2] = {RFAL_PICOPASS_CMD_READCHECK, 0x02};
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = rfalConvMsTo1fc(20);

    ret = rfalTransceiveBlockingTxRx(
        txBuf,
        sizeof(txBuf),
        (uint8_t*)rcRes,
        sizeof(rfalPicoPassReadCheckRes),
        &recvLen,
        flags,
        fwt);
    // printf("readcheck rx: %d %s\n", recvLen, hex2Str(rcRes->CCNR, 8));

    if(ret == ERR_CRC) {
        return ERR_NONE;
    }

    return ret;
}

ReturnCode rfalPicoPassPollerCheck(uint8_t* mac, rfalPicoPassCheckRes* chkRes) {
    ReturnCode ret;
    rfalPicoPassCheckReq chkReq;
    chkReq.CMD = RFAL_PICOPASS_CMD_CHECK;
    ST_MEMCPY(chkReq.mac, mac, 4);
    ST_MEMSET(chkReq.null, 0, 4);
    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = rfalConvMsTo1fc(20);

    // printf("check tx: %s\n", hex2Str((uint8_t *)&chkReq, sizeof(rfalPicoPassCheckReq)));
    ret = rfalTransceiveBlockingTxRx(
        (uint8_t*)&chkReq,
        sizeof(rfalPicoPassCheckReq),
        (uint8_t*)chkRes,
        sizeof(rfalPicoPassCheckRes),
        &recvLen,
        flags,
        fwt);
    // printf("check rx: %d %s\n", recvLen, hex2Str(chkRes->mac, 4));
    if(ret == ERR_CRC) {
        return ERR_NONE;
    }

    return ret;
}

ReturnCode rfalPicoPassPollerReadBlock(uint8_t blockNum, rfalPicoPassReadBlockRes* readRes) {
    ReturnCode ret;

    uint8_t txBuf[4] = {RFAL_PICOPASS_CMD_READ, 0, 0, 0};
    txBuf[1] = blockNum;
    uint16_t crc = rfalCrcCalculateCcitt(0xE012, txBuf + 1, 1);
    memcpy(txBuf + 2, &crc, sizeof(uint16_t));

    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = rfalConvMsTo1fc(20);

    ret = rfalTransceiveBlockingTxRx(
        txBuf,
        sizeof(txBuf),
        (uint8_t*)readRes,
        sizeof(rfalPicoPassReadBlockRes),
        &recvLen,
        flags,
        fwt);
    return ret;
}

ReturnCode rfalPicoPassPollerWriteBlock(uint8_t blockNum, uint8_t data[8], uint8_t mac[4]) {
    ReturnCode ret;

    uint8_t txBuf[14] = {RFAL_PICOPASS_CMD_WRITE, blockNum, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(txBuf + 2, data, RFAL_PICOPASS_MAX_BLOCK_LEN);
    memcpy(txBuf + 10, mac, 4);

    uint16_t recvLen = 0;
    uint32_t flags = RFAL_PICOPASS_TXRX_FLAGS;
    uint32_t fwt = rfalConvMsTo1fc(20);
    rfalPicoPassReadBlockRes block;

    ret = rfalTransceiveBlockingTxRx(
        txBuf, sizeof(txBuf), (uint8_t*)&block, sizeof(block), &recvLen, flags, fwt);

    if(ret == ERR_NONE) {
        // TODO: compare response
    }

    return ret;
}
