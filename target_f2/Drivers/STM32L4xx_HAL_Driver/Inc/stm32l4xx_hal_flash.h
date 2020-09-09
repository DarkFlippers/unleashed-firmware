/**
  ******************************************************************************
  * @file    stm32l4xx_hal_flash.h
  * @author  MCD Application Team
  * @brief   Header file of FLASH HAL module.
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
#ifndef __STM32L4xx_HAL_FLASH_H
#define __STM32L4xx_HAL_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup FLASH
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup FLASH_Exported_Types FLASH Exported Types
  * @{
  */

/**
  * @brief  FLASH Erase structure definition
  */
typedef struct
{
  uint32_t TypeErase;   /*!< Mass erase or page erase.
                             This parameter can be a value of @ref FLASH_Type_Erase */
  uint32_t Banks;       /*!< Select bank to erase.
                             This parameter must be a value of @ref FLASH_Banks
                             (FLASH_BANK_BOTH should be used only for mass erase) */
  uint32_t Page;        /*!< Initial Flash page to erase when page erase is disabled
                             This parameter must be a value between 0 and (max number of pages in the bank - 1)
                             (eg : 255 for 1MB dual bank) */
  uint32_t NbPages;     /*!< Number of pages to be erased.
                             This parameter must be a value between 1 and (max number of pages in the bank - value of initial page)*/
} FLASH_EraseInitTypeDef;

/**
  * @brief  FLASH Option Bytes Program structure definition
  */
typedef struct
{
  uint32_t OptionType;     /*!< Option byte to be configured.
                                This parameter can be a combination of the values of @ref FLASH_OB_Type */
  uint32_t WRPArea;        /*!< Write protection area to be programmed (used for OPTIONBYTE_WRP).
                                Only one WRP area could be programmed at the same time.
                                This parameter can be value of @ref FLASH_OB_WRP_Area */
  uint32_t WRPStartOffset; /*!< Write protection start offset (used for OPTIONBYTE_WRP).
                                This parameter must be a value between 0 and (max number of pages in the bank - 1)
                                (eg : 25 for 1MB dual bank) */
  uint32_t WRPEndOffset;   /*!< Write protection end offset (used for OPTIONBYTE_WRP).
                                This parameter must be a value between WRPStartOffset and (max number of pages in the bank - 1) */
  uint32_t RDPLevel;       /*!< Set the read protection level.. (used for OPTIONBYTE_RDP).
                                This parameter can be a value of @ref FLASH_OB_Read_Protection */
  uint32_t USERType;       /*!< User option byte(s) to be configured (used for OPTIONBYTE_USER).
                                This parameter can be a combination of @ref FLASH_OB_USER_Type */
  uint32_t USERConfig;     /*!< Value of the user option byte (used for OPTIONBYTE_USER).
                                This parameter can be a combination of @ref FLASH_OB_USER_BOR_LEVEL,
                                @ref FLASH_OB_USER_nRST_STOP, @ref FLASH_OB_USER_nRST_STANDBY,
                                @ref FLASH_OB_USER_nRST_SHUTDOWN, @ref FLASH_OB_USER_IWDG_SW,
                                @ref FLASH_OB_USER_IWDG_STOP, @ref FLASH_OB_USER_IWDG_STANDBY,
                                @ref FLASH_OB_USER_WWDG_SW, @ref FLASH_OB_USER_BFB2,
                                @ref FLASH_OB_USER_DUALBANK, @ref FLASH_OB_USER_nBOOT1,
                                @ref FLASH_OB_USER_SRAM2_PE and @ref FLASH_OB_USER_SRAM2_RST */
  uint32_t PCROPConfig;    /*!< Configuration of the PCROP (used for OPTIONBYTE_PCROP).
                                This parameter must be a combination of @ref FLASH_Banks (except FLASH_BANK_BOTH)
                                and @ref FLASH_OB_PCROP_RDP */
  uint32_t PCROPStartAddr; /*!< PCROP Start address (used for OPTIONBYTE_PCROP).
                                This parameter must be a value between begin and end of bank
                                => Be careful of the bank swapping for the address */
  uint32_t PCROPEndAddr;   /*!< PCROP End address (used for OPTIONBYTE_PCROP).
                                This parameter must be a value between PCROP Start address and end of bank */
} FLASH_OBProgramInitTypeDef;

/**
  * @brief  FLASH Procedure structure definition
  */
typedef enum
{
  FLASH_PROC_NONE = 0,
  FLASH_PROC_PAGE_ERASE,
  FLASH_PROC_MASS_ERASE,
  FLASH_PROC_PROGRAM,
  FLASH_PROC_PROGRAM_LAST
} FLASH_ProcedureTypeDef;

/**
  * @brief  FLASH Cache structure definition
  */
typedef enum
{
  FLASH_CACHE_DISABLED = 0,
  FLASH_CACHE_ICACHE_ENABLED,
  FLASH_CACHE_DCACHE_ENABLED,
  FLASH_CACHE_ICACHE_DCACHE_ENABLED
} FLASH_CacheTypeDef;

/**
  * @brief  FLASH handle Structure definition
  */
