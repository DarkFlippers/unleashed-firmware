
#include "rfal_picopass.h"
#include "utils.h"

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
    /*
 * ./reveng -w 16 -s 0c07cc47 0c064556 0c083bbf 0c09b2ae       
 width=16  poly=0x1021  init=0xd924  refin=true  refout=true  xorout=0x0000  check=0x1329  residue=0x0000  name=(none)
0c  06  45  56
0c  07  cc  47
0c  08  3b  bf
0c  09  b2  ae
 */

    uint8_t readCmds[4][4] = {
        {RFAL_PICOPASS_CMD_READ, 6, 0x45, 0x56},
        {RFAL_PICOPASS_CMD_READ, 7, 0xcc, 0x47},
        {RFAL_PICOPASS_CMD_READ, 8, 0x3b, 0xbf},
        {RFAL_PICOPASS_CMD_READ, 9, 0xb2, 0xae}};

    uint8_t* txBuf = readCmds[blockNum - 6];
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
    // printf("check rx: %d %s\n", recvLen, hex2Str(readRes->data, RFAL_PICOPASS_MAX_BLOCK_LEN));

    return ret;
}
