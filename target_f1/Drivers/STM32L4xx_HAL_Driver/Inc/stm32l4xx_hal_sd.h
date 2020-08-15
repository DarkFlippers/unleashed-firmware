/**
  ******************************************************************************
  * @file    stm32l4xx_hal_sd.h
  * @author  MCD Application Team
  * @brief   Header file of SD HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32L4xx_HAL_SD_H
#define STM32L4xx_HAL_SD_H

#ifdef __cplusplus
 extern "C" {
#endif

#if defined(SDMMC1)

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_sdmmc.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup SD SD
  * @brief SD HAL module driver
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup SD_Exported_Types SD Exported Types
  * @{
  */

/** @defgroup SD_Exported_Types_Group1 SD State enumeration structure
  * @{
  */
typedef enum
{
  HAL_SD_STATE_RESET                  = ((uint32_t)0x00000000U),  /*!< SD not yet initialized or disabled  */
  HAL_SD_STATE_READY                  = ((uint32_t)0x00000001U),  /*!< SD initialized and ready for use    */
  HAL_SD_STATE_TIMEOUT                = ((uint32_t)0x00000002U),  /*!< SD Timeout state                    */
  HAL_SD_STATE_BUSY                   = ((uint32_t)0x00000003U),  /*!< SD process ongoing                  */
  HAL_SD_STATE_PROGRAMMING            = ((uint32_t)0x00000004U),  /*!< SD Programming State                */
  HAL_SD_STATE_RECEIVING              = ((uint32_t)0x00000005U),  /*!< SD Receiving State                  */
  HAL_SD_STATE_TRANSFER               = ((uint32_t)0x00000006U),  /*!< SD Transfert State                  */
  HAL_SD_STATE_ERROR                  = ((uint32_t)0x0000000FU)   /*!< SD is in error state                */
}HAL_SD_StateTypeDef;
/**
  * @}
  */

/** @defgroup SD_Exported_Types_Group2 SD Card State enumeration structure
  * @{
  */
typedef uint32_t HAL_SD_CardStateTypeDef;

#define HAL_SD_CARD_READY          0x00000001U  /*!< Card state is ready                     */
#define HAL_SD_CARD_IDENTIFICATION 0x00000002U  /*!< Card is in identification state         */
#define HAL_SD_CARD_STANDBY        0x00000003U  /*!< Card is in standby state                */
#define HAL_SD_CARD_TRANSFER       0x00000004U  /*!< Card is in transfer state               */
#define HAL_SD_CARD_SENDING        0x00000005U  /*!< Card is sending an operation            */
#define HAL_SD_CARD_RECEIVING      0x00000006U  /*!< Card is receiving operation information */
#define HAL_SD_CARD_PROGRAMMING    0x00000007U  /*!< Card is in programming state            */
#define HAL_SD_CARD_DISCONNECTED   0x00000008U  /*!< Card is disconnected                    */
#define HAL_SD_CARD_ERROR          0x000000FFU  /*!< Card response Error                     */
/**
  * @}
  */

/** @defgroup SD_Exported_Types_Group3 SD Handle Structure definition
  * @{
  */
#define SD_InitTypeDef      SDMMC_InitTypeDef
#define SD_TypeDef          SDMMC_TypeDef

/**
  * @brief  SD Card Information Structure definition
  */
typedef struct
{
  uint32_t CardType;                     /*!< Specifies the card Type                         */

  uint32_t CardVersion;                  /*!< Specifies the card version                      */

  uint32_t Class;                        /*!< Specifies the class of the card class           */

  uint32_t RelCardAdd;                   /*!< Specifies the Relative Card Address             */

  uint32_t BlockNbr;                     /*!< Specifies the Card Capacity in blocks           */

  uint32_t BlockSize;                    /*!< Specifies one block size in bytes               */

  uint32_t LogBlockNbr;                  /*!< Specifies the Card logical Capacity in blocks   */

  uint32_t LogBlockSize;                 /*!< Specifies logical block size in bytes           */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)

  uint32_t CardSpeed;                    /*!< Specifies the card Speed                        */

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
}HAL_SD_CardInfoTypeDef;

/**
  * @brief  SD handle Structure definition
  */
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
typedef struct __SD_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
{
  SD_TypeDef                   *Instance;        /*!< SD registers base address           */

  SD_InitTypeDef               Init;             /*!< SD required parameters              */

  HAL_LockTypeDef              Lock;             /*!< SD locking object                   */

  uint8_t                      *pTxBuffPtr;      /*!< Pointer to SD Tx transfer Buffer    */

  uint32_t                     TxXferSize;       /*!< SD Tx Transfer size                 */

  uint8_t                      *pRxBuffPtr;      /*!< Pointer to SD Rx transfer Buffer    */

  uint32_t                     RxXferSize;       /*!< SD Rx Transfer size                 */

  __IO uint32_t                Context;          /*!< SD transfer context                 */

  __IO HAL_SD_StateTypeDef     State;            /*!< SD card State                       */

  __IO uint32_t                ErrorCode;        /*!< SD Card Error codes                 */

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)

  DMA_HandleTypeDef            *hdmatx;          /*!< SD Tx DMA handle parameters         */

  DMA_HandleTypeDef            *hdmarx;          /*!< SD Rx DMA handle parameters         */