typedef struct
{
  HAL_LockTypeDef             Lock;              /* FLASH locking object */
  __IO uint32_t               ErrorCode;         /* FLASH error code */
  __IO FLASH_ProcedureTypeDef ProcedureOnGoing;  /* Internal variable to indicate which procedure is ongoing or not in IT context */
  __IO uint32_t               Address;           /* Internal variable to save address selected for program in IT context */
  __IO uint32_t               Bank;              /* Internal variable to save current bank selected during erase in IT context */
  __IO uint32_t               Page;              /* Internal variable to define the current page which is erasing in IT context */
  __IO uint32_t               NbPagesToErase;    /* Internal variable to save the remaining pages to erase in IT context */
  __IO FLASH_CacheTypeDef     CacheToReactivate; /* Internal variable to indicate which caches should be reactivated */
}FLASH_ProcessTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup FLASH_Exported_Constants FLASH Exported Constants
  * @{
  */

/** @defgroup FLASH_Error FLASH Error
  * @{
  */
#define HAL_FLASH_ERROR_NONE      0x00000000U
#define HAL_FLASH_ERROR_OP        FLASH_FLAG_OPERR
#define HAL_FLASH_ERROR_PROG      FLASH_FLAG_PROGERR
#define HAL_FLASH_ERROR_WRP       FLASH_FLAG_WRPERR
#define HAL_FLASH_ERROR_PGA       FLASH_FLAG_PGAERR
#define HAL_FLASH_ERROR_SIZ       FLASH_FLAG_SIZERR
#define HAL_FLASH_ERROR_PGS       FLASH_FLAG_PGSERR
#define HAL_FLASH_ERROR_MIS       FLASH_FLAG_MISERR
#define HAL_FLASH_ERROR_FAST      FLASH_FLAG_FASTERR
#define HAL_FLASH_ERROR_RD        FLASH_FLAG_RDERR
#define HAL_FLASH_ERROR_OPTV      FLASH_FLAG_OPTVERR
#define HAL_FLASH_ERROR_ECCD      FLASH_FLAG_ECCD
#if defined (STM32L412xx) || defined (STM32L422xx) || defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || \
    defined (STM32L443xx) || defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || \
    defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define HAL_FLASH_ERROR_PEMPTY    FLASH_FLAG_PEMPTY
#endif
/**
  * @}
  */

/** @defgroup FLASH_Type_Erase FLASH Erase Type
  * @{
  */
#define FLASH_TYPEERASE_PAGES     ((uint32_t)0x00)  /*!<Pages erase only*/
#define FLASH_TYPEERASE_MASSERASE ((uint32_t)0x01)  /*!<Flash mass erase activation*/
/**
  * @}
  */

/** @defgroup FLASH_Banks FLASH Banks
  * @{
  */
#define FLASH_BANK_1              ((uint32_t)0x01)                          /*!< Bank 1   */
#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_BANK_2              ((uint32_t)0x02)                          /*!< Bank 2   */
#define FLASH_BANK_BOTH           ((uint32_t)(FLASH_BANK_1 | FLASH_BANK_2)) /*!< Bank1 and Bank2  */
#else
#define FLASH_BANK_BOTH           ((uint32_t)(FLASH_BANK_1))                /*!< Bank 1   */
#endif
/**
  * @}
  */


/** @defgroup FLASH_Type_Program FLASH Program Type
  * @{
  */
#define FLASH_TYPEPROGRAM_DOUBLEWORD    ((uint32_t)0x00)  /*!<Program a double-word (64-bit) at a specified address.*/
#define FLASH_TYPEPROGRAM_FAST          ((uint32_t)0x01)  /*!<Fast program a 32 row double-word (64-bit) at a specified address.
                                                                 And another 32 row double-word (64-bit) will be programmed */
#define FLASH_TYPEPROGRAM_FAST_AND_LAST ((uint32_t)0x02)  /*!<Fast program a 32 row double-word (64-bit) at a specified address.
                                                                 And this is the last 32 row double-word (64-bit) programmed */
/**
  * @}
  */

/** @defgroup FLASH_OB_Type FLASH Option Bytes Type
  * @{
  */
#define OPTIONBYTE_WRP            ((uint32_t)0x01)  /*!< WRP option byte configuration */
#define OPTIONBYTE_RDP            ((uint32_t)0x02)  /*!< RDP option byte configuration */
#define OPTIONBYTE_USER           ((uint32_t)0x04)  /*!< USER option byte configuration */
#define OPTIONBYTE_PCROP          ((uint32_t)0x08)  /*!< PCROP option byte configuration */
/**
  * @}
  */

/** @defgroup FLASH_OB_WRP_Area FLASH WRP Area
  * @{
  */
#define OB_WRPAREA_BANK1_AREAA    ((uint32_t)0x00)  /*!< Flash Bank 1 Area A */
#define OB_WRPAREA_BANK1_AREAB    ((uint32_t)0x01)  /*!< Flash Bank 1 Area B */
#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define OB_WRPAREA_BANK2_AREAA    ((uint32_t)0x02)  /*!< Flash Bank 2 Area A */
#define OB_WRPAREA_BANK2_AREAB    ((uint32_t)0x04)  /*!< Flash Bank 2 Area B */
#endif
/**
  * @}
  */

/** @defgroup FLASH_OB_Read_Protection FLASH Option Bytes Read Protection
  * @{
  */
#define OB_RDP_LEVEL_0            ((uint32_t)0xAA)
#define OB_RDP_LEVEL_1            ((uint32_t)0xBB)
#define OB_RDP_LEVEL_2            ((uint32_t)0xCC) /*!< Warning: When enabling read protection level 2
                                                        it's no more possible to go back to level 1 or 0 */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_Type FLASH Option Bytes User Type
  * @{
  */
#define OB_USER_BOR_LEV           ((uint32_t)0x0001)                   /*!< BOR reset Level */
#define OB_USER_nRST_STOP         ((uint32_t)0x0002)                   /*!< Reset generated when entering the stop mode */
#define OB_USER_nRST_STDBY        ((uint32_t)0x0004)                   /*!< Reset generated when entering the standby mode */
#define OB_USER_IWDG_SW           ((uint32_t)0x0008)                   /*!< Independent watchdog selection */
#define OB_USER_IWDG_STOP         ((uint32_t)0x0010)                   /*!< Independent watchdog counter freeze in stop mode */
#define OB_USER_IWDG_STDBY        ((uint32_t)0x0020)                   /*!< Independent watchdog counter freeze in standby mode */
#define OB_USER_WWDG_SW           ((uint32_t)0x0040)                   /*!< Window watchdog selection */
#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define OB_USER_BFB2              ((uint32_t)0x0080)                   /*!< Dual-bank boot */
#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define OB_USER_DUALBANK          ((uint32_t)0x0100)                   /*!< Dual-Bank on 1MB or 512kB Flash memory devices */
#else
#define OB_USER_DUALBANK          ((uint32_t)0x0100)                   /*!< Dual-Bank on 512KB or 256KB Flash memory devices */
#endif
#endif
#define OB_USER_nBOOT1            ((uint32_t)0x0200)                   /*!< Boot configuration */
#define OB_USER_SRAM2_PE          ((uint32_t)0x0400)                   /*!< SRAM2 parity check enable */
#define OB_USER_SRAM2_RST         ((uint32_t)0x0800)                   /*!< SRAM2 Erase when system reset */
#define OB_USER_nRST_SHDW         ((uint32_t)0x1000)                   /*!< Reset generated when entering the shutdown mode */
#if defined (STM32L412xx) || defined (STM32L422xx) || defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || \
    defined (STM32L442xx) || defined (STM32L443xx) || defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define OB_USER_nSWBOOT0          ((uint32_t)0x2000)                   /*!< Software BOOT0 */
#define OB_USER_nBOOT0            ((uint32_t)0x4000)                   /*!< nBOOT0 option bit */
#endif
#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define OB_USER_DBANK             ((uint32_t)0x8000)                   /*!< Single bank with 128-bits data or two banks with 64-bits data */
#endif
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_BOR_LEVEL FLASH Option Bytes User BOR Level
  * @{
  */
#define OB_BOR_LEVEL_0            ((uint32_t)FLASH_OPTR_BOR_LEV_0)     /*!< Reset level threshold is around 1.7V */
#define OB_BOR_LEVEL_1            ((uint32_t)FLASH_OPTR_BOR_LEV_1)     /*!< Reset level threshold is around 2.0V */
#define OB_BOR_LEVEL_2            ((uint32_t)FLASH_OPTR_BOR_LEV_2)     /*!< Reset level threshold is around 2.2V */
#define OB_BOR_LEVEL_3            ((uint32_t)FLASH_OPTR_BOR_LEV_3)     /*!< Reset level threshold is around 2.5V */
#define OB_BOR_LEVEL_4            ((uint32_t)FLASH_OPTR_BOR_LEV_4)     /*!< Reset level threshold is around 2.8V */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_nRST_STOP FLASH Option Bytes User Reset On Stop
  * @{
  */
#define OB_STOP_RST               ((uint32_t)0x0000)                   /*!< Reset generated when entering the stop mode */
#define OB_STOP_NORST             ((uint32_t)FLASH_OPTR_nRST_STOP)     /*!< No reset generated when entering the stop mode */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_nRST_STANDBY FLASH Option Bytes User Reset On Standby
  * @{
  */
#define OB_STANDBY_RST            ((uint32_t)0x0000)                   /*!< Reset generated when entering the standby mode */
#define OB_STANDBY_NORST          ((uint32_t)FLASH_OPTR_nRST_STDBY)    /*!< No reset generated when entering the standby mode */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_nRST_SHUTDOWN FLASH Option Bytes User Reset On Shutdown
  * @{
  */
#define OB_SHUTDOWN_RST           ((uint32_t)0x0000)                   /*!< Reset generated when entering the shutdown mode */
#define OB_SHUTDOWN_NORST         ((uint32_t)FLASH_OPTR_nRST_SHDW)     /*!< No reset generated when entering the shutdown mode */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_IWDG_SW FLASH Option Bytes User IWDG Type
  * @{
  */
#define OB_IWDG_HW                ((uint32_t)0x00000)                  /*!< Hardware independent watchdog */
#define OB_IWDG_SW                ((uint32_t)FLASH_OPTR_IWDG_SW)       /*!< Software independent watchdog */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_IWDG_STOP FLASH Option Bytes User IWDG Mode On Stop
  * @{
  */
#define OB_IWDG_STOP_FREEZE       ((uint32_t)0x00000)                  /*!< Independent watchdog counter is frozen in Stop mode */
#define OB_IWDG_STOP_RUN          ((uint32_t)FLASH_OPTR_IWDG_STOP)     /*!< Independent watchdog counter is running in Stop mode */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_IWDG_STANDBY FLASH Option Bytes User IWDG Mode On Standby
  * @{
  */
#define OB_IWDG_STDBY_FREEZE      ((uint32_t)0x00000)                  /*!< Independent watchdog counter is frozen in Standby mode */
#define OB_IWDG_STDBY_RUN         ((uint32_t)FLASH_OPTR_IWDG_STDBY)    /*!< Independent watchdog counter is running in Standby mode */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_WWDG_SW FLASH Option Bytes User WWDG Type
  * @{
  */
#define OB_WWDG_HW                ((uint32_t)0x00000)                  /*!< Hardware window watchdog */
#define OB_WWDG_SW                ((uint32_t)FLASH_OPTR_WWDG_SW)       /*!< Software window watchdog */
/**
  * @}
  */

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
/** @defgroup FLASH_OB_USER_BFB2 FLASH Option Bytes User BFB2 Mode
  * @{
  */
#define OB_BFB2_DISABLE           ((uint32_t)0x000000)                 /*!< Dual-bank boot disable */
#define OB_BFB2_ENABLE            ((uint32_t)FLASH_OPTR_BFB2)          /*!< Dual-bank boot enable */
/**
  * @}
  */
#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
/** @defgroup FLASH_OB_USER_DUALBANK FLASH Option Bytes User Dual-bank Type
  * @{
  */
#define OB_DUALBANK_SINGLE        ((uint32_t)0x000000)                 /*!< 1 MB/512 kB Single-bank Flash */
#define OB_DUALBANK_DUAL          ((uint32_t)FLASH_OPTR_DB1M)          /*!< 1 MB/512 kB Dual-bank Flash */
/**
  * @}
  */
#else
/** @defgroup FLASH_OB_USER_DUALBANK FLASH Option Bytes User Dual-bank Type
  * @{
  */
#define OB_DUALBANK_SINGLE        ((uint32_t)0x000000)                 /*!< 256 KB/512 KB Single-bank Flash */
#define OB_DUALBANK_DUAL          ((uint32_t)FLASH_OPTR_DUALBANK)      /*!< 256 KB/512 KB Dual-bank Flash */
/**
  * @}
  */
#endif
#endif

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
/** @defgroup FLASH_OB_USER_DBANK FLASH Option Bytes User DBANK Type
  * @{
  */
#define OB_DBANK_128_BITS         ((uint32_t)0x000000)                 /*!< Single-bank with 128-bits data */
#define OB_DBANK_64_BITS          ((uint32_t)FLASH_OPTR_DBANK)         /*!< Dual-bank with 64-bits data */
#endif
/**
  * @}
  */
/** @defgroup FLASH_OB_USER_nBOOT1 FLASH Option Bytes User BOOT1 Type
  * @{
  */
#define OB_BOOT1_SRAM             ((uint32_t)0x000000)                 /*!< Embedded SRAM1 is selected as boot space (if BOOT0=1) */
#define OB_BOOT1_SYSTEM           ((uint32_t)FLASH_OPTR_nBOOT1)        /*!< System memory is selected as boot space (if BOOT0=1) */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_SRAM2_PE FLASH Option Bytes User SRAM2 Parity Check Type
  * @{
  */
#define OB_SRAM2_PARITY_ENABLE    ((uint32_t)0x0000000)                /*!< SRAM2 parity check enable */
#define OB_SRAM2_PARITY_DISABLE   ((uint32_t)FLASH_OPTR_SRAM2_PE)      /*!< SRAM2 parity check disable */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_SRAM2_RST FLASH Option Bytes User SRAM2 Erase On Reset Type
  * @{
  */
#define OB_SRAM2_RST_ERASE        ((uint32_t)0x0000000)                /*!< SRAM2 erased when a system reset occurs */
#define OB_SRAM2_RST_NOT_ERASE    ((uint32_t)FLASH_OPTR_SRAM2_RST)     /*!< SRAM2 is not erased when a system reset occurs */
/**
  * @}
  */

#if defined (STM32L412xx) || defined (STM32L422xx) || defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || \
    defined (STM32L442xx) || defined (STM32L443xx) || defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
/** @defgroup OB_USER_nSWBOOT0 FLASH Option Bytes User Software BOOT0
  * @{
  */
#define OB_BOOT0_FROM_OB          ((uint32_t)0x0000000)                /*!< BOOT0 taken from the option bit nBOOT0 */
#define OB_BOOT0_FROM_PIN         ((uint32_t)FLASH_OPTR_nSWBOOT0)      /*!< BOOT0 taken from PH3/BOOT0 pin */
/**
  * @}
  */

/** @defgroup OB_USER_nBOOT0 FLASH Option Bytes User nBOOT0 option bit
  * @{
  */
#define OB_BOOT0_RESET            ((uint32_t)0x0000000)                /*!< nBOOT0 = 0 */
#define OB_BOOT0_SET              ((uint32_t)FLASH_OPTR_nBOOT0)        /*!< nBOOT0 = 1 */
/**
  * @}
  */
#endif

/** @defgroup FLASH_OB_PCROP_RDP FLASH Option Bytes PCROP On RDP Level Type
  * @{
  */
#define OB_PCROP_RDP_NOT_ERASE    ((uint32_t)0x00000000)               /*!< PCROP area is not erased when the RDP level
                                                                            is decreased from Level 1 to Level 0 */
#define OB_PCROP_RDP_ERASE        ((uint32_t)FLASH_PCROP1ER_PCROP_RDP) /*!< PCROP area is erased when the RDP level is
                                                                            decreased from Level 1 to Level 0 (full mass erase) */
/**
  * @}
  */

/** @defgroup FLASH_Latency FLASH Latency
  * @{
  */
#define FLASH_LATENCY_0           FLASH_ACR_LATENCY_0WS                /*!< FLASH Zero wait state */
#define FLASH_LATENCY_1           FLASH_ACR_LATENCY_1WS                /*!< FLASH One wait state */
#define FLASH_LATENCY_2           FLASH_ACR_LATENCY_2WS                /*!< FLASH Two wait states */
#define FLASH_LATENCY_3           FLASH_ACR_LATENCY_3WS                /*!< FLASH Three wait states */
#define FLASH_LATENCY_4           FLASH_ACR_LATENCY_4WS                /*!< FLASH Four wait states */
#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_LATENCY_5           FLASH_ACR_LATENCY_5WS                /*!< FLASH Five wait state */
#define FLASH_LATENCY_6           FLASH_ACR_LATENCY_6WS                /*!< FLASH Six wait state */
#define FLASH_LATENCY_7           FLASH_ACR_LATENCY_7WS                /*!< FLASH Seven wait states */
#define FLASH_LATENCY_8           FLASH_ACR_LATENCY_8WS                /*!< FLASH Eight wait states */
#define FLASH_LATENCY_9           FLASH_ACR_LATENCY_9WS                /*!< FLASH Nine wait states */
#define FLASH_LATENCY_10          FLASH_ACR_LATENCY_10WS               /*!< FLASH Ten wait state */
#define FLASH_LATENCY_11          FLASH_ACR_LATENCY_11WS               /*!< FLASH Eleven wait state */
#define FLASH_LATENCY_12          FLASH_ACR_LATENCY_12WS               /*!< FLASH Twelve wait states */
#define FLASH_LATENCY_13          FLASH_ACR_LATENCY_13WS               /*!< FLASH Thirteen wait states */
#define FLASH_LATENCY_14          FLASH_ACR_LATENCY_14WS               /*!< FLASH Fourteen wait states */
#define FLASH_LATENCY_15          FLASH_ACR_LATENCY_15WS               /*!< FLASH Fifteen wait states */
#endif
/**
  * @}
  */

/** @defgroup FLASH_Keys FLASH Keys
  * @{
  */
#define FLASH_KEY1                0x45670123U                          /*!< Flash key1 */
#define FLASH_KEY2                0xCDEF89ABU                          /*!< Flash key2: used with FLASH_KEY1
                                                                            to unlock the FLASH registers access */

#define FLASH_PDKEY1              0x04152637U                          /*!< Flash power down key1 */
#define FLASH_PDKEY2              0xFAFBFCFDU                          /*!< Flash power down key2: used with FLASH_PDKEY1
                                                                            to unlock the RUN_PD bit in FLASH_ACR */

#define FLASH_OPTKEY1             0x08192A3BU                          /*!< Flash option byte key1 */
#define FLASH_OPTKEY2             0x4C5D6E7FU                          /*!< Flash option byte key2: used with FLASH_OPTKEY1
                                                                            to allow option bytes operations */
/**
  * @}
  */

/** @defgroup FLASH_Flags FLASH Flags Definition
  * @{
  */
#define FLASH_FLAG_EOP            FLASH_SR_EOP                         /*!< FLASH End of operation flag */
#define FLASH_FLAG_OPERR          FLASH_SR_OPERR                       /*!< FLASH Operation error flag */
#define FLASH_FLAG_PROGERR        FLASH_SR_PROGERR                     /*!< FLASH Programming error flag */
#define FLASH_FLAG_WRPERR         FLASH_SR_WRPERR                      /*!< FLASH Write protection error flag */
#define FLASH_FLAG_PGAERR         FLASH_SR_PGAERR                      /*!< FLASH Programming alignment error flag */
#define FLASH_FLAG_SIZERR         FLASH_SR_SIZERR                      /*!< FLASH Size error flag  */
#define FLASH_FLAG_PGSERR         FLASH_SR_PGSERR                      /*!< FLASH Programming sequence error flag */
#define FLASH_FLAG_MISERR         FLASH_SR_MISERR                      /*!< FLASH Fast programming data miss error flag */
#define FLASH_FLAG_FASTERR        FLASH_SR_FASTERR                     /*!< FLASH Fast programming error flag */
#define FLASH_FLAG_RDERR          FLASH_SR_RDERR                       /*!< FLASH PCROP read error flag */
#define FLASH_FLAG_OPTVERR        FLASH_SR_OPTVERR                     /*!< FLASH Option validity error flag  */
#define FLASH_FLAG_BSY            FLASH_SR_BSY                         /*!< FLASH Busy flag */
#if defined (STM32L412xx) || defined (STM32L422xx) || defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || \
    defined (STM32L443xx) || defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx) || defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || \
    defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_FLAG_PEMPTY         FLASH_SR_PEMPTY                      /*!< FLASH Program empty */
#define FLASH_FLAG_SR_ERRORS      (FLASH_FLAG_OPERR   | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR | \
                                   FLASH_FLAG_PGAERR  | FLASH_FLAG_SIZERR  | FLASH_FLAG_PGSERR | \
                                   FLASH_FLAG_MISERR  | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR  | \
                                   FLASH_FLAG_OPTVERR | FLASH_FLAG_PEMPTY)
#else
#define FLASH_FLAG_SR_ERRORS      (FLASH_FLAG_OPERR   | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR | \
                                   FLASH_FLAG_PGAERR  | FLASH_FLAG_SIZERR  | FLASH_FLAG_PGSERR | \
                                   FLASH_FLAG_MISERR  | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR  | \
                                   FLASH_FLAG_OPTVERR)
#endif
#define FLASH_FLAG_ECCC           FLASH_ECCR_ECCC                      /*!< FLASH ECC correction */
#define FLASH_FLAG_ECCD           FLASH_ECCR_ECCD                      /*!< FLASH ECC detection */

#define FLASH_FLAG_ALL_ERRORS     (FLASH_FLAG_OPERR   | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR | \
                                   FLASH_FLAG_PGAERR  | FLASH_FLAG_SIZERR  | FLASH_FLAG_PGSERR | \
                                   FLASH_FLAG_MISERR  | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR  | \
                                   FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCD)
/**
  * @}
  */

/** @defgroup FLASH_Interrupt_definition FLASH Interrupts Definition
  * @brief FLASH Interrupt definition
  * @{
  */
#define FLASH_IT_EOP              FLASH_CR_EOPIE                       /*!< End of FLASH Operation Interrupt source */
#define FLASH_IT_OPERR            FLASH_CR_ERRIE                       /*!< Error Interrupt source */
#define FLASH_IT_RDERR            FLASH_CR_RDERRIE                     /*!< PCROP Read Error Interrupt source*/
#define FLASH_IT_ECCC             (FLASH_ECCR_ECCIE >> 24)             /*!< ECC Correction Interrupt source */
/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup FLASH_Exported_Macros FLASH Exported Macros
 *  @brief macros to control FLASH features
 *  @{
 */

/**
  * @brief  Set the FLASH Latency.
  * @param  __LATENCY__: FLASH Latency
  *         This parameter can be one of the following values :
  *     @arg FLASH_LATENCY_0: FLASH Zero wait state
  *     @arg FLASH_LATENCY_1: FLASH One wait state
  *     @arg FLASH_LATENCY_2: FLASH Two wait states
  *     @arg FLASH_LATENCY_3: FLASH Three wait states
  *     @arg FLASH_LATENCY_4: FLASH Four wait states
  * @retval None
  */
#define __HAL_FLASH_SET_LATENCY(__LATENCY__)    (MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (__LATENCY__)))

/**
  * @brief  Get the FLASH Latency.
  * @retval FLASH Latency
  *         This parameter can be one of the following values :
  *     @arg FLASH_LATENCY_0: FLASH Zero wait state
  *     @arg FLASH_LATENCY_1: FLASH One wait state
  *     @arg FLASH_LATENCY_2: FLASH Two wait states
  *     @arg FLASH_LATENCY_3: FLASH Three wait states
  *     @arg FLASH_LATENCY_4: FLASH Four wait states
  */
#define __HAL_FLASH_GET_LATENCY()               READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY)

/**
  * @brief  Enable the FLASH prefetch buffer.
  * @retval None
  */
#define __HAL_FLASH_PREFETCH_BUFFER_ENABLE()    SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN)

/**
  * @brief  Disable the FLASH prefetch buffer.
  * @retval None
  */
#define __HAL_FLASH_PREFETCH_BUFFER_DISABLE()   CLEAR_BIT(FLASH->ACR, FLASH_ACR_PRFTEN)

/**
  * @brief  Enable the FLASH instruction cache.
  * @retval none
  */
#define __HAL_FLASH_INSTRUCTION_CACHE_ENABLE()  SET_BIT(FLASH->ACR, FLASH_ACR_ICEN)

/**
  * @brief  Disable the FLASH instruction cache.
  * @retval none
  */
#define __HAL_FLASH_INSTRUCTION_CACHE_DISABLE() CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICEN)

/**
  * @brief  Enable the FLASH data cache.
  * @retval none
  */
#define __HAL_FLASH_DATA_CACHE_ENABLE()         SET_BIT(FLASH->ACR, FLASH_ACR_DCEN)

/**
  * @brief  Disable the FLASH data cache.
  * @retval none
  */
#define __HAL_FLASH_DATA_CACHE_DISABLE()        CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCEN)

/**
  * @brief  Reset the FLASH instruction Cache.
  * @note   This function must be used only when the Instruction Cache is disabled.
  * @retval None
  */
#define __HAL_FLASH_INSTRUCTION_CACHE_RESET()   do { SET_BIT(FLASH->ACR, FLASH_ACR_ICRST);   \
                                                     CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICRST); \
                                                   } while (0)

/**
  * @brief  Reset the FLASH data Cache.
  * @note   This function must be used only when the data Cache is disabled.
  * @retval None
  */
#define __HAL_FLASH_DATA_CACHE_RESET()          do { SET_BIT(FLASH->ACR, FLASH_ACR_DCRST);   \
                                                     CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCRST); \
                                                   } while (0)

/**
  * @brief  Enable the FLASH power down during Low-power run mode.
  * @note   Writing this bit  to 0 this bit, automatically the keys are
  *         loss and a new unlock sequence is necessary to re-write it to 1.
  */
#define __HAL_FLASH_POWER_DOWN_ENABLE()         do { WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY1); \
                                                     WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY2); \
                                                     SET_BIT(FLASH->ACR, FLASH_ACR_RUN_PD);   \
                                                   } while (0)

/**
  * @brief  Disable the FLASH power down during Low-power run mode.
  * @note   Writing this bit  to 0 this bit, automatically the keys are
  *         loss and a new unlock sequence is necessary to re-write it to 1.
  */
#define __HAL_FLASH_POWER_DOWN_DISABLE()        do { WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY1); \
                                                     WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY2); \
                                                     CLEAR_BIT(FLASH->ACR, FLASH_ACR_RUN_PD); \
                                                   } while (0)

/**
  * @brief  Enable the FLASH power down during Low-Power sleep mode
  * @retval none
  */
#define __HAL_FLASH_SLEEP_POWERDOWN_ENABLE()    SET_BIT(FLASH->ACR, FLASH_ACR_SLEEP_PD)

/**
  * @brief  Disable the FLASH power down during Low-Power sleep mode
  * @retval none
  */
#define __HAL_FLASH_SLEEP_POWERDOWN_DISABLE()   CLEAR_BIT(FLASH->ACR, FLASH_ACR_SLEEP_PD)

/**
  * @}
  */

/** @defgroup FLASH_Interrupt FLASH Interrupts Macros
 *  @brief macros to handle FLASH interrupts
 * @{
 */

/**
  * @brief  Enable the specified FLASH interrupt.
  * @param  __INTERRUPT__: FLASH interrupt
  *         This parameter can be any combination of the following values:
  *     @arg FLASH_IT_EOP: End of FLASH Operation Interrupt
  *     @arg FLASH_IT_OPERR: Error Interrupt
  *     @arg FLASH_IT_RDERR: PCROP Read Error Interrupt
  *     @arg FLASH_IT_ECCC: ECC Correction Interrupt
  * @retval none
  */
#define __HAL_FLASH_ENABLE_IT(__INTERRUPT__)    do { if(((__INTERRUPT__) & FLASH_IT_ECCC) != 0U) { SET_BIT(FLASH->ECCR, FLASH_ECCR_ECCIE); }\
                                                     if(((__INTERRUPT__) & (~FLASH_IT_ECCC)) != 0U) { SET_BIT(FLASH->CR, ((__INTERRUPT__) & (~FLASH_IT_ECCC))); }\
                                                   } while(0)

