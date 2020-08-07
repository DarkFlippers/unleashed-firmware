/**
  ******************************************************************************
  * @file    stm32l4xx_hal_nand.h
  * @author  MCD Application Team
  * @brief   Header file of NAND HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L4xx_HAL_NAND_H
#define __STM32L4xx_HAL_NAND_H

#ifdef __cplusplus
 extern "C" {
#endif

#if defined(FMC_BANK3)

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_fmc.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup NAND
  * @{
  */

/* Exported typedef ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/** @defgroup NAND_Exported_Types NAND Exported Types
  * @{
  */

/**
  * @brief  HAL NAND State structures definition
  */
typedef enum
{
  HAL_NAND_STATE_RESET     = 0x00U,  /*!< NAND not yet initialized or disabled */
  HAL_NAND_STATE_READY     = 0x01U,  /*!< NAND initialized and ready for use   */
  HAL_NAND_STATE_BUSY      = 0x02U,  /*!< NAND internal process is ongoing     */
  HAL_NAND_STATE_ERROR     = 0x03U   /*!< NAND error state                     */
}HAL_NAND_StateTypeDef;

/**
  * @brief  NAND Memory electronic signature Structure definition
  */
typedef struct
{
  /*<! NAND memory electronic signature maker and device IDs */
  uint8_t Maker_Id;

  uint8_t Device_Id;

  uint8_t Third_Id;

  uint8_t Fourth_Id;
}NAND_IDTypeDef;

/**
  * @brief  NAND Memory address Structure definition
  */
typedef struct
{
  uint16_t Page;   /*!< NAND memory Page address  */

  uint16_t Plane;  /*!< NAND memory Zone address  */

  uint16_t Block;  /*!< NAND memory Block address */
}NAND_AddressTypeDef;

/**
  * @brief  NAND Memory info Structure definition
  */
typedef struct
{
  uint32_t        PageSize;              /*!< NAND memory page (without spare area) size measured in bytes
                                              for 8 bits adressing or words for 16 bits addressing             */

  uint32_t        SpareAreaSize;         /*!< NAND memory spare area size measured in bytes
                                              for 8 bits adressing or words for 16 bits addressing             */

  uint32_t        BlockSize;             /*!< NAND memory block size measured in number of pages               */

  uint32_t        BlockNbr;              /*!< NAND memory number of total blocks                               */

  uint32_t        PlaneNbr;              /*!< NAND memory number of planes                                     */

  uint32_t        PlaneSize;             /*!< NAND memory zone size measured in number of blocks               */

  FunctionalState ExtraCommandEnable;    /*!< NAND extra command needed for Page reading mode. This
                                              parameter is mandatory for some NAND parts after the read
                                              command (NAND_CMD_AREA_TRUE1) and before DATA reading sequence.
                                              Example: Toshiba THTH58BYG3S0HBAI6.
                                              This parameter could be ENABLE or DISABLE
                                              Please check the Read Mode sequnece in the NAND device datasheet */
} NAND_DeviceConfigTypeDef;

/**
  * @brief  NAND handle Structure definition
  */
typedef struct
{
  FMC_NAND_TypeDef             *Instance;  /*!< Register base address                        */

  FMC_NAND_InitTypeDef         Init;       /*!< NAND device control configuration parameters */

  HAL_LockTypeDef              Lock;       /*!< NAND locking object                          */

  __IO HAL_NAND_StateTypeDef   State;      /*!< NAND device access state                     */

  NAND_DeviceConfigTypeDef     Config;     /*!< NAND phusical characteristic information structure    */

} NAND_HandleTypeDef;
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/** @defgroup NAND_Exported_Macros NAND Exported Macros
 * @{
 */

/** @brief Reset NAND handle state
  * @param  __HANDLE__ specifies the NAND handle.
  * @retval None
  */
#define __HAL_NAND_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_NAND_STATE_RESET)

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup NAND_Exported_Functions NAND Exported Functions
  * @{
  */

/** @addtogroup NAND_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */

/* Initialization/de-initialization functions  ********************************/
HAL_StatusTypeDef  HAL_NAND_Init(NAND_HandleTypeDef *hnand, FMC_NAND_PCC_TimingTypeDef *ComSpace_Timing, FMC_NAND_PCC_TimingTypeDef *AttSpace_Timing);
HAL_StatusTypeDef  HAL_NAND_DeInit(NAND_HandleTypeDef *hnand);

HAL_StatusTypeDef  HAL_NAND_ConfigDevice(NAND_HandleTypeDef *hnand, NAND_DeviceConfigTypeDef *pDeviceConfig);

HAL_StatusTypeDef  HAL_NAND_Read_ID(NAND_HandleTypeDef *hnand, NAND_IDTypeDef *pNAND_ID);

void               HAL_NAND_MspInit(NAND_HandleTypeDef *hnand);
void               HAL_NAND_MspDeInit(NAND_HandleTypeDef *hnand);
void               HAL_NAND_IRQHandler(NAND_HandleTypeDef *hnand);
void               HAL_NAND_ITCallback(NAND_HandleTypeDef *hnand);

/**
  * @}
  */

/** @addtogroup NAND_Exported_Functions_Group2 Input and Output functions
  * @{
  */

/* IO operation functions  ****************************************************/

HAL_StatusTypeDef  HAL_NAND_Reset(NAND_HandleTypeDef *hnand);

HAL_StatusTypeDef  HAL_NAND_Read_Page_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumPageToRead);
HAL_StatusTypeDef  HAL_NAND_Write_Page_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumPageToWrite);
HAL_StatusTypeDef  HAL_NAND_Read_SpareArea_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumSpareAreaToRead);
HAL_StatusTypeDef  HAL_NAND_Write_SpareArea_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t NumSpareAreaTowrite);

