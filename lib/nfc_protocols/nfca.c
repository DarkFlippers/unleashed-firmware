#include "nfca.h"
#include <string.h>
#include <stdio.h>

#define NFCA_CMD_RATS (0xE0U)

#define NFCA_CRC_INIT (0x6363)

typedef struct {
    uint8_t cmd;
    uint8_t param;
} nfca_cmd_rats;

static uint8_t nfca_default_ats[] = {0x05, 0x78, 0x80, 0x80, 0x00};

static uint8_t nfca_sleep_req[] = {0x50, 0x00};

uint16_t nfca_get_crc16(uint8_t* buff, uint16_t len) {
    uint16_t crc = NFCA_CRC_INIT;
    uint8_t byte = 0;

    for(uint8_t i = 0; i < len; i++) {
        byte = buff[i];
        byte ^= (uint8_t)(crc & 0xff);
        byte ^= byte << 4;
        crc = (crc >> 8) ^ (((uint16_t)byte) << 8) ^ (((uint16_t)byte) << 3) ^
              (((uint16_t)byte) >> 4);
    }

    return crc;
}

void nfca_append_crc16(uint8_t* buff, uint16_t len) {
    uint16_t crc = nfca_get_crc16(buff, len);
    buff[len] = (uint8_t)crc;
    buff[len + 1] = (uint8_t)(crc >> 8);
}

bool nfca_emulation_handler(
    uint8_t* buff_rx,
    uint16_t buff_rx_len,
    uint8_t* buff_tx,
    uint16_t* buff_tx_len) {
    bool sleep = false;
    uint8_t rx_bytes = buff_rx_len / 8;

    if(rx_bytes == sizeof(nfca_sleep_req) && !memcmp(buff_rx, nfca_sleep_req, rx_bytes)) {
        sleep = true;
    } else if(rx_bytes == sizeof(nfca_cmd_rats) && buff_rx[0] == NFCA_CMD_RATS) {
        memcpy(buff_tx, nfca_default_ats, sizeof(nfca_default_ats));
        *buff_tx_len = sizeof(nfca_default_ats) * 8;
    }

    return sleep;
}