/**
  * @brief  Disable the specified FLASH interrupt.
  * @param  __INTERRUPT__: FLASH interrupt
  *         This parameter can be any combination of the following values:
  *     @arg FLASH_IT_EOP: End of FLASH Operation Interrupt
  *     @arg FLASH_IT_OPERR: Error Interrupt
  *     @arg FLASH_IT_RDERR: PCROP Read Error Interrupt
  *     @arg FLASH_IT_ECCC: ECC Correction Interrupt
  * @retval none
  */
#define __HAL_FLASH_DISABLE_IT(__INTERRUPT__)   do { if(((__INTERRUPT__) & FLASH_IT_ECCC) != 0U) { CLEAR_BIT(FLASH->ECCR, FLASH_ECCR_ECCIE); }\
                                                     if(((__INTERRUPT__) & (~FLASH_IT_ECCC)) != 0U) { CLEAR_BIT(FLASH->CR, ((__INTERRUPT__) & (~FLASH_IT_ECCC))); }\
                                                   } while(0)

/**
  * @brief  Check whether the specified FLASH flag is set or not.
  * @param  __FLAG__: specifies the FLASH flag to check.
  *   This parameter can be one of the following values:
  *     @arg FLASH_FLAG_EOP: FLASH End of Operation flag
  *     @arg FLASH_FLAG_OPERR: FLASH Operation error flag
  *     @arg FLASH_FLAG_PROGERR: FLASH Programming error flag
  *     @arg FLASH_FLAG_WRPERR: FLASH Write protection error flag
  *     @arg FLASH_FLAG_PGAERR: FLASH Programming alignment error flag
  *     @arg FLASH_FLAG_SIZERR: FLASH Size error flag
  *     @arg FLASH_FLAG_PGSERR: FLASH Programming sequence error flag
  *     @arg FLASH_FLAG_MISERR: FLASH Fast programming data miss error flag
  *     @arg FLASH_FLAG_FASTERR: FLASH Fast programming error flag
  *     @arg FLASH_FLAG_RDERR: FLASH PCROP read  error flag
  *     @arg FLASH_FLAG_OPTVERR: FLASH Option validity error flag
  *     @arg FLASH_FLAG_BSY: FLASH write/erase operations in progress flag
  *     @arg FLASH_FLAG_PEMPTY : FLASH Boot from not programmed flash (apply only for STM32L43x/STM32L44x devices)
  *     @arg FLASH_FLAG_ECCC: FLASH one ECC error has been detected and corrected
  *     @arg FLASH_FLAG_ECCD: FLASH two ECC errors have been detected
  * @retval The new state of FLASH_FLAG (SET or RESET).
  */