#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */
  HAL_SD_CardInfoTypeDef       SdCard;           /*!< SD Card information                 */

  uint32_t                     CSD[4];           /*!< SD card specific data table         */

  uint32_t                     CID[4];           /*!< SD card identification number table */

#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
  void (* TxCpltCallback)                 (struct __SD_HandleTypeDef *hsd);
  void (* RxCpltCallback)                 (struct __SD_HandleTypeDef *hsd);
  void (* ErrorCallback)                  (struct __SD_HandleTypeDef *hsd);
  void (* AbortCpltCallback)              (struct __SD_HandleTypeDef *hsd);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  void (* Read_DMADblBuf0CpltCallback)    (struct __SD_HandleTypeDef *hsd);
  void (* Read_DMADblBuf1CpltCallback)    (struct __SD_HandleTypeDef *hsd);
  void (* Write_DMADblBuf0CpltCallback)   (struct __SD_HandleTypeDef *hsd);
  void (* Write_DMADblBuf1CpltCallback)   (struct __SD_HandleTypeDef *hsd);

  void (* DriveTransceiver_1_8V_Callback) (FlagStatus status);
#endif

  void (* MspInitCallback)                (struct __SD_HandleTypeDef *hsd);
  void (* MspDeInitCallback)              (struct __SD_HandleTypeDef *hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
}SD_HandleTypeDef;

/**
  * @}
  */

/** @defgroup SD_Exported_Types_Group4 Card Specific Data: CSD Register
  * @{
  */
typedef struct
{
  __IO uint8_t  CSDStruct;            /*!< CSD structure                         */
  __IO uint8_t  SysSpecVersion;       /*!< System specification version          */
  __IO uint8_t  Reserved1;            /*!< Reserved                              */
  __IO uint8_t  TAAC;                 /*!< Data read access time 1               */
  __IO uint8_t  NSAC;                 /*!< Data read access time 2 in CLK cycles */
  __IO uint8_t  MaxBusClkFrec;        /*!< Max. bus clock frequency              */
  __IO uint16_t CardComdClasses;      /*!< Card command classes                  */
  __IO uint8_t  RdBlockLen;           /*!< Max. read data block length           */
  __IO uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed       */
  __IO uint8_t  WrBlockMisalign;      /*!< Write block misalignment              */
  __IO uint8_t  RdBlockMisalign;      /*!< Read block misalignment               */
  __IO uint8_t  DSRImpl;              /*!< DSR implemented                       */
  __IO uint8_t  Reserved2;            /*!< Reserved                              */
  __IO uint32_t DeviceSize;           /*!< Device Size                           */
  __IO uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min           */
  __IO uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max           */
  __IO uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min          */
  __IO uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max          */
  __IO uint8_t  DeviceSizeMul;        /*!< Device size multiplier                */
  __IO uint8_t  EraseGrSize;          /*!< Erase group size                      */
  __IO uint8_t  EraseGrMul;           /*!< Erase group size multiplier           */
  __IO uint8_t  WrProtectGrSize;      /*!< Write protect group size              */
  __IO uint8_t  WrProtectGrEnable;    /*!< Write protect group enable            */
  __IO uint8_t  ManDeflECC;           /*!< Manufacturer default ECC              */
  __IO uint8_t  WrSpeedFact;          /*!< Write speed factor                    */
  __IO uint8_t  MaxWrBlockLen;        /*!< Max. write data block length          */
  __IO uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed      */
  __IO uint8_t  Reserved3;            /*!< Reserved                              */
  __IO uint8_t  ContentProtectAppli;  /*!< Content protection application        */
  __IO uint8_t  FileFormatGroup;      /*!< File format group                     */
  __IO uint8_t  CopyFlag;             /*!< Copy flag (OTP)                       */
  __IO uint8_t  PermWrProtect;        /*!< Permanent write protection            */
  __IO uint8_t  TempWrProtect;        /*!< Temporary write protection            */
  __IO uint8_t  FileFormat;           /*!< File format                           */
  __IO uint8_t  ECC;                  /*!< ECC code                              */
  __IO uint8_t  CSD_CRC;              /*!< CSD CRC                               */
  __IO uint8_t  Reserved4;            /*!< Always 1                              */
}HAL_SD_CardCSDTypeDef;
/**
  * @}
  */

/** @defgroup SD_Exported_Types_Group5 Card Identification Data: CID Register
  * @{
  */
typedef struct
{
  __IO uint8_t  ManufacturerID;  /*!< Manufacturer ID       */
  __IO uint16_t OEM_AppliID;     /*!< OEM/Application ID    */
  __IO uint32_t ProdName1;       /*!< Product Name part1    */
  __IO uint8_t  ProdName2;       /*!< Product Name part2    */
  __IO uint8_t  ProdRev;         /*!< Product Revision      */
  __IO uint32_t ProdSN;          /*!< Product Serial Number */
  __IO uint8_t  Reserved1;       /*!< Reserved1             */
  __IO uint16_t ManufactDate;    /*!< Manufacturing Date    */
  __IO uint8_t  CID_CRC;         /*!< CID CRC               */
  __IO uint8_t  Reserved2;       /*!< Always 1              */

}HAL_SD_CardCIDTypeDef;
/**
  * @}
  */

