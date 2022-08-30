
#ifndef RFAL_PICOPASS_H
#define RFAL_PICOPASS_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "platform.h"
#include "rfal_rf.h"
#include "rfal_crc.h"
#include "st_errno.h"

#define RFAL_PICOPASS_UID_LEN 8
#define RFAL_PICOPASS_MAX_BLOCK_LEN 8

#define RFAL_PICOPASS_TXRX_FLAGS                                                            \
    (RFAL_TXRX_FLAGS_CRC_TX_MANUAL | RFAL_TXRX_FLAGS_AGC_ON | RFAL_TXRX_FLAGS_PAR_RX_REMV | \
     RFAL_TXRX_FLAGS_CRC_RX_KEEP)

enum {
    RFAL_PICOPASS_CMD_ACTALL = 0x0A,
    RFAL_PICOPASS_CMD_IDENTIFY = 0x0C,
    RFAL_PICOPASS_CMD_SELECT = 0x81,
    RFAL_PICOPASS_CMD_READCHECK = 0x88,
    RFAL_PICOPASS_CMD_CHECK = 0x05,
    RFAL_PICOPASS_CMD_READ = 0x0C,
    RFAL_PICOPASS_CMD_WRITE = 0x87,
};

typedef struct {
    uint8_t CSN[RFAL_PICOPASS_UID_LEN]; // Anti-collision CSN
    uint8_t crc[2];
} rfalPicoPassIdentifyRes;

typedef struct {
    uint8_t CSN[RFAL_PICOPASS_UID_LEN]; // Real CSN
    uint8_t crc[2];
} rfalPicoPassSelectRes;

typedef struct {
    uint8_t CCNR[8];
} rfalPicoPassReadCheckRes;

typedef struct {
    uint8_t mac[4];
} rfalPicoPassCheckRes;

typedef struct {
    uint8_t data[RFAL_PICOPASS_MAX_BLOCK_LEN];
    uint8_t crc[2];
} rfalPicoPassReadBlockRes;

ReturnCode rfalPicoPassPollerInitialize(void);
ReturnCode rfalPicoPassPollerCheckPresence(void);
ReturnCode rfalPicoPassPollerIdentify(rfalPicoPassIdentifyRes* idRes);
ReturnCode rfalPicoPassPollerSelect(uint8_t* csn, rfalPicoPassSelectRes* selRes);
ReturnCode rfalPicoPassPollerReadCheck(rfalPicoPassReadCheckRes* rcRes);
ReturnCode rfalPicoPassPollerCheck(uint8_t* mac, rfalPicoPassCheckRes* chkRes);
ReturnCode rfalPicoPassPollerReadBlock(uint8_t blockNum, rfalPicoPassReadBlockRes* readRes);
ReturnCode rfalPicoPassPollerWriteBlock(uint8_t blockNum, uint8_t data[8], uint8_t mac[4]);

#endif /* RFAL_PICOPASS_H */