#define __HAL_FLASH_GET_FLAG(__FLAG__)          ((((__FLAG__) & (FLASH_FLAG_ECCC | FLASH_FLAG_ECCD)) != 0U) ? \
                                                 (READ_BIT(FLASH->ECCR, (__FLAG__)) == (__FLAG__))          : \
                                                 (READ_BIT(FLASH->SR,   (__FLAG__)) == (__FLAG__)))

/**
  * @brief  Clear the FLASH's pending flags.
  * @param  __FLAG__: specifies the FLASH flags to clear.
  *   This parameter can be any combination of the following values:
  *     @arg FLASH_FLAG_EOP: FLASH End of Operation flag
  *     @arg FLASH_FLAG_OPERR: FLASH Operation error flag
  *     @arg FLASH_FLAG_PROGERR: FLASH Programming error flag
  *     @arg FLASH_FLAG_WRPERR: FLASH Write protection error flag
  *     @arg FLASH_FLAG_PGAERR: FLASH Programming alignment error flag
  *     @arg FLASH_FLAG_SIZERR: FLASH Size error flag
  *     @arg FLASH_FLAG_PGSERR: FLASH Programming sequence error flag
  *     @arg FLASH_FLAG_MISERR: FLASH Fast programming data miss error flag
  *     @arg FLASH_FLAG_FASTERR: FLASH Fast programming error flag
  *     @arg FLASH_FLAG_RDERR: FLASH PCROP read  error flag
  *     @arg FLASH_FLAG_OPTVERR: FLASH Option validity error flag
  *     @arg FLASH_FLAG_ECCC: FLASH one ECC error has been detected and corrected
  *     @arg FLASH_FLAG_ECCD: FLASH two ECC errors have been detected
  *     @arg FLASH_FLAG_ALL_ERRORS: FLASH All errors flags
  * @retval None
  */