/** @defgroup SD_Exported_Types_Group6 SD Card Status returned by ACMD13
  * @{
  */
typedef struct
{
  __IO uint8_t  DataBusWidth;           /*!< Shows the currently defined data bus width                 */
  __IO uint8_t  SecuredMode;            /*!< Card is in secured mode of operation                       */
  __IO uint16_t CardType;               /*!< Carries information about card type                        */
  __IO uint32_t ProtectedAreaSize;      /*!< Carries information about the capacity of protected area   */
  __IO uint8_t  SpeedClass;             /*!< Carries information about the speed class of the card      */
  __IO uint8_t  PerformanceMove;        /*!< Carries information about the card's performance move      */
  __IO uint8_t  AllocationUnitSize;     /*!< Carries information about the card's allocation unit size  */
  __IO uint16_t EraseSize;              /*!< Determines the number of AUs to be erased in one operation */
  __IO uint8_t  EraseTimeout;           /*!< Determines the timeout for any number of AU erase          */
  __IO uint8_t  EraseOffset;            /*!< Carries information about the erase offset                 */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  __IO uint8_t  UhsSpeedGrade;          /*!< Carries information about the speed grade of UHS card      */
  __IO uint8_t  UhsAllocationUnitSize;  /*!< Carries information about the UHS card's allocation unit size  */
  __IO uint8_t  VideoSpeedClass;        /*!< Carries information about the Video Speed Class of UHS card    */
#endif
}HAL_SD_CardStatusTypeDef;
/**
  * @}
  */

#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
/** @defgroup SD_Exported_Types_Group7 SD Callback ID enumeration definition
  * @{
  */
typedef enum
{
  HAL_SD_TX_CPLT_CB_ID                 = 0x00U,  /*!< SD Tx Complete Callback ID                     */
  HAL_SD_RX_CPLT_CB_ID                 = 0x01U,  /*!< SD Rx Complete Callback ID                     */
  HAL_SD_ERROR_CB_ID                   = 0x02U,  /*!< SD Error Callback ID                           */
  HAL_SD_ABORT_CB_ID                   = 0x03U,  /*!< SD Abort Callback ID                           */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  HAL_SD_READ_DMA_DBL_BUF0_CPLT_CB_ID  = 0x04U,  /*!< SD Rx DMA Double Buffer 0 Complete Callback ID */
  HAL_SD_READ_DMA_DBL_BUF1_CPLT_CB_ID  = 0x05U,  /*!< SD Rx DMA Double Buffer 1 Complete Callback ID */
  HAL_SD_WRITE_DMA_DBL_BUF0_CPLT_CB_ID = 0x06U,  /*!< SD Tx DMA Double Buffer 0 Complete Callback ID */
  HAL_SD_WRITE_DMA_DBL_BUF1_CPLT_CB_ID = 0x07U,  /*!< SD Tx DMA Double Buffer 1 Complete Callback ID */
#endif

  HAL_SD_MSP_INIT_CB_ID                = 0x10U,  /*!< SD MspInit Callback ID                         */
  HAL_SD_MSP_DEINIT_CB_ID              = 0x11U   /*!< SD MspDeInit Callback ID                       */
}HAL_SD_CallbackIDTypeDef;
/**
  * @}
  */

/** @defgroup SD_Exported_Types_Group8 SD Callback pointer definition
  * @{
  */
