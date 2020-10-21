#include "platform.h"
#include <assert.h>

HAL_StatusTypeDef platformSpiTxRx(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len) {
    HAL_StatusTypeDef ret;
    if (txBuf && rxBuf) {
        ret = HAL_SPI_TransmitReceive(&hspi3, (uint8_t*)txBuf, rxBuf, len, HAL_MAX_DELAY);
    } else if (txBuf) {
        ret = HAL_SPI_Transmit(&hspi3, (uint8_t*)txBuf, len, HAL_MAX_DELAY);
    } else if (rxBuf) {
        ret = HAL_SPI_Receive(&hspi3, (uint8_t*)rxBuf, len, HAL_MAX_DELAY);
    }
    
    if(ret != HAL_OK) {
        exit(250);
    }
    return ret;
}


void platformProtectST25RComm()
{
    if (HAL_SPI_DeInit(&hspi3) != HAL_OK) {
        Error_Handler();
    }

    hspi3.Init.Mode = SPI_MODE_MASTER;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 7;
    hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    if (HAL_SPI_Init(&hspi3) != HAL_OK) {
        Error_Handler();
    }
}

void platformUnprotectST25RComm()
{
    
}