#define __HAL_FLASH_CLEAR_FLAG(__FLAG__)        do { if(((__FLAG__) & (FLASH_FLAG_ECCC | FLASH_FLAG_ECCD)) != 0U) { SET_BIT(FLASH->ECCR, ((__FLAG__) & (FLASH_FLAG_ECCC | FLASH_FLAG_ECCD))); }\
                                                     if(((__FLAG__) & ~(FLASH_FLAG_ECCC | FLASH_FLAG_ECCD)) != 0U) { WRITE_REG(FLASH->SR, ((__FLAG__) & ~(FLASH_FLAG_ECCC | FLASH_FLAG_ECCD))); }\
                                                   } while(0)
/**
  * @}
  */

/* Include FLASH HAL Extended module */
#include "stm32l4xx_hal_flash_ex.h"
#include "stm32l4xx_hal_flash_ramfunc.h"

/* Exported functions --------------------------------------------------------*/
/** @addtogroup FLASH_Exported_Functions
  * @{
  */

/* Program operation functions  ***********************************************/
/** @addtogroup FLASH_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef  HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data);
HAL_StatusTypeDef  HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint64_t Data);
/* FLASH IRQ handler method */
void               HAL_FLASH_IRQHandler(void);
/* Callbacks in non blocking modes */
void               HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue);
void               HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);
/**
  * @}
  */