HAL_StatusTypeDef  HAL_NAND_Read_Page_16b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint16_t *pBuffer, uint32_t NumPageToRead);
HAL_StatusTypeDef  HAL_NAND_Write_Page_16b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint16_t *pBuffer, uint32_t NumPageToWrite);
HAL_StatusTypeDef  HAL_NAND_Read_SpareArea_16b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint16_t *pBuffer, uint32_t NumSpareAreaToRead);
HAL_StatusTypeDef  HAL_NAND_Write_SpareArea_16b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint16_t *pBuffer, uint32_t NumSpareAreaTowrite);

HAL_StatusTypeDef  HAL_NAND_Erase_Block(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress);

uint32_t           HAL_NAND_Address_Inc(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress);

/**
  * @}
  */

/** @addtogroup NAND_Exported_Functions_Group3 Peripheral Control functions
  * @{
  */

/* NAND Control functions  ****************************************************/
HAL_StatusTypeDef  HAL_NAND_ECC_Enable(NAND_HandleTypeDef *hnand);
HAL_StatusTypeDef  HAL_NAND_ECC_Disable(NAND_HandleTypeDef *hnand);
HAL_StatusTypeDef  HAL_NAND_GetECC(NAND_HandleTypeDef *hnand, uint32_t *ECCval, uint32_t Timeout);

/**
  * @}
  */

/** @addtogroup NAND_Exported_Functions_Group4 Peripheral State functions
  * @{
  */
/* NAND State functions *******************************************************/
HAL_NAND_StateTypeDef HAL_NAND_GetState(NAND_HandleTypeDef *hnand);
uint32_t              HAL_NAND_Read_Status(NAND_HandleTypeDef *hnand);
/**
  * @}
  */

/**
  * @}
  */
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup NAND_Private_Constants NAND Private Constants
  * @{
  */
#define NAND_DEVICE                FMC_BANK3
#define NAND_WRITE_TIMEOUT         ((uint32_t)0x01000000U)

#define CMD_AREA                   ((uint32_t)(1<<16))  /* A16 = CLE high */
#define ADDR_AREA                  ((uint32_t)(1<<17))  /* A17 = ALE high */

#define NAND_CMD_AREA_A            ((uint8_t)0x00U)
#define NAND_CMD_AREA_B            ((uint8_t)0x01U)
#define NAND_CMD_AREA_C            ((uint8_t)0x50U)
#define NAND_CMD_AREA_TRUE1        ((uint8_t)0x30U)

#define NAND_CMD_WRITE0            ((uint8_t)0x80U)
#define NAND_CMD_WRITE_TRUE1       ((uint8_t)0x10U)
#define NAND_CMD_ERASE0            ((uint8_t)0x60U)
#define NAND_CMD_ERASE1            ((uint8_t)0xD0U)
#define NAND_CMD_READID            ((uint8_t)0x90U)
#define NAND_CMD_STATUS            ((uint8_t)0x70U)
#define NAND_CMD_LOCK_STATUS       ((uint8_t)0x7AU)
#define NAND_CMD_RESET             ((uint8_t)0xFFU)

/* NAND memory status */
#define NAND_VALID_ADDRESS         ((uint32_t)0x00000100U)
#define NAND_INVALID_ADDRESS       ((uint32_t)0x00000200U)
#define NAND_TIMEOUT_ERROR         ((uint32_t)0x00000400U)
#define NAND_BUSY                  ((uint32_t)0x00000000U)
#define NAND_ERROR                 ((uint32_t)0x00000001U)
#define NAND_READY                 ((uint32_t)0x00000040U)
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup NAND_Private_Macros NAND Private Macros
  * @{
  */

/**
  * @brief  NAND memory address computation.
  * @param  __ADDRESS__ NAND memory address.
  * @param  __HANDLE__  NAND handle.
  * @retval NAND Raw address value
  */
#define ARRAY_ADDRESS(__ADDRESS__ , __HANDLE__) (((__ADDRESS__)->Page) + \
                         (((__ADDRESS__)->Block + (((__ADDRESS__)->Plane) * ((__HANDLE__)->Config.PlaneSize)))* ((__HANDLE__)->Config.BlockSize)))

#define COLUMN_ADDRESS( __HANDLE__) ((__HANDLE__)->Config.PageSize)

/**
  * @brief  NAND memory address cycling.
  * @param  __ADDRESS__ NAND memory address.
  * @retval NAND address cycling value.
  */
#define ADDR_1ST_CYCLE(__ADDRESS__)       (uint8_t)(__ADDRESS__)              /* 1st addressing cycle */
#define ADDR_2ND_CYCLE(__ADDRESS__)       (uint8_t)((__ADDRESS__) >> 8)       /* 2nd addressing cycle */
#define ADDR_3RD_CYCLE(__ADDRESS__)       (uint8_t)((__ADDRESS__) >> 16)      /* 3rd addressing cycle */
#define ADDR_4TH_CYCLE(__ADDRESS__)       (uint8_t)((__ADDRESS__) >> 24)      /* 4th addressing cycle */

/**
  * @brief  NAND memory Columns cycling.
  * @param  __ADDRESS__ NAND memory address.
  * @retval NAND Column address cycling value.
  */
#define COLUMN_1ST_CYCLE(__ADDRESS__)     (uint8_t)(__ADDRESS__)              /* 1st Column addressing cycle */
#define COLUMN_2ND_CYCLE(__ADDRESS__)     (uint8_t)((__ADDRESS__) >> 8)       /* 2nd Column addressing cycle */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* FMC_BANK3 */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_HAL_NAND_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
