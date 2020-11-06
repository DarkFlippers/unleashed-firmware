/**
  ******************************************************************************
  * File Name          : SPI.c
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}
/* SPI2 init function */
void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = SPI_R_SCK_Pin|SPI_R_MISO_Pin|SPI_R_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */
  
  // SD Card need faster spi gpio
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE END SPI1_MspInit 1 */
  }
  else if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PB15     ------> SPI2_MOSI
    PD1     ------> SPI2_SCK
    */
    GPIO_InitStruct.Pin = SPI_D_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(SPI_D_MOSI_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPI_D_SCK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(SPI_D_SCK_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, SPI_R_SCK_Pin|SPI_R_MISO_Pin|SPI_R_MOSI_Pin);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
  else if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PB15     ------> SPI2_MOSI
    PD1     ------> SPI2_SCK
    */
    HAL_GPIO_DeInit(SPI_D_MOSI_GPIO_Port, SPI_D_MOSI_Pin);

    HAL_GPIO_DeInit(SPI_D_SCK_GPIO_Port, SPI_D_SCK_Pin);

  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void NFC_SPI_Reconfigure() {
  if (HAL_SPI_DeInit(&SPI_R) != HAL_OK) {
      Error_Handler();
  }

  SPI_R.Init.Mode = SPI_MODE_MASTER;
  SPI_R.Init.Direction = SPI_DIRECTION_2LINES;
  SPI_R.Init.DataSize = SPI_DATASIZE_8BIT;
  SPI_R.Init.CLKPolarity = SPI_POLARITY_LOW;
  SPI_R.Init.CLKPhase = SPI_PHASE_2EDGE;
  SPI_R.Init.NSS = SPI_NSS_SOFT;
  SPI_R.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; // 8mhz, 10mhz is max
  SPI_R.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SPI_R.Init.TIMode = SPI_TIMODE_DISABLE;
  SPI_R.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SPI_R.Init.CRCPolynomial = 7;
  SPI_R.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  SPI_R.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  if (HAL_SPI_Init(&SPI_R) != HAL_OK) {
      Error_Handler();
  }
}

void SD_SPI_Reconfigure_Slow(void) {
  if (HAL_SPI_DeInit(&SPI_SD_HANDLE) != HAL_OK) {
      Error_Handler();
  }

  SPI_SD_HANDLE.Init.Mode = SPI_MODE_MASTER;
  SPI_SD_HANDLE.Init.Direction = SPI_DIRECTION_2LINES;
  SPI_SD_HANDLE.Init.DataSize = SPI_DATASIZE_8BIT;
  SPI_SD_HANDLE.Init.CLKPolarity = SPI_POLARITY_LOW;
  SPI_SD_HANDLE.Init.CLKPhase = SPI_PHASE_1EDGE;
  SPI_SD_HANDLE.Init.NSS = SPI_NSS_SOFT;
  SPI_SD_HANDLE.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  SPI_SD_HANDLE.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SPI_SD_HANDLE.Init.TIMode = SPI_TIMODE_DISABLE;
  SPI_SD_HANDLE.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SPI_SD_HANDLE.Init.CRCPolynomial = 7;
  SPI_SD_HANDLE.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  SPI_SD_HANDLE.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;

  if(HAL_SPI_Init(&SPI_SD_HANDLE) != HAL_OK) {
      Error_Handler();
  }
}

void SD_SPI_Reconfigure_Fast(void) {
  if(HAL_SPI_DeInit(&SPI_SD_HANDLE) != HAL_OK) {
        Error_Handler();
  }

  SPI_SD_HANDLE.Init.Mode = SPI_MODE_MASTER;
  SPI_SD_HANDLE.Init.Direction = SPI_DIRECTION_2LINES;
  SPI_SD_HANDLE.Init.DataSize = SPI_DATASIZE_8BIT;
  SPI_SD_HANDLE.Init.CLKPolarity = SPI_POLARITY_LOW;
  SPI_SD_HANDLE.Init.CLKPhase = SPI_PHASE_1EDGE;
  SPI_SD_HANDLE.Init.NSS = SPI_NSS_SOFT;
  SPI_SD_HANDLE.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  SPI_SD_HANDLE.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SPI_SD_HANDLE.Init.TIMode = SPI_TIMODE_DISABLE;
  SPI_SD_HANDLE.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SPI_SD_HANDLE.Init.CRCPolynomial = 7;
  SPI_SD_HANDLE.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  SPI_SD_HANDLE.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;

  if(HAL_SPI_Init(&SPI_SD_HANDLE) != HAL_OK) {
      Error_Handler();
  }
}

void CC1101_SPI_Reconfigure(void) {
  if(HAL_SPI_DeInit(&SPI_R) != HAL_OK) {
      Error_Handler();
  }

  SPI_R.Init.Mode = SPI_MODE_MASTER;
  SPI_R.Init.Direction = SPI_DIRECTION_2LINES;
  SPI_R.Init.DataSize = SPI_DATASIZE_8BIT;
  SPI_R.Init.CLKPolarity = SPI_POLARITY_LOW;
  SPI_R.Init.CLKPhase = SPI_PHASE_1EDGE;
  SPI_R.Init.NSS = SPI_NSS_SOFT;
  SPI_R.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  SPI_R.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SPI_R.Init.TIMode = SPI_TIMODE_DISABLE;
  SPI_R.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SPI_R.Init.CRCPolynomial = 7;
  SPI_R.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  SPI_R.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  if(HAL_SPI_Init(&SPI_R) != HAL_OK) {
      Error_Handler();
  }
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