/* Peripheral Control functions  **********************************************/
/** @addtogroup FLASH_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef  HAL_FLASH_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_Lock(void);
/* Option bytes control */
HAL_StatusTypeDef  HAL_FLASH_OB_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Lock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Launch(void);
/**
  * @}
  */

/* Peripheral State functions  ************************************************/
/** @addtogroup FLASH_Exported_Functions_Group3
  * @{
  */
uint32_t HAL_FLASH_GetError(void);
/**
  * @}
  */

/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/** @addtogroup FLASH_Private_Variables FLASH Private Variables
 * @{
 */
extern FLASH_ProcessTypeDef pFlash;
/**
  * @}
  */

/* Private function ----------------------------------------------------------*/
/** @addtogroup FLASH_Private_Functions FLASH Private Functions
 * @{
 */
HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);
/**
  * @}
  */

/* Private constants --------------------------------------------------------*/
/** @defgroup FLASH_Private_Constants FLASH Private Constants
  * @{
  */
#define FLASH_SIZE_DATA_REGISTER           ((uint32_t)0x1FFF75E0)

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_SIZE                         (((((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) == 0x0000FFFFU)) ? (0x800U << 10U) : \
                                            (((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) << 10U))
#elif defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx)
#define FLASH_SIZE                         (((((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) == 0x0000FFFFU)) ? (0x200U << 10U) : \
                                            (((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) << 10U))
#elif defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx)
#define FLASH_SIZE                         (((((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) == 0x0000FFFFU)) ? (0x100U << 10U) : \
                                            (((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) << 10U))
#elif defined (STM32L412xx) || defined (STM32L422xx)
#define FLASH_SIZE                         (((((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) == 0x0000FFFFU)) ? (0x80U << 10U) :  \
                                            (((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) << 10U))
#else
#define FLASH_SIZE                         (((((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU))== 0x0000FFFFU)) ? (0x400U << 10U) : \
                                            (((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0000FFFFU)) << 10U))
#endif

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_BANK_SIZE                    (FLASH_SIZE >> 1U)
#else
#define FLASH_BANK_SIZE                    (FLASH_SIZE)
#endif

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_PAGE_SIZE                    ((uint32_t)0x1000)
#define FLASH_PAGE_SIZE_128_BITS           ((uint32_t)0x2000)
#else
#define FLASH_PAGE_SIZE                    ((uint32_t)0x800)
#endif

#define FLASH_TIMEOUT_VALUE                ((uint32_t)50000)/* 50 s */
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup FLASH_Private_Macros FLASH Private Macros
 *  @{
 */

#define IS_FLASH_TYPEERASE(VALUE)          (((VALUE) == FLASH_TYPEERASE_PAGES) || \
                                            ((VALUE) == FLASH_TYPEERASE_MASSERASE))

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_FLASH_BANK(BANK)                (((BANK) == FLASH_BANK_1)  || \
                                            ((BANK) == FLASH_BANK_2)  || \
                                            ((BANK) == FLASH_BANK_BOTH))

#define IS_FLASH_BANK_EXCLUSIVE(BANK)      (((BANK) == FLASH_BANK_1)  || \
                                            ((BANK) == FLASH_BANK_2))