typedef void (*pSD_CallbackTypeDef)           (SD_HandleTypeDef *hsd);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
typedef void (*pSD_TransceiverCallbackTypeDef)(FlagStatus status);
#endif
/**
  * @}
  */
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup SD_Exported_Constants Exported Constants
  * @{
  */

#define BLOCKSIZE   ((uint32_t)512U) /*!< Block size is 512 bytes */

/** @defgroup SD_Exported_Constansts_Group1 SD Error status enumeration Structure definition
  * @{
  */
#define HAL_SD_ERROR_NONE                     SDMMC_ERROR_NONE                        /*!< No error                                                     */
#define HAL_SD_ERROR_CMD_CRC_FAIL             SDMMC_ERROR_CMD_CRC_FAIL                /*!< Command response received (but CRC check failed)             */
#define HAL_SD_ERROR_DATA_CRC_FAIL            SDMMC_ERROR_DATA_CRC_FAIL               /*!< Data block sent/received (CRC check failed)                  */
#define HAL_SD_ERROR_CMD_RSP_TIMEOUT          SDMMC_ERROR_CMD_RSP_TIMEOUT             /*!< Command response timeout                                     */
#define HAL_SD_ERROR_DATA_TIMEOUT             SDMMC_ERROR_DATA_TIMEOUT                /*!< Data timeout                                                 */
#define HAL_SD_ERROR_TX_UNDERRUN              SDMMC_ERROR_TX_UNDERRUN                 /*!< Transmit FIFO underrun                                       */
#define HAL_SD_ERROR_RX_OVERRUN               SDMMC_ERROR_RX_OVERRUN                  /*!< Receive FIFO overrun                                         */
#define HAL_SD_ERROR_ADDR_MISALIGNED          SDMMC_ERROR_ADDR_MISALIGNED             /*!< Misaligned address                                           */
#define HAL_SD_ERROR_BLOCK_LEN_ERR            SDMMC_ERROR_BLOCK_LEN_ERR               /*!< Transferred block length is not allowed for the card or the
                                                                                          number of transferred bytes does not match the block length   */
#define HAL_SD_ERROR_ERASE_SEQ_ERR            SDMMC_ERROR_ERASE_SEQ_ERR               /*!< An error in the sequence of erase command occurs             */
#define HAL_SD_ERROR_BAD_ERASE_PARAM          SDMMC_ERROR_BAD_ERASE_PARAM             /*!< An invalid selection for erase groups                        */
#define HAL_SD_ERROR_WRITE_PROT_VIOLATION     SDMMC_ERROR_WRITE_PROT_VIOLATION        /*!< Attempt to program a write protect block                     */
#define HAL_SD_ERROR_LOCK_UNLOCK_FAILED       SDMMC_ERROR_LOCK_UNLOCK_FAILED          /*!< Sequence or password error has been detected in unlock
                                                                                           command or if there was an attempt to access a locked card   */
#define HAL_SD_ERROR_COM_CRC_FAILED           SDMMC_ERROR_COM_CRC_FAILED              /*!< CRC check of the previous command failed                     */
#define HAL_SD_ERROR_ILLEGAL_CMD              SDMMC_ERROR_ILLEGAL_CMD                 /*!< Command is not legal for the card state                      */
#define HAL_SD_ERROR_CARD_ECC_FAILED          SDMMC_ERROR_CARD_ECC_FAILED             /*!< Card internal ECC was applied but failed to correct the data */
#define HAL_SD_ERROR_CC_ERR                   SDMMC_ERROR_CC_ERR                      /*!< Internal card controller error                               */
#define HAL_SD_ERROR_GENERAL_UNKNOWN_ERR      SDMMC_ERROR_GENERAL_UNKNOWN_ERR         /*!< General or unknown error                                     */
#define HAL_SD_ERROR_STREAM_READ_UNDERRUN     SDMMC_ERROR_STREAM_READ_UNDERRUN        /*!< The card could not sustain data reading in stream rmode      */
#define HAL_SD_ERROR_STREAM_WRITE_OVERRUN     SDMMC_ERROR_STREAM_WRITE_OVERRUN        /*!< The card could not sustain data programming in stream mode   */
#define HAL_SD_ERROR_CID_CSD_OVERWRITE        SDMMC_ERROR_CID_CSD_OVERWRITE           /*!< CID/CSD overwrite error                                      */
#define HAL_SD_ERROR_WP_ERASE_SKIP            SDMMC_ERROR_WP_ERASE_SKIP               /*!< Only partial address space was erased                        */
#define HAL_SD_ERROR_CARD_ECC_DISABLED        SDMMC_ERROR_CARD_ECC_DISABLED           /*!< Command has been executed without using internal ECC         */
#define HAL_SD_ERROR_ERASE_RESET              SDMMC_ERROR_ERASE_RESET                 /*!< Erase sequence was cleared before executing because an out
                                                                                           of erase sequence command was received                       */
#define HAL_SD_ERROR_AKE_SEQ_ERR              SDMMC_ERROR_AKE_SEQ_ERR                 /*!< Error in sequence of authentication                          */
#define HAL_SD_ERROR_INVALID_VOLTRANGE        SDMMC_ERROR_INVALID_VOLTRANGE           /*!< Error in case of invalid voltage range                       */
#define HAL_SD_ERROR_ADDR_OUT_OF_RANGE        SDMMC_ERROR_ADDR_OUT_OF_RANGE           /*!< Error when addressed block is out of range                   */
#define HAL_SD_ERROR_REQUEST_NOT_APPLICABLE   SDMMC_ERROR_REQUEST_NOT_APPLICABLE      /*!< Error when command request is not applicable                 */
#define HAL_SD_ERROR_PARAM                    SDMMC_ERROR_INVALID_PARAMETER           /*!< the used parameter is not valid                              */
#define HAL_SD_ERROR_UNSUPPORTED_FEATURE      SDMMC_ERROR_UNSUPPORTED_FEATURE         /*!< Error when feature is not insupported                        */
#define HAL_SD_ERROR_BUSY                     SDMMC_ERROR_BUSY                        /*!< Error when transfer process is busy                          */
#define HAL_SD_ERROR_DMA                      SDMMC_ERROR_DMA                         /*!< Error while DMA transfer                                     */
#define HAL_SD_ERROR_TIMEOUT                  SDMMC_ERROR_TIMEOUT                     /*!< Timeout error                                                 */

#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
#define HAL_SD_ERROR_INVALID_CALLBACK         SDMMC_ERROR_INVALID_PARAMETER       /*!< Invalid callback error                                        */
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @defgroup SD_Exported_Constansts_Group2 SD context enumeration
  * @{
  */
#define   SD_CONTEXT_NONE                 ((uint32_t)0x00000000U)  /*!< None                             */
#define   SD_CONTEXT_READ_SINGLE_BLOCK    ((uint32_t)0x00000001U)  /*!< Read single block operation      */
#define   SD_CONTEXT_READ_MULTIPLE_BLOCK  ((uint32_t)0x00000002U)  /*!< Read multiple blocks operation   */
#define   SD_CONTEXT_WRITE_SINGLE_BLOCK   ((uint32_t)0x00000010U)  /*!< Write single block operation     */
#define   SD_CONTEXT_WRITE_MULTIPLE_BLOCK ((uint32_t)0x00000020U)  /*!< Write multiple blocks operation  */
#define   SD_CONTEXT_IT                   ((uint32_t)0x00000008U)  /*!< Process in Interrupt mode        */
#define   SD_CONTEXT_DMA                  ((uint32_t)0x00000080U)  /*!< Process in DMA mode              */

/**
  * @}
  */

/** @defgroup SD_Exported_Constansts_Group3 SD Supported Memory Cards
  * @{
  */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define CARD_NORMAL_SPEED        ((uint32_t)0x00000000U)    /*!< Normal Speed Card <12.5Mo/s , Spec Version 1.01    */
#define CARD_HIGH_SPEED          ((uint32_t)0x00000100U)    /*!< High Speed Card <25Mo/s , Spec version 2.00        */
#define CARD_ULTRA_HIGH_SPEED    ((uint32_t)0x00000200U)    /*!< UHS-I SD Card <50Mo/s for SDR50, DDR5 Cards
                                                                 and <104Mo/s for SDR104, Spec version 3.01          */
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#define CARD_SDSC                  ((uint32_t)0x00000000U)  /*!< SD Standard Capacity <2Go                          */
#define CARD_SDHC_SDXC             ((uint32_t)0x00000001U)  /*!< SD High Capacity <32Go, SD Extended Capacity <2To  */
#define CARD_SECURED               ((uint32_t)0x00000003U)

/**
  * @}
  */

/** @defgroup SD_Exported_Constansts_Group4 SD Supported Version
  * @{
  */
#define CARD_V1_X                  ((uint32_t)0x00000000U)
#define CARD_V2_X                  ((uint32_t)0x00000001U)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup SD_Exported_macros SD Exported Macros
 *  @brief macros to handle interrupts and specific clock configurations
 * @{
 */
/** @brief Reset SD handle state.
  * @param  __HANDLE__ : SD handle.
  * @retval None
  */
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
#define __HAL_SD_RESET_HANDLE_STATE(__HANDLE__)           do {                                              \
                                                               (__HANDLE__)->State = HAL_SD_STATE_RESET; \
                                                               (__HANDLE__)->MspInitCallback = NULL;       \
                                                               (__HANDLE__)->MspDeInitCallback = NULL;     \
                                                             } while(0)
#else
#define __HAL_SD_RESET_HANDLE_STATE(__HANDLE__)           ((__HANDLE__)->State = HAL_SD_STATE_RESET)
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
/**
  * @brief  Enable the SD device.
  * @retval None
  */
#define __HAL_SD_ENABLE(__HANDLE__) __SDMMC_ENABLE((__HANDLE__)->Instance)

/**
  * @brief  Disable the SD device.
  * @retval None
  */
#define __HAL_SD_DISABLE(__HANDLE__) __SDMMC_DISABLE((__HANDLE__)->Instance)

/**
  * @brief  Enable the SDMMC DMA transfer.
  * @retval None
  */
#define __HAL_SD_DMA_ENABLE(__HANDLE__) __SDMMC_DMA_ENABLE((__HANDLE__)->Instance)

/**
  * @brief  Disable the SDMMC DMA transfer.
  * @retval None
  */
#define __HAL_SD_DMA_DISABLE(__HANDLE__)  __SDMMC_DMA_DISABLE((__HANDLE__)->Instance)
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

/**
  * @brief  Enable the SD device interrupt.
  * @param  __HANDLE__: SD Handle
  * @param  __INTERRUPT__: specifies the SDMMC interrupt sources to be enabled.
  *         This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_CMDACT:     Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:      Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:      Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE:   Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF:   Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:    Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:    Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:    Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:    Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:     Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:     Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval None
  */
#define __HAL_SD_ENABLE_IT(__HANDLE__, __INTERRUPT__) __SDMMC_ENABLE_IT((__HANDLE__)->Instance, (__INTERRUPT__))

/**
  * @brief  Disable the SD device interrupt.
  * @param  __HANDLE__: SD Handle
  * @param  __INTERRUPT__: specifies the SDMMC interrupt sources to be disabled.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_CMDACT:     Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:      Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:      Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE:   Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF:   Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:    Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:    Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:    Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:    Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:     Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:     Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval None
  */
#define __HAL_SD_DISABLE_IT(__HANDLE__, __INTERRUPT__) __SDMMC_DISABLE_IT((__HANDLE__)->Instance, (__INTERRUPT__))

/**
  * @brief  Check whether the specified SD flag is set or not.
  * @param  __HANDLE__: SD Handle
  * @param  __FLAG__: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_FLAG_CCRCFAIL:   Command response received (CRC check failed)
  *            @arg SDMMC_FLAG_DCRCFAIL:   Data block sent/received (CRC check failed)
  *            @arg SDMMC_FLAG_CTIMEOUT:   Command response timeout
  *            @arg SDMMC_FLAG_DTIMEOUT:   Data timeout
  *            @arg SDMMC_FLAG_TXUNDERR:   Transmit FIFO underrun error
  *            @arg SDMMC_FLAG_RXOVERR:    Received FIFO overrun error
  *            @arg SDMMC_FLAG_CMDREND:    Command response received (CRC check passed)
  *            @arg SDMMC_FLAG_CMDSENT:    Command sent (no response required)
  *            @arg SDMMC_FLAG_DATAEND:    Data end (data counter, DATACOUNT, is zero)
  *            @arg SDMMC_FLAG_DHOLD:      Data transfer Hold
  *            @arg SDMMC_FLAG_DBCKEND:    Data block sent/received (CRC check passed)
  *            @arg SDMMC_FLAG_DABORT:     Data transfer aborted by CMD12
  *            @arg SDMMC_FLAG_DPSMACT:    Data path state machine active
  *            @arg SDMMC_FLAG_CPSMACT:    Command path state machine active
  *            @arg SDMMC_FLAG_CMDACT:     Command transfer in progress
  *            @arg SDMMC_FLAG_TXACT:      Data transmit in progress
  *            @arg SDMMC_FLAG_RXACT:      Data receive in progress
  *            @arg SDMMC_FLAG_TXFIFOHE:   Transmit FIFO Half Empty
  *            @arg SDMMC_FLAG_RXFIFOHF:   Receive FIFO Half Full
  *            @arg SDMMC_FLAG_TXFIFOF:    Transmit FIFO full
  *            @arg SDMMC_FLAG_RXFIFOF:    Receive FIFO full
  *            @arg SDMMC_FLAG_TXFIFOE:    Transmit FIFO empty
  *            @arg SDMMC_FLAG_RXFIFOE:    Receive FIFO empty
  *            @arg SDMMC_FLAG_BUSYD0:     Inverted value of SDMMC_D0 line (Busy)
  *            @arg SDMMC_FLAG_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected
  *            @arg SDMMC_FLAG_TXDAVL:     Data available in transmit FIFO
  *            @arg SDMMC_FLAG_RXDAVL:     Data available in receive FIFO
  *            @arg SDMMC_FLAG_SDIOIT:     SDIO interrupt received
  *            @arg SDMMC_FLAG_ACKFAIL:    Boot Acknowledgment received
  *            @arg SDMMC_FLAG_ACKTIMEOUT: Boot Acknowledgment timeout
  *            @arg SDMMC_FLAG_VSWEND:     Voltage switch critical timing section completion
  *            @arg SDMMC_FLAG_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure
  *            @arg SDMMC_FLAG_IDMATE:     IDMA transfer error
  *            @arg SDMMC_FLAG_IDMABTC:    IDMA buffer transfer complete
  * @retval The new state of SD FLAG (SET or RESET).
  */
#define __HAL_SD_GET_FLAG(__HANDLE__, __FLAG__) __SDMMC_GET_FLAG((__HANDLE__)->Instance, (__FLAG__))

/**
  * @brief  Clear the SD's pending flags.
  * @param  __HANDLE__: SD Handle
  * @param  __FLAG__: specifies the flag to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_FLAG_CCRCFAIL:   Command response received (CRC check failed)
  *            @arg SDMMC_FLAG_DCRCFAIL:   Data block sent/received (CRC check failed)
  *            @arg SDMMC_FLAG_CTIMEOUT:   Command response timeout
  *            @arg SDMMC_FLAG_DTIMEOUT:   Data timeout
  *            @arg SDMMC_FLAG_TXUNDERR:   Transmit FIFO underrun error
  *            @arg SDMMC_FLAG_RXOVERR:    Received FIFO overrun error
  *            @arg SDMMC_FLAG_CMDREND:    Command response received (CRC check passed)
  *            @arg SDMMC_FLAG_CMDSENT:    Command sent (no response required)
  *            @arg SDMMC_FLAG_DATAEND:    Data end (data counter, DATACOUNT, is zero)
  *            @arg SDMMC_FLAG_DHOLD:      Data transfer Hold
  *            @arg SDMMC_FLAG_DBCKEND:    Data block sent/received (CRC check passed)
  *            @arg SDMMC_FLAG_DABORT:     Data transfer aborted by CMD12
  *            @arg SDMMC_FLAG_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected
  *            @arg SDMMC_FLAG_SDIOIT:     SDIO interrupt received
  *            @arg SDMMC_FLAG_ACKFAIL:    Boot Acknowledgment received
  *            @arg SDMMC_FLAG_ACKTIMEOUT: Boot Acknowledgment timeout
  *            @arg SDMMC_FLAG_VSWEND:     Voltage switch critical timing section completion
  *            @arg SDMMC_FLAG_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure
  *            @arg SDMMC_FLAG_IDMATE:     IDMA transfer error
  *            @arg SDMMC_FLAG_IDMABTC:    IDMA buffer transfer complete
  * @retval None
  */
#define __HAL_SD_CLEAR_FLAG(__HANDLE__, __FLAG__) __SDMMC_CLEAR_FLAG((__HANDLE__)->Instance, (__FLAG__))

/**
  * @brief  Check whether the specified SD interrupt has occurred or not.
  * @param  __HANDLE__: SD Handle
  * @param  __INTERRUPT__: specifies the SDMMC interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_CMDACT:     Command transfer in progress interrupt
  *            @arg SDMMC_IT_TXACT:      Data transmit in progress interrupt
  *            @arg SDMMC_IT_RXACT:      Data receive in progress interrupt
  *            @arg SDMMC_IT_TXFIFOHE:   Transmit FIFO Half Empty interrupt
  *            @arg SDMMC_IT_RXFIFOHF:   Receive FIFO Half Full interrupt
  *            @arg SDMMC_IT_TXFIFOF:    Transmit FIFO full interrupt
  *            @arg SDMMC_IT_RXFIFOF:    Receive FIFO full interrupt
  *            @arg SDMMC_IT_TXFIFOE:    Transmit FIFO empty interrupt
  *            @arg SDMMC_IT_RXFIFOE:    Receive FIFO empty interrupt
  *            @arg SDMMC_IT_TXDAVL:     Data available in transmit FIFO interrupt
  *            @arg SDMMC_IT_RXDAVL:     Data available in receive FIFO interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval The new state of SD IT (SET or RESET).
  */
#define __HAL_SD_GET_IT(__HANDLE__, __INTERRUPT__) __SDMMC_GET_IT((__HANDLE__)->Instance, (__INTERRUPT__))

/**
  * @brief  Clear the SD's interrupt pending bits.
  * @param  __HANDLE__: SD Handle
  * @param  __INTERRUPT__: specifies the interrupt pending bit to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDMMC_IT_CCRCFAIL:   Command response received (CRC check failed) interrupt
  *            @arg SDMMC_IT_DCRCFAIL:   Data block sent/received (CRC check failed) interrupt
  *            @arg SDMMC_IT_CTIMEOUT:   Command response timeout interrupt
  *            @arg SDMMC_IT_DTIMEOUT:   Data timeout interrupt
  *            @arg SDMMC_IT_TXUNDERR:   Transmit FIFO underrun error interrupt
  *            @arg SDMMC_IT_RXOVERR:    Received FIFO overrun error interrupt
  *            @arg SDMMC_IT_CMDREND:    Command response received (CRC check passed) interrupt
  *            @arg SDMMC_IT_CMDSENT:    Command sent (no response required) interrupt
  *            @arg SDMMC_IT_DATAEND:    Data end (data counter, DATACOUNT, is zero) interrupt
  *            @arg SDMMC_IT_DHOLD:      Data transfer Hold interrupt
  *            @arg SDMMC_IT_DBCKEND:    Data block sent/received (CRC check passed) interrupt
  *            @arg SDMMC_IT_DABORT:     Data transfer aborted by CMD12 interrupt
  *            @arg SDMMC_IT_BUSYD0END:  End of SDMMC_D0 Busy following a CMD response detected interrupt
  *            @arg SDMMC_IT_SDIOIT:     SDIO interrupt received interrupt
  *            @arg SDMMC_IT_ACKFAIL:    Boot Acknowledgment received interrupt
  *            @arg SDMMC_IT_ACKTIMEOUT: Boot Acknowledgment timeout interrupt
  *            @arg SDMMC_IT_VSWEND:     Voltage switch critical timing section completion interrupt
  *            @arg SDMMC_IT_CKSTOP:     SDMMC_CK stopped in Voltage switch procedure interrupt
  *            @arg SDMMC_IT_IDMABTC:    IDMA buffer transfer complete interrupt
  * @retval None
  */
#define __HAL_SD_CLEAR_IT(__HANDLE__, __INTERRUPT__) __SDMMC_CLEAR_IT((__HANDLE__)->Instance, (__INTERRUPT__))

/**
  * @}
  */

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/* Include SD HAL Extension module */
#include "stm32l4xx_hal_sd_ex.h"
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/* Exported functions --------------------------------------------------------*/
/** @defgroup SD_Exported_Functions SD Exported Functions
  * @{
  */

/** @defgroup SD_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
HAL_StatusTypeDef HAL_SD_Init     (SD_HandleTypeDef *hsd);
HAL_StatusTypeDef HAL_SD_InitCard (SD_HandleTypeDef *hsd);
HAL_StatusTypeDef HAL_SD_DeInit   (SD_HandleTypeDef *hsd);
void              HAL_SD_MspInit  (SD_HandleTypeDef *hsd);
void              HAL_SD_MspDeInit(SD_HandleTypeDef *hsd);
/**
  * @}
  */

/** @defgroup SD_Exported_Functions_Group2 Input and Output operation functions
  * @{
  */
/* Blocking mode: Polling */
HAL_StatusTypeDef HAL_SD_ReadBlocks     (SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);
HAL_StatusTypeDef HAL_SD_WriteBlocks    (SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);
HAL_StatusTypeDef HAL_SD_Erase          (SD_HandleTypeDef *hsd, uint32_t BlockStartAdd, uint32_t BlockEndAdd);
/* Non-Blocking mode: IT */
HAL_StatusTypeDef HAL_SD_ReadBlocks_IT  (SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
HAL_StatusTypeDef HAL_SD_WriteBlocks_IT (SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
/* Non-Blocking mode: DMA */
HAL_StatusTypeDef HAL_SD_ReadBlocks_DMA (SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
HAL_StatusTypeDef HAL_SD_WriteBlocks_DMA(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);

void              HAL_SD_IRQHandler     (SD_HandleTypeDef *hsd);

/* Callback in non blocking modes (DMA) */
void              HAL_SD_TxCpltCallback (SD_HandleTypeDef *hsd);
void              HAL_SD_RxCpltCallback (SD_HandleTypeDef *hsd);
void              HAL_SD_ErrorCallback  (SD_HandleTypeDef *hsd);
void              HAL_SD_AbortCallback  (SD_HandleTypeDef *hsd);

#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
/* SD callback registering/unregistering */
HAL_StatusTypeDef HAL_SD_RegisterCallback  (SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackID, pSD_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_SD_UnRegisterCallback(SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackID);

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
HAL_StatusTypeDef HAL_SD_RegisterTransceiverCallback  (SD_HandleTypeDef *hsd, pSD_TransceiverCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_SD_UnRegisterTransceiverCallback(SD_HandleTypeDef *hsd);
#endif
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup SD_Exported_Functions_Group3 Peripheral Control functions
  * @{
  */
HAL_StatusTypeDef HAL_SD_ConfigWideBusOperation(SD_HandleTypeDef *hsd, uint32_t WideMode);
HAL_StatusTypeDef HAL_SD_ConfigSpeedBusOperation(SD_HandleTypeDef *hsd, uint32_t SpeedMode);
/**
  * @}
  */

/** @defgroup SD_Exported_Functions_Group4 SD card related functions
  * @{
  */
HAL_StatusTypeDef       HAL_SD_SendSDStatus (SD_HandleTypeDef *hsd, uint32_t *pSDstatus);
HAL_SD_CardStateTypeDef HAL_SD_GetCardState (SD_HandleTypeDef *hsd);
HAL_StatusTypeDef       HAL_SD_GetCardCID   (SD_HandleTypeDef *hsd, HAL_SD_CardCIDTypeDef *pCID);
HAL_StatusTypeDef       HAL_SD_GetCardCSD   (SD_HandleTypeDef *hsd, HAL_SD_CardCSDTypeDef *pCSD);
HAL_StatusTypeDef       HAL_SD_GetCardStatus(SD_HandleTypeDef *hsd, HAL_SD_CardStatusTypeDef *pStatus);
HAL_StatusTypeDef       HAL_SD_GetCardInfo  (SD_HandleTypeDef *hsd, HAL_SD_CardInfoTypeDef *pCardInfo);
/**
  * @}
  */

/** @defgroup SD_Exported_Functions_Group5 Peripheral State and Errors functions
  * @{
  */
HAL_SD_StateTypeDef HAL_SD_GetState(SD_HandleTypeDef *hsd);
uint32_t            HAL_SD_GetError(SD_HandleTypeDef *hsd);
/**
  * @}
  */

/** @defgroup SD_Exported_Functions_Group6 Perioheral Abort management
  * @{
  */
HAL_StatusTypeDef HAL_SD_Abort   (SD_HandleTypeDef *hsd);
HAL_StatusTypeDef HAL_SD_Abort_IT(SD_HandleTypeDef *hsd);
/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/** @defgroup SD_Private_Types SD Private Types
  * @{
  */

/**
  * @}
  */

/* Private defines -----------------------------------------------------------*/
/** @defgroup SD_Private_Defines SD Private Defines
  * @{
  */

/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/** @defgroup SD_Private_Variables SD Private Variables
  * @{
  */

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup SD_Private_Constants SD Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup SD_Private_Macros SD Private Macros
  * @{
  */

/**
  * @}
  */

/* Private functions prototypes ----------------------------------------------*/
/** @defgroup SD_Private_Functions_Prototypes SD Private Functions Prototypes
  * @{
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @defgroup SD_Private_Functions SD Private Functions
  * @{
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

/**
  * @}
  */

#endif /* SDMMC1 */

#ifdef __cplusplus
}
#endif


#endif /* STM32L4xx_HAL_SD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
