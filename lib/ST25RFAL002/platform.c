#include "platform.h"
#include <assert.h>
#include <main.h>
#include <spi.h>

HAL_StatusTypeDef platformSpiTxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len) {
    HAL_StatusTypeDef ret;
    if (txBuf && rxBuf) {
        ret = HAL_SPI_TransmitReceive(&SPI_R, (uint8_t*)txBuf, rxBuf, len, HAL_MAX_DELAY);
    } else if (txBuf) {
        ret = HAL_SPI_Transmit(&SPI_R, (uint8_t*)txBuf, len, HAL_MAX_DELAY);
    } else if (rxBuf) {
        ret = HAL_SPI_Receive(&SPI_R, (uint8_t*)rxBuf, len, HAL_MAX_DELAY);
    }
    
    if(ret != HAL_OK) {
        exit(250);
    }
    return ret;
}


void platformProtectST25RComm()
{
    NFC_SPI_Reconfigure();
}

void platformUnprotectST25RComm()
{
    
}