#else
#define IS_FLASH_BANK(BANK)                ((BANK) == FLASH_BANK_1)

#define IS_FLASH_BANK_EXCLUSIVE(BANK)      ((BANK) == FLASH_BANK_1)
#endif

#define IS_FLASH_TYPEPROGRAM(VALUE)        (((VALUE) == FLASH_TYPEPROGRAM_DOUBLEWORD) || \
                                            ((VALUE) == FLASH_TYPEPROGRAM_FAST) || \
                                            ((VALUE) == FLASH_TYPEPROGRAM_FAST_AND_LAST))

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_FLASH_MAIN_MEM_ADDRESS(ADDRESS) (((ADDRESS) >= (FLASH_BASE)) && ((ADDRESS) <= (FLASH_BASE+0x1FFFFFU)))
#else
#define IS_FLASH_MAIN_MEM_ADDRESS(ADDRESS) (((ADDRESS) >= (FLASH_BASE))         && ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x400U) ? \
                                            ((ADDRESS) <= (FLASH_BASE+0xFFFFFU)) : ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x200U) ? \
                                            ((ADDRESS) <= (FLASH_BASE+0x7FFFFU)) : ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x100U) ? \
                                            ((ADDRESS) <= (FLASH_BASE+0x3FFFFU)) : ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x80U) ? \
                                            ((ADDRESS) <= (FLASH_BASE+0x1FFFFU)) : ((ADDRESS) <= (FLASH_BASE+0xFFFFFU)))))))
#endif

#define IS_FLASH_OTP_ADDRESS(ADDRESS)      (((ADDRESS) >= 0x1FFF7000U) && ((ADDRESS) <= 0x1FFF73FFU))

#define IS_FLASH_PROGRAM_ADDRESS(ADDRESS)  ((IS_FLASH_MAIN_MEM_ADDRESS(ADDRESS)) || (IS_FLASH_OTP_ADDRESS(ADDRESS)))

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_FLASH_PAGE(PAGE)                ((PAGE) < 256U)
#elif defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || defined(STM32L496xx) || defined(STM32L4A6xx)
#define IS_FLASH_PAGE(PAGE)                (((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x400U) ? ((PAGE) < 256U) : \
                                            ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x200U) ? ((PAGE) < 128U) : \
                                            ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x100U) ? ((PAGE) < 64U)  : \
                                            ((PAGE) < 256U)))))
#elif defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx)
#define IS_FLASH_PAGE(PAGE)                (((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x200U) ? ((PAGE) < 256U) : \
                                            ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x100U) ? ((PAGE) < 128U) : \
                                            ((PAGE) < 256U))))
#else
#define IS_FLASH_PAGE(PAGE)                (((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x100U) ? ((PAGE) < 128U) : \
                                            ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) & (0x0FFFU)) == 0x80U)  ? ((PAGE) < 64U)  : \
                                            ((PAGE) < 128U))))
#endif

