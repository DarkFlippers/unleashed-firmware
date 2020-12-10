/**
  ******************************************************************************
  * @file    aes.c
  * @brief   This file provides code for the configuration
  *          of the AES instances.
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
#include "aes.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CRYP_HandleTypeDef hcryp1;
__ALIGN_BEGIN static const uint32_t pKeyAES1[4] __ALIGN_END = {
                            0x00000000,0x00000000,0x00000000,0x00000000};
CRYP_HandleTypeDef hcryp2;
__ALIGN_BEGIN static const uint32_t pKeyAES2[4] __ALIGN_END = {
                            0x00000000,0x00000000,0x00000000,0x00000000};

/* AES1 init function */
void MX_AES1_Init(void)
{

  hcryp1.Instance = AES1;
  hcryp1.Init.DataType = CRYP_DATATYPE_32B;
  hcryp1.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp1.Init.pKey = (uint32_t *)pKeyAES1;
  hcryp1.Init.Algorithm = CRYP_AES_ECB;
  hcryp1.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_WORD;
  hcryp1.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
  if (HAL_CRYP_Init(&hcryp1) != HAL_OK)
  {
    Error_Handler();
  }

}
/* AES2 init function */
void MX_AES2_Init(void)
{

  hcryp2.Instance = AES2;
  hcryp2.Init.DataType = CRYP_DATATYPE_32B;
  hcryp2.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp2.Init.pKey = (uint32_t *)pKeyAES2;
  hcryp2.Init.Algorithm = CRYP_AES_ECB;
  hcryp2.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_WORD;
  hcryp2.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
  if (HAL_CRYP_Init(&hcryp2) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CRYP_MspInit(CRYP_HandleTypeDef* crypHandle)
{

  if(crypHandle->Instance==AES1)
  {
  /* USER CODE BEGIN AES1_MspInit 0 */

  /* USER CODE END AES1_MspInit 0 */
    /* AES1 clock enable */
    __HAL_RCC_AES1_CLK_ENABLE();
  /* USER CODE BEGIN AES1_MspInit 1 */

  /* USER CODE END AES1_MspInit 1 */
  }
  else if(crypHandle->Instance==AES2)
  {
  /* USER CODE BEGIN AES2_MspInit 0 */

  /* USER CODE END AES2_MspInit 0 */
    /* AES2 clock enable */
    __HAL_RCC_AES2_CLK_ENABLE();
  /* USER CODE BEGIN AES2_MspInit 1 */

  /* USER CODE END AES2_MspInit 1 */
  }
}

void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef* crypHandle)
{

  if(crypHandle->Instance==AES1)
  {
  /* USER CODE BEGIN AES1_MspDeInit 0 */

  /* USER CODE END AES1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_AES1_CLK_DISABLE();
  /* USER CODE BEGIN AES1_MspDeInit 1 */

  /* USER CODE END AES1_MspDeInit 1 */
  }
  else if(crypHandle->Instance==AES2)
  {
  /* USER CODE BEGIN AES2_MspDeInit 0 */

  /* USER CODE END AES2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_AES2_CLK_DISABLE();
  /* USER CODE BEGIN AES2_MspDeInit 1 */

  /* USER CODE END AES2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