#define IS_OPTIONBYTE(VALUE)               (((VALUE) <= (OPTIONBYTE_WRP | OPTIONBYTE_RDP | OPTIONBYTE_USER | OPTIONBYTE_PCROP)))

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_OB_WRPAREA(VALUE)               (((VALUE) == OB_WRPAREA_BANK1_AREAA) || ((VALUE) == OB_WRPAREA_BANK1_AREAB) || \
                                            ((VALUE) == OB_WRPAREA_BANK2_AREAA) || ((VALUE) == OB_WRPAREA_BANK2_AREAB))
#else
#define IS_OB_WRPAREA(VALUE)               (((VALUE) == OB_WRPAREA_BANK1_AREAA) || ((VALUE) == OB_WRPAREA_BANK1_AREAB))
#endif

#define IS_OB_RDP_LEVEL(LEVEL)             (((LEVEL) == OB_RDP_LEVEL_0)   ||\
                                            ((LEVEL) == OB_RDP_LEVEL_1)/* ||\
                                            ((LEVEL) == OB_RDP_LEVEL_2)*/)

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_OB_USER_TYPE(TYPE)              (((TYPE) <= (uint32_t)0xFFFFU) && ((TYPE) != 0U))
#elif defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || defined(STM32L496xx) || defined(STM32L4A6xx)
#define IS_OB_USER_TYPE(TYPE)              (((TYPE) <= (uint32_t)0x1FFFU) && ((TYPE) != 0U))
#else
#define IS_OB_USER_TYPE(TYPE)              (((TYPE) <= (uint32_t)0x7E7FU) && ((TYPE) != 0U) && (((TYPE)&0x0180U) == 0U))
#endif

#define IS_OB_USER_BOR_LEVEL(LEVEL)        (((LEVEL) == OB_BOR_LEVEL_0) || ((LEVEL) == OB_BOR_LEVEL_1) || \
                                            ((LEVEL) == OB_BOR_LEVEL_2) || ((LEVEL) == OB_BOR_LEVEL_3) || \
                                            ((LEVEL) == OB_BOR_LEVEL_4))

#define IS_OB_USER_STOP(VALUE)             (((VALUE) == OB_STOP_RST) || ((VALUE) == OB_STOP_NORST))

#define IS_OB_USER_STANDBY(VALUE)          (((VALUE) == OB_STANDBY_RST) || ((VALUE) == OB_STANDBY_NORST))

#define IS_OB_USER_SHUTDOWN(VALUE)         (((VALUE) == OB_SHUTDOWN_RST) || ((VALUE) == OB_SHUTDOWN_NORST))

#define IS_OB_USER_IWDG(VALUE)             (((VALUE) == OB_IWDG_HW) || ((VALUE) == OB_IWDG_SW))

#define IS_OB_USER_IWDG_STOP(VALUE)        (((VALUE) == OB_IWDG_STOP_FREEZE) || ((VALUE) == OB_IWDG_STOP_RUN))

#define IS_OB_USER_IWDG_STDBY(VALUE)       (((VALUE) == OB_IWDG_STDBY_FREEZE) || ((VALUE) == OB_IWDG_STDBY_RUN))

#define IS_OB_USER_WWDG(VALUE)             (((VALUE) == OB_WWDG_HW) || ((VALUE) == OB_WWDG_SW))

#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_OB_USER_BFB2(VALUE)             (((VALUE) == OB_BFB2_DISABLE) || ((VALUE) == OB_BFB2_ENABLE))

#define IS_OB_USER_DUALBANK(VALUE)         (((VALUE) == OB_DUALBANK_SINGLE) || ((VALUE) == OB_DUALBANK_DUAL))
#endif

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_OB_USER_DBANK(VALUE)            (((VALUE) == OB_DBANK_128_BITS) || ((VALUE) == OB_DBANK_64_BITS))
#endif

#define IS_OB_USER_BOOT1(VALUE)            (((VALUE) == OB_BOOT1_SRAM) || ((VALUE) == OB_BOOT1_SYSTEM))

#define IS_OB_USER_SRAM2_PARITY(VALUE)     (((VALUE) == OB_SRAM2_PARITY_ENABLE) || ((VALUE) == OB_SRAM2_PARITY_DISABLE))

#define IS_OB_USER_SRAM2_RST(VALUE)        (((VALUE) == OB_SRAM2_RST_ERASE) || ((VALUE) == OB_SRAM2_RST_NOT_ERASE))

#if defined (STM32L412xx) || defined (STM32L422xx) || defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || \
    defined (STM32L442xx) || defined (STM32L443xx) || defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_OB_USER_SWBOOT0(VALUE)          (((VALUE) == OB_BOOT0_FROM_OB) || ((VALUE) == OB_BOOT0_FROM_PIN))

#define IS_OB_USER_BOOT0(VALUE)            (((VALUE) == OB_BOOT0_RESET) || ((VALUE) == OB_BOOT0_SET))
#endif

#define IS_OB_PCROP_RDP(VALUE)             (((VALUE) == OB_PCROP_RDP_NOT_ERASE) || ((VALUE) == OB_PCROP_RDP_ERASE))

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define IS_FLASH_LATENCY(LATENCY)          (((LATENCY) == FLASH_LATENCY_0) || ((LATENCY) == FLASH_LATENCY_1) || \
                                            ((LATENCY) == FLASH_LATENCY_2) || ((LATENCY) == FLASH_LATENCY_3) || \
                                            ((LATENCY) == FLASH_LATENCY_4) || ((LATENCY) == FLASH_LATENCY_5) || \
                                            ((LATENCY) == FLASH_LATENCY_6) || ((LATENCY) == FLASH_LATENCY_7) || \
                                            ((LATENCY) == FLASH_LATENCY_8) || ((LATENCY) == FLASH_LATENCY_9) || \
                                            ((LATENCY) == FLASH_LATENCY_10) || ((LATENCY) == FLASH_LATENCY_11) || \
                                            ((LATENCY) == FLASH_LATENCY_12) || ((LATENCY) == FLASH_LATENCY_13) || \
                                            ((LATENCY) == FLASH_LATENCY_14) || ((LATENCY) == FLASH_LATENCY_15))
#else
#define IS_FLASH_LATENCY(LATENCY)          (((LATENCY) == FLASH_LATENCY_0) || \
                                            ((LATENCY) == FLASH_LATENCY_1) || \
                                            ((LATENCY) == FLASH_LATENCY_2) || \
                                            ((LATENCY) == FLASH_LATENCY_3) || \
                                            ((LATENCY) == FLASH_LATENCY_4))
#endif
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

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_HAL_FLASH_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
