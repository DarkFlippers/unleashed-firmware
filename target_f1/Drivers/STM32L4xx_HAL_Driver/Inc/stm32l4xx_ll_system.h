/**
  ******************************************************************************
  * @file    stm32l4xx_ll_system.h
  * @author  MCD Application Team
  * @brief   Header file of SYSTEM LL module.
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
    The LL SYSTEM driver contains a set of generic APIs that can be
    used by user:
      (+) Some of the FLASH features need to be handled in the SYSTEM file.
      (+) Access to DBGCMU registers
      (+) Access to SYSCFG registers
      (+) Access to VREFBUF registers

  @endverbatim
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
#ifndef STM32L4xx_LL_SYSTEM_H
#define STM32L4xx_LL_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"

/** @addtogroup STM32L4xx_LL_Driver
  * @{
  */

#if defined (FLASH) || defined (SYSCFG) || defined (DBGMCU) || defined (VREFBUF)

/** @defgroup SYSTEM_LL SYSTEM
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/
/** @defgroup SYSTEM_LL_Private_Constants SYSTEM Private Constants
  * @{
  */

/**
 * @brief Power-down in Run mode Flash key
 */
#define FLASH_PDKEY1                  0x04152637U /*!< Flash power down key1 */
#define FLASH_PDKEY2                  0xFAFBFCFDU /*!< Flash power down key2: used with FLASH_PDKEY1 
                                                       to unlock the RUN_PD bit in FLASH_ACR */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup SYSTEM_LL_Exported_Constants SYSTEM Exported Constants
  * @{
  */

/** @defgroup SYSTEM_LL_EC_REMAP SYSCFG REMAP
* @{
*/
#define LL_SYSCFG_REMAP_FLASH              0x00000000U                                           /*!< Main Flash memory mapped at 0x00000000              */
#define LL_SYSCFG_REMAP_SYSTEMFLASH        SYSCFG_MEMRMP_MEM_MODE_0                              /*!< System Flash memory mapped at 0x00000000            */
#define LL_SYSCFG_REMAP_SRAM               (SYSCFG_MEMRMP_MEM_MODE_1 | SYSCFG_MEMRMP_MEM_MODE_0) /*!< SRAM1 mapped at 0x00000000                          */
#if defined(FMC_Bank1_R)
#define LL_SYSCFG_REMAP_FMC                SYSCFG_MEMRMP_MEM_MODE_1                              /*!< FMC bank 1 (NOR/PSRAM 1 and 2) mapped at 0x00000000 */
#endif /* FMC_Bank1_R */
#define LL_SYSCFG_REMAP_QUADSPI            (SYSCFG_MEMRMP_MEM_MODE_2 | SYSCFG_MEMRMP_MEM_MODE_1) /*!< QUADSPI memory mapped at 0x00000000                 */
/**
  * @}
  */

#if defined(SYSCFG_MEMRMP_FB_MODE)
/** @defgroup SYSTEM_LL_EC_BANKMODE SYSCFG BANK MODE
  * @{
  */
#define LL_SYSCFG_BANKMODE_BANK1           0x00000000U               /*!< Flash Bank1 mapped at 0x08000000 (and aliased @0x00000000) 
                                                                      and Flash Bank2 mapped at 0x08080000 (and aliased at 0x00080000) */
#define LL_SYSCFG_BANKMODE_BANK2           SYSCFG_MEMRMP_FB_MODE     /*!< Flash Bank2 mapped at 0x08000000 (and aliased @0x00000000) 
                                                                      and Flash Bank1 mapped at 0x08080000 (and aliased at 0x00080000) */
/**
  * @}
  */

#endif /* SYSCFG_MEMRMP_FB_MODE */
/** @defgroup SYSTEM_LL_EC_I2C_FASTMODEPLUS SYSCFG I2C FASTMODEPLUS
  * @{
  */
#define LL_SYSCFG_I2C_FASTMODEPLUS_PB6     SYSCFG_CFGR1_I2C_PB6_FMP  /*!< Enable Fast Mode Plus on PB6       */
#define LL_SYSCFG_I2C_FASTMODEPLUS_PB7     SYSCFG_CFGR1_I2C_PB7_FMP  /*!< Enable Fast Mode Plus on PB7       */
#if defined(SYSCFG_CFGR1_I2C_PB8_FMP)
#define LL_SYSCFG_I2C_FASTMODEPLUS_PB8     SYSCFG_CFGR1_I2C_PB8_FMP  /*!< Enable Fast Mode Plus on PB8       */
#endif /* SYSCFG_CFGR1_I2C_PB8_FMP */
#if defined(SYSCFG_CFGR1_I2C_PB9_FMP)
#define LL_SYSCFG_I2C_FASTMODEPLUS_PB9     SYSCFG_CFGR1_I2C_PB9_FMP  /*!< Enable Fast Mode Plus on PB9       */
#endif /* SYSCFG_CFGR1_I2C_PB9_FMP */
#define LL_SYSCFG_I2C_FASTMODEPLUS_I2C1    SYSCFG_CFGR1_I2C1_FMP     /*!< Enable Fast Mode Plus on I2C1 pins */
#if defined(I2C2)
#define LL_SYSCFG_I2C_FASTMODEPLUS_I2C2    SYSCFG_CFGR1_I2C2_FMP     /*!< Enable Fast Mode Plus on I2C2 pins */
#endif /* I2C2 */
#define LL_SYSCFG_I2C_FASTMODEPLUS_I2C3    SYSCFG_CFGR1_I2C3_FMP     /*!< Enable Fast Mode Plus on I2C3 pins */
#if defined(I2C4)
#define LL_SYSCFG_I2C_FASTMODEPLUS_I2C4    SYSCFG_CFGR1_I2C4_FMP     /*!< Enable Fast Mode Plus on I2C4 pins */
#endif /* I2C4 */
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_EXTI_PORT SYSCFG EXTI PORT
  * @{
  */
#define LL_SYSCFG_EXTI_PORTA               0U                        /*!< EXTI PORT A                        */
#define LL_SYSCFG_EXTI_PORTB               1U                        /*!< EXTI PORT B                        */
#define LL_SYSCFG_EXTI_PORTC               2U                        /*!< EXTI PORT C                        */
#define LL_SYSCFG_EXTI_PORTD               3U                        /*!< EXTI PORT D                        */
#define LL_SYSCFG_EXTI_PORTE               4U                        /*!< EXTI PORT E                        */
#if defined(GPIOF)
#define LL_SYSCFG_EXTI_PORTF               5U                        /*!< EXTI PORT F                        */
#endif /* GPIOF */
#if defined(GPIOG)
#define LL_SYSCFG_EXTI_PORTG               6U                        /*!< EXTI PORT G                        */
#endif /* GPIOG */
#define LL_SYSCFG_EXTI_PORTH               7U                        /*!< EXTI PORT H                        */
#if defined(GPIOI)
#define LL_SYSCFG_EXTI_PORTI               8U                        /*!< EXTI PORT I                        */
#endif /* GPIOI */
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_EXTI_LINE SYSCFG EXTI LINE
  * @{
  */
#define LL_SYSCFG_EXTI_LINE0               (uint32_t)(0x000FU << 16U | 0U)  /* !< EXTI_POSITION_0  | EXTICR[0] */
#define LL_SYSCFG_EXTI_LINE1               (uint32_t)(0x00F0U << 16U | 0U)  /* !< EXTI_POSITION_4  | EXTICR[0] */
#define LL_SYSCFG_EXTI_LINE2               (uint32_t)(0x0F00U << 16U | 0U)  /* !< EXTI_POSITION_8  | EXTICR[0] */
#define LL_SYSCFG_EXTI_LINE3               (uint32_t)(0xF000U << 16U | 0U)  /* !< EXTI_POSITION_12 | EXTICR[0] */
#define LL_SYSCFG_EXTI_LINE4               (uint32_t)(0x000FU << 16U | 1U)  /* !< EXTI_POSITION_0  | EXTICR[1] */
#define LL_SYSCFG_EXTI_LINE5               (uint32_t)(0x00F0U << 16U | 1U)  /* !< EXTI_POSITION_4  | EXTICR[1] */
#define LL_SYSCFG_EXTI_LINE6               (uint32_t)(0x0F00U << 16U | 1U)  /* !< EXTI_POSITION_8  | EXTICR[1] */
#define LL_SYSCFG_EXTI_LINE7               (uint32_t)(0xF000U << 16U | 1U)  /* !< EXTI_POSITION_12 | EXTICR[1] */
#define LL_SYSCFG_EXTI_LINE8               (uint32_t)(0x000FU << 16U | 2U)  /* !< EXTI_POSITION_0  | EXTICR[2] */
#define LL_SYSCFG_EXTI_LINE9               (uint32_t)(0x00F0U << 16U | 2U)  /* !< EXTI_POSITION_4  | EXTICR[2] */
#define LL_SYSCFG_EXTI_LINE10              (uint32_t)(0x0F00U << 16U | 2U)  /* !< EXTI_POSITION_8  | EXTICR[2] */
#define LL_SYSCFG_EXTI_LINE11              (uint32_t)(0xF000U << 16U | 2U)  /* !< EXTI_POSITION_12 | EXTICR[2] */
#define LL_SYSCFG_EXTI_LINE12              (uint32_t)(0x000FU << 16U | 3U)  /* !< EXTI_POSITION_0  | EXTICR[3] */
#define LL_SYSCFG_EXTI_LINE13              (uint32_t)(0x00F0U << 16U | 3U)  /* !< EXTI_POSITION_4  | EXTICR[3] */
#define LL_SYSCFG_EXTI_LINE14              (uint32_t)(0x0F00U << 16U | 3U)  /* !< EXTI_POSITION_8  | EXTICR[3] */
#define LL_SYSCFG_EXTI_LINE15              (uint32_t)(0xF000U << 16U | 3U)  /* !< EXTI_POSITION_12 | EXTICR[3] */
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_TIMBREAK SYSCFG TIMER BREAK
  * @{
  */
#define LL_SYSCFG_TIMBREAK_ECC             SYSCFG_CFGR2_ECCL  /*!< Enables and locks the ECC error signal 
                                                                   with Break Input of TIM1/8/15/16/17                           */
#define LL_SYSCFG_TIMBREAK_PVD             SYSCFG_CFGR2_PVDL  /*!< Enables and locks the PVD connection 
                                                                   with TIM1/8/15/16/17 Break Input 
                                                                   and also the PVDE and PLS bits of the Power Control Interface */
#define LL_SYSCFG_TIMBREAK_SRAM2_PARITY    SYSCFG_CFGR2_SPL   /*!< Enables and locks the SRAM2_PARITY error signal 
                                                                   with Break Input of TIM1/8/15/16/17                           */
#define LL_SYSCFG_TIMBREAK_LOCKUP          SYSCFG_CFGR2_CLL   /*!< Enables and locks the LOCKUP output of CortexM4 
                                                                   with Break Input of TIM1/15/16/17                             */
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_SRAM2WRP SYSCFG SRAM2 WRP
  * @{
  */
#define LL_SYSCFG_SRAM2WRP_PAGE0           SYSCFG_SWPR_PAGE0  /*!< SRAM2 Write protection page 0  */
#define LL_SYSCFG_SRAM2WRP_PAGE1           SYSCFG_SWPR_PAGE1  /*!< SRAM2 Write protection page 1  */
#define LL_SYSCFG_SRAM2WRP_PAGE2           SYSCFG_SWPR_PAGE2  /*!< SRAM2 Write protection page 2  */
#define LL_SYSCFG_SRAM2WRP_PAGE3           SYSCFG_SWPR_PAGE3  /*!< SRAM2 Write protection page 3  */
#define LL_SYSCFG_SRAM2WRP_PAGE4           SYSCFG_SWPR_PAGE4  /*!< SRAM2 Write protection page 4  */
#define LL_SYSCFG_SRAM2WRP_PAGE5           SYSCFG_SWPR_PAGE5  /*!< SRAM2 Write protection page 5  */
#define LL_SYSCFG_SRAM2WRP_PAGE6           SYSCFG_SWPR_PAGE6  /*!< SRAM2 Write protection page 6  */
#define LL_SYSCFG_SRAM2WRP_PAGE7           SYSCFG_SWPR_PAGE7  /*!< SRAM2 Write protection page 7  */
#define LL_SYSCFG_SRAM2WRP_PAGE8           SYSCFG_SWPR_PAGE8  /*!< SRAM2 Write protection page 8  */
#define LL_SYSCFG_SRAM2WRP_PAGE9           SYSCFG_SWPR_PAGE9  /*!< SRAM2 Write protection page 9  */
#define LL_SYSCFG_SRAM2WRP_PAGE10          SYSCFG_SWPR_PAGE10 /*!< SRAM2 Write protection page 10 */
#define LL_SYSCFG_SRAM2WRP_PAGE11          SYSCFG_SWPR_PAGE11 /*!< SRAM2 Write protection page 11 */
#define LL_SYSCFG_SRAM2WRP_PAGE12          SYSCFG_SWPR_PAGE12 /*!< SRAM2 Write protection page 12 */
#define LL_SYSCFG_SRAM2WRP_PAGE13          SYSCFG_SWPR_PAGE13 /*!< SRAM2 Write protection page 13 */
#define LL_SYSCFG_SRAM2WRP_PAGE14          SYSCFG_SWPR_PAGE14 /*!< SRAM2 Write protection page 14 */
#define LL_SYSCFG_SRAM2WRP_PAGE15          SYSCFG_SWPR_PAGE15 /*!< SRAM2 Write protection page 15 */
#if defined(SYSCFG_SWPR_PAGE31)
#define LL_SYSCFG_SRAM2WRP_PAGE16          SYSCFG_SWPR_PAGE16 /*!< SRAM2 Write protection page 16 */
#define LL_SYSCFG_SRAM2WRP_PAGE17          SYSCFG_SWPR_PAGE17 /*!< SRAM2 Write protection page 17 */
#define LL_SYSCFG_SRAM2WRP_PAGE18          SYSCFG_SWPR_PAGE18 /*!< SRAM2 Write protection page 18 */
#define LL_SYSCFG_SRAM2WRP_PAGE19          SYSCFG_SWPR_PAGE19 /*!< SRAM2 Write protection page 19 */
#define LL_SYSCFG_SRAM2WRP_PAGE20          SYSCFG_SWPR_PAGE20 /*!< SRAM2 Write protection page 20 */
#define LL_SYSCFG_SRAM2WRP_PAGE21          SYSCFG_SWPR_PAGE21 /*!< SRAM2 Write protection page 21 */
#define LL_SYSCFG_SRAM2WRP_PAGE22          SYSCFG_SWPR_PAGE22 /*!< SRAM2 Write protection page 22 */
#define LL_SYSCFG_SRAM2WRP_PAGE23          SYSCFG_SWPR_PAGE23 /*!< SRAM2 Write protection page 23 */
#define LL_SYSCFG_SRAM2WRP_PAGE24          SYSCFG_SWPR_PAGE24 /*!< SRAM2 Write protection page 24 */
#define LL_SYSCFG_SRAM2WRP_PAGE25          SYSCFG_SWPR_PAGE25 /*!< SRAM2 Write protection page 25 */
#define LL_SYSCFG_SRAM2WRP_PAGE26          SYSCFG_SWPR_PAGE26 /*!< SRAM2 Write protection page 26 */
#define LL_SYSCFG_SRAM2WRP_PAGE27          SYSCFG_SWPR_PAGE27 /*!< SRAM2 Write protection page 27 */
#define LL_SYSCFG_SRAM2WRP_PAGE28          SYSCFG_SWPR_PAGE28 /*!< SRAM2 Write protection page 28 */
#define LL_SYSCFG_SRAM2WRP_PAGE29          SYSCFG_SWPR_PAGE29 /*!< SRAM2 Write protection page 29 */
#define LL_SYSCFG_SRAM2WRP_PAGE30          SYSCFG_SWPR_PAGE30 /*!< SRAM2 Write protection page 30 */
#define LL_SYSCFG_SRAM2WRP_PAGE31          SYSCFG_SWPR_PAGE31 /*!< SRAM2 Write protection page 31 */
#endif /* SYSCFG_SWPR_PAGE31 */
#if defined(SYSCFG_SWPR2_PAGE63)
#define LL_SYSCFG_SRAM2WRP_PAGE32          SYSCFG_SWPR2_PAGE32 /*!< SRAM2 Write protection page 32 */
#define LL_SYSCFG_SRAM2WRP_PAGE33          SYSCFG_SWPR2_PAGE33 /*!< SRAM2 Write protection page 33 */
#define LL_SYSCFG_SRAM2WRP_PAGE34          SYSCFG_SWPR2_PAGE34 /*!< SRAM2 Write protection page 34 */
#define LL_SYSCFG_SRAM2WRP_PAGE35          SYSCFG_SWPR2_PAGE35 /*!< SRAM2 Write protection page 35 */
#define LL_SYSCFG_SRAM2WRP_PAGE36          SYSCFG_SWPR2_PAGE36 /*!< SRAM2 Write protection page 36 */
#define LL_SYSCFG_SRAM2WRP_PAGE37          SYSCFG_SWPR2_PAGE37 /*!< SRAM2 Write protection page 37 */
#define LL_SYSCFG_SRAM2WRP_PAGE38          SYSCFG_SWPR2_PAGE38 /*!< SRAM2 Write protection page 38 */
#define LL_SYSCFG_SRAM2WRP_PAGE39          SYSCFG_SWPR2_PAGE39 /*!< SRAM2 Write protection page 39 */
#define LL_SYSCFG_SRAM2WRP_PAGE40          SYSCFG_SWPR2_PAGE40 /*!< SRAM2 Write protection page 40 */
#define LL_SYSCFG_SRAM2WRP_PAGE41          SYSCFG_SWPR2_PAGE41 /*!< SRAM2 Write protection page 41 */
#define LL_SYSCFG_SRAM2WRP_PAGE42          SYSCFG_SWPR2_PAGE42 /*!< SRAM2 Write protection page 42 */
#define LL_SYSCFG_SRAM2WRP_PAGE43          SYSCFG_SWPR2_PAGE43 /*!< SRAM2 Write protection page 43 */
#define LL_SYSCFG_SRAM2WRP_PAGE44          SYSCFG_SWPR2_PAGE44 /*!< SRAM2 Write protection page 44 */
#define LL_SYSCFG_SRAM2WRP_PAGE45          SYSCFG_SWPR2_PAGE45 /*!< SRAM2 Write protection page 45 */
#define LL_SYSCFG_SRAM2WRP_PAGE46          SYSCFG_SWPR2_PAGE46 /*!< SRAM2 Write protection page 46 */
#define LL_SYSCFG_SRAM2WRP_PAGE47          SYSCFG_SWPR2_PAGE47 /*!< SRAM2 Write protection page 47 */
#define LL_SYSCFG_SRAM2WRP_PAGE48          SYSCFG_SWPR2_PAGE48 /*!< SRAM2 Write protection page 48 */
#define LL_SYSCFG_SRAM2WRP_PAGE49          SYSCFG_SWPR2_PAGE49 /*!< SRAM2 Write protection page 49 */
#define LL_SYSCFG_SRAM2WRP_PAGE50          SYSCFG_SWPR2_PAGE50 /*!< SRAM2 Write protection page 50 */
#define LL_SYSCFG_SRAM2WRP_PAGE51          SYSCFG_SWPR2_PAGE51 /*!< SRAM2 Write protection page 51 */
#define LL_SYSCFG_SRAM2WRP_PAGE52          SYSCFG_SWPR2_PAGE52 /*!< SRAM2 Write protection page 52 */
#define LL_SYSCFG_SRAM2WRP_PAGE53          SYSCFG_SWPR2_PAGE53 /*!< SRAM2 Write protection page 53 */
#define LL_SYSCFG_SRAM2WRP_PAGE54          SYSCFG_SWPR2_PAGE54 /*!< SRAM2 Write protection page 54 */
#define LL_SYSCFG_SRAM2WRP_PAGE55          SYSCFG_SWPR2_PAGE55 /*!< SRAM2 Write protection page 55 */
#define LL_SYSCFG_SRAM2WRP_PAGE56          SYSCFG_SWPR2_PAGE56 /*!< SRAM2 Write protection page 56 */
#define LL_SYSCFG_SRAM2WRP_PAGE57          SYSCFG_SWPR2_PAGE57 /*!< SRAM2 Write protection page 57 */
#define LL_SYSCFG_SRAM2WRP_PAGE58          SYSCFG_SWPR2_PAGE58 /*!< SRAM2 Write protection page 58 */
#define LL_SYSCFG_SRAM2WRP_PAGE59          SYSCFG_SWPR2_PAGE59 /*!< SRAM2 Write protection page 59 */
#define LL_SYSCFG_SRAM2WRP_PAGE60          SYSCFG_SWPR2_PAGE60 /*!< SRAM2 Write protection page 60 */
#define LL_SYSCFG_SRAM2WRP_PAGE61          SYSCFG_SWPR2_PAGE61 /*!< SRAM2 Write protection page 61 */
#define LL_SYSCFG_SRAM2WRP_PAGE62          SYSCFG_SWPR2_PAGE62 /*!< SRAM2 Write protection page 62 */
#define LL_SYSCFG_SRAM2WRP_PAGE63          SYSCFG_SWPR2_PAGE63 /*!< SRAM2 Write protection page 63 */
#endif /* SYSCFG_SWPR2_PAGE63 */
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_TRACE DBGMCU TRACE Pin Assignment
  * @{
  */
#define LL_DBGMCU_TRACE_NONE               0x00000000U                                     /*!< TRACE pins not assigned (default state) */
#define LL_DBGMCU_TRACE_ASYNCH             DBGMCU_CR_TRACE_IOEN                            /*!< TRACE pin assignment for Asynchronous Mode */
#define LL_DBGMCU_TRACE_SYNCH_SIZE1        (DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE_0) /*!< TRACE pin assignment for Synchronous Mode with a TRACEDATA size of 1 */
#define LL_DBGMCU_TRACE_SYNCH_SIZE2        (DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE_1) /*!< TRACE pin assignment for Synchronous Mode with a TRACEDATA size of 2 */
#define LL_DBGMCU_TRACE_SYNCH_SIZE4        (DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE)   /*!< TRACE pin assignment for Synchronous Mode with a TRACEDATA size of 4 */
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_APB1_GRP1_STOP_IP DBGMCU APB1 GRP1 STOP IP
  * @{
  */
#define LL_DBGMCU_APB1_GRP1_TIM2_STOP      DBGMCU_APB1FZR1_DBG_TIM2_STOP   /*!< The counter clock of TIM2 is stopped when the core is halted*/
#if defined(TIM3)
#define LL_DBGMCU_APB1_GRP1_TIM3_STOP      DBGMCU_APB1FZR1_DBG_TIM3_STOP   /*!< The counter clock of TIM3 is stopped when the core is halted*/
#endif /* TIM3 */
#if defined(TIM4)
#define LL_DBGMCU_APB1_GRP1_TIM4_STOP      DBGMCU_APB1FZR1_DBG_TIM4_STOP   /*!< The counter clock of TIM4 is stopped when the core is halted*/
#endif /* TIM4 */
#if defined(TIM5)
#define LL_DBGMCU_APB1_GRP1_TIM5_STOP      DBGMCU_APB1FZR1_DBG_TIM5_STOP   /*!< The counter clock of TIM5 is stopped when the core is halted*/
#endif /* TIM5 */
#define LL_DBGMCU_APB1_GRP1_TIM6_STOP      DBGMCU_APB1FZR1_DBG_TIM6_STOP   /*!< The counter clock of TIM6 is stopped when the core is halted*/
#if defined(TIM7)
#define LL_DBGMCU_APB1_GRP1_TIM7_STOP      DBGMCU_APB1FZR1_DBG_TIM7_STOP   /*!< The counter clock of TIM7 is stopped when the core is halted*/
#endif /* TIM7 */
#define LL_DBGMCU_APB1_GRP1_RTC_STOP       DBGMCU_APB1FZR1_DBG_RTC_STOP    /*!< The clock of the RTC counter is stopped when the core is halted*/
#define LL_DBGMCU_APB1_GRP1_WWDG_STOP      DBGMCU_APB1FZR1_DBG_WWDG_STOP   /*!< The window watchdog counter clock is stopped when the core is halted*/
#define LL_DBGMCU_APB1_GRP1_IWDG_STOP      DBGMCU_APB1FZR1_DBG_IWDG_STOP   /*!< The independent watchdog counter clock is stopped when the core is halted*/
#define LL_DBGMCU_APB1_GRP1_I2C1_STOP      DBGMCU_APB1FZR1_DBG_I2C1_STOP   /*!< The I2C1 SMBus timeout is frozen*/
#if defined(I2C2)
#define LL_DBGMCU_APB1_GRP1_I2C2_STOP      DBGMCU_APB1FZR1_DBG_I2C2_STOP   /*!< The I2C2 SMBus timeout is frozen*/
#endif /* I2C2 */
#define LL_DBGMCU_APB1_GRP1_I2C3_STOP      DBGMCU_APB1FZR1_DBG_I2C3_STOP   /*!< The I2C3 SMBus timeout is frozen*/
#define LL_DBGMCU_APB1_GRP1_CAN_STOP       DBGMCU_APB1FZR1_DBG_CAN_STOP    /*!< The bxCAN receive registers are frozen*/
#if defined(CAN2)
#define LL_DBGMCU_APB1_GRP1_CAN2_STOP      DBGMCU_APB1FZR1_DBG_CAN2_STOP   /*!< The bxCAN2 receive registers are frozen*/
#endif /* CAN2 */
#define LL_DBGMCU_APB1_GRP1_LPTIM1_STOP    DBGMCU_APB1FZR1_DBG_LPTIM1_STOP /*!< The counter clock of LPTIM1 is stopped when the core is halted*/
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_APB1_GRP2_STOP_IP DBGMCU APB1 GRP2 STOP IP
  * @{
  */
#if defined(I2C4)
#define LL_DBGMCU_APB1_GRP2_I2C4_STOP      DBGMCU_APB1FZR2_DBG_I2C4_STOP   /*!< The I2C4 SMBus timeout is frozen*/
#endif /* I2C4 */
#define LL_DBGMCU_APB1_GRP2_LPTIM2_STOP    DBGMCU_APB1FZR2_DBG_LPTIM2_STOP /*!< The counter clock of LPTIM2 is stopped when the core is halted*/
/**
  * @}
  */

/** @defgroup SYSTEM_LL_EC_APB2_GRP1_STOP_IP DBGMCU APB2 GRP1 STOP IP
  * @{
  */
#define LL_DBGMCU_APB2_GRP1_TIM1_STOP      DBGMCU_APB2FZ_DBG_TIM1_STOP     /*!< The counter clock of TIM1 is stopped when the core is halted*/
#if defined(TIM8)
#define LL_DBGMCU_APB2_GRP1_TIM8_STOP      DBGMCU_APB2FZ_DBG_TIM8_STOP     /*!< The counter clock of TIM8 is stopped when the core is halted*/
#endif /* TIM8 */
#define LL_DBGMCU_APB2_GRP1_TIM15_STOP     DBGMCU_APB2FZ_DBG_TIM15_STOP    /*!< The counter clock of TIM15 is stopped when the core is halted*/
#define LL_DBGMCU_APB2_GRP1_TIM16_STOP     DBGMCU_APB2FZ_DBG_TIM16_STOP    /*!< The counter clock of TIM16 is stopped when the core is halted*/
#if defined(TIM17)
#define LL_DBGMCU_APB2_GRP1_TIM17_STOP     DBGMCU_APB2FZ_DBG_TIM17_STOP    /*!< The counter clock of TIM17 is stopped when the core is halted*/
#endif /* TIM17 */
/**
  * @}
  */

#if defined(VREFBUF)
/** @defgroup SYSTEM_LL_EC_VOLTAGE VREFBUF VOLTAGE
  * @{
  */
#define LL_VREFBUF_VOLTAGE_SCALE0          ((uint32_t)0x00000000) /*!< Voltage reference scale 0 (VREF_OUT1) */
#define LL_VREFBUF_VOLTAGE_SCALE1          VREFBUF_CSR_VRS        /*!< Voltage reference scale 1 (VREF_OUT2) */
/**
  * @}
  */
#endif /* VREFBUF */

/** @defgroup SYSTEM_LL_EC_LATENCY FLASH LATENCY
  * @{
  */
#define LL_FLASH_LATENCY_0                 FLASH_ACR_LATENCY_0WS   /*!< FLASH Zero wait state */
#define LL_FLASH_LATENCY_1                 FLASH_ACR_LATENCY_1WS   /*!< FLASH One wait state */
#define LL_FLASH_LATENCY_2                 FLASH_ACR_LATENCY_2WS   /*!< FLASH Two wait states */
#define LL_FLASH_LATENCY_3                 FLASH_ACR_LATENCY_3WS   /*!< FLASH Three wait states */
#define LL_FLASH_LATENCY_4                 FLASH_ACR_LATENCY_4WS   /*!< FLASH Four wait states */
#if defined(FLASH_ACR_LATENCY_5WS)
#define LL_FLASH_LATENCY_5                 FLASH_ACR_LATENCY_5WS   /*!< FLASH five wait state */
#define LL_FLASH_LATENCY_6                 FLASH_ACR_LATENCY_6WS   /*!< FLASH six wait state */
#define LL_FLASH_LATENCY_7                 FLASH_ACR_LATENCY_7WS   /*!< FLASH seven wait states */
#define LL_FLASH_LATENCY_8                 FLASH_ACR_LATENCY_8WS   /*!< FLASH eight wait states */
#define LL_FLASH_LATENCY_9                 FLASH_ACR_LATENCY_9WS   /*!< FLASH nine wait states */
#define LL_FLASH_LATENCY_10                FLASH_ACR_LATENCY_10WS  /*!< FLASH ten wait states */
#define LL_FLASH_LATENCY_11                FLASH_ACR_LATENCY_11WS  /*!< FLASH eleven wait states */
#define LL_FLASH_LATENCY_12                FLASH_ACR_LATENCY_12WS  /*!< FLASH twelve wait states */
#define LL_FLASH_LATENCY_13                FLASH_ACR_LATENCY_13WS  /*!< FLASH thirteen wait states */
#define LL_FLASH_LATENCY_14                FLASH_ACR_LATENCY_14WS  /*!< FLASH fourteen wait states */
#define LL_FLASH_LATENCY_15                FLASH_ACR_LATENCY_15WS  /*!< FLASH fifteen wait states */
#endif
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @defgroup SYSTEM_LL_Exported_Functions SYSTEM Exported Functions
  * @{
  */

/** @defgroup SYSTEM_LL_EF_SYSCFG SYSCFG
  * @{
  */

/**
  * @brief  Set memory mapping at address 0x00000000
  * @rmtoll SYSCFG_MEMRMP MEM_MODE      LL_SYSCFG_SetRemapMemory
  * @param  Memory This parameter can be one of the following values:
  *         @arg @ref LL_SYSCFG_REMAP_FLASH
  *         @arg @ref LL_SYSCFG_REMAP_SYSTEMFLASH
  *         @arg @ref LL_SYSCFG_REMAP_SRAM
  *         @arg @ref LL_SYSCFG_REMAP_FMC (*)
  *         @arg @ref LL_SYSCFG_REMAP_QUADSPI
  *
  *         (*) value not defined in all devices
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_SetRemapMemory(uint32_t Memory)
{
  MODIFY_REG(SYSCFG->MEMRMP, SYSCFG_MEMRMP_MEM_MODE, Memory);
}

/**
  * @brief  Get memory mapping at address 0x00000000
  * @rmtoll SYSCFG_MEMRMP MEM_MODE      LL_SYSCFG_GetRemapMemory
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_SYSCFG_REMAP_FLASH
  *         @arg @ref LL_SYSCFG_REMAP_SYSTEMFLASH
  *         @arg @ref LL_SYSCFG_REMAP_SRAM
  *         @arg @ref LL_SYSCFG_REMAP_FMC (*)
  *         @arg @ref LL_SYSCFG_REMAP_QUADSPI
  *
  *         (*) value not defined in all devices
  */
__STATIC_INLINE uint32_t LL_SYSCFG_GetRemapMemory(void)
{
  return (uint32_t)(READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_MEM_MODE));
}

#if defined(SYSCFG_MEMRMP_FB_MODE)
/**
  * @brief  Select Flash bank mode (Bank flashed at 0x08000000)
  * @rmtoll SYSCFG_MEMRMP FB_MODE       LL_SYSCFG_SetFlashBankMode
  * @param  Bank This parameter can be one of the following values:
  *         @arg @ref LL_SYSCFG_BANKMODE_BANK1
  *         @arg @ref LL_SYSCFG_BANKMODE_BANK2
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_SetFlashBankMode(uint32_t Bank)
{
  MODIFY_REG(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE, Bank);
}

/**
  * @brief  Get Flash bank mode (Bank flashed at 0x08000000)
  * @rmtoll SYSCFG_MEMRMP FB_MODE       LL_SYSCFG_GetFlashBankMode
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_SYSCFG_BANKMODE_BANK1
  *         @arg @ref LL_SYSCFG_BANKMODE_BANK2
  */
__STATIC_INLINE uint32_t LL_SYSCFG_GetFlashBankMode(void)
{
  return (uint32_t)(READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE));
}
#endif /* SYSCFG_MEMRMP_FB_MODE */

/**
  * @brief  Firewall protection enabled
  * @rmtoll SYSCFG_CFGR1 FWDIS         LL_SYSCFG_EnableFirewall
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableFirewall(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FWDIS);
}

/**
  * @brief  Check if Firewall protection is enabled or not
  * @rmtoll SYSCFG_CFGR1 FWDIS         LL_SYSCFG_IsEnabledFirewall
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsEnabledFirewall(void)
{
  return !(READ_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FWDIS) == SYSCFG_CFGR1_FWDIS);
}

/**
  * @brief  Enable I/O analog switch voltage booster.
  * @note   When voltage booster is enabled, I/O analog switches are supplied
  *         by a dedicated voltage booster, from VDD power domain. This is
  *         the recommended configuration with low VDDA voltage operation.
  * @note   The I/O analog switch voltage booster is relevant for peripherals
  *         using I/O in analog input: ADC, COMP, OPAMP.
  *         However, COMP and OPAMP inputs have a high impedance and
  *         voltage booster do not impact performance significantly.
  *         Therefore, the voltage booster is mainly intended for
  *         usage with ADC.
  * @rmtoll SYSCFG_CFGR1 BOOSTEN       LL_SYSCFG_EnableAnalogBooster
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableAnalogBooster(void)
{
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_BOOSTEN);
}

/**
  * @brief  Disable I/O analog switch voltage booster.
  * @note   When voltage booster is enabled, I/O analog switches are supplied
  *         by a dedicated voltage booster, from VDD power domain. This is
  *         the recommended configuration with low VDDA voltage operation.
  * @note   The I/O analog switch voltage booster is relevant for peripherals
  *         using I/O in analog input: ADC, COMP, OPAMP.
  *         However, COMP and OPAMP inputs have a high impedance and
  *         voltage booster do not impact performance significantly.
  *         Therefore, the voltage booster is mainly intended for
  *         usage with ADC.
  * @rmtoll SYSCFG_CFGR1 BOOSTEN       LL_SYSCFG_DisableAnalogBooster
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableAnalogBooster(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_BOOSTEN);
}

/**
  * @brief  Enable the I2C fast mode plus driving capability.
  * @rmtoll SYSCFG_CFGR1 I2C_PBx_FMP   LL_SYSCFG_EnableFastModePlus\n
  *         SYSCFG_CFGR1 I2Cx_FMP      LL_SYSCFG_EnableFastModePlus
  * @param  ConfigFastModePlus This parameter can be a combination of the following values:
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB6
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB7
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB8 (*)
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB9 (*)
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C1
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C2 (*)
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C3
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C4 (*)
  *
  *         (*) value not defined in all devices
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableFastModePlus(uint32_t ConfigFastModePlus)
{
  SET_BIT(SYSCFG->CFGR1, ConfigFastModePlus);
}

/**
  * @brief  Disable the I2C fast mode plus driving capability.
  * @rmtoll SYSCFG_CFGR1 I2C_PBx_FMP   LL_SYSCFG_DisableFastModePlus\n
  *         SYSCFG_CFGR1 I2Cx_FMP      LL_SYSCFG_DisableFastModePlus
  * @param  ConfigFastModePlus This parameter can be a combination of the following values:
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB6
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB7
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB8 (*)
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_PB9 (*)
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C1
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C2 (*)
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C3
  *         @arg @ref LL_SYSCFG_I2C_FASTMODEPLUS_I2C4 (*)
  *
  *         (*) value not defined in all devices
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableFastModePlus(uint32_t ConfigFastModePlus)
{
  CLEAR_BIT(SYSCFG->CFGR1, ConfigFastModePlus);
}

/**
  * @brief  Enable Floating Point Unit Invalid operation Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_0      LL_SYSCFG_EnableIT_FPU_IOC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableIT_FPU_IOC(void)
{
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_0);
}

/**
  * @brief  Enable Floating Point Unit Divide-by-zero Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_1      LL_SYSCFG_EnableIT_FPU_DZC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableIT_FPU_DZC(void)
{
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_1);
}

/**
  * @brief  Enable Floating Point Unit Underflow Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_2      LL_SYSCFG_EnableIT_FPU_UFC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableIT_FPU_UFC(void)
{
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_2);
}

/**
  * @brief  Enable Floating Point Unit Overflow Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_3      LL_SYSCFG_EnableIT_FPU_OFC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableIT_FPU_OFC(void)
{
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_3);
}

/**
  * @brief  Enable Floating Point Unit Input denormal Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_4      LL_SYSCFG_EnableIT_FPU_IDC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableIT_FPU_IDC(void)
{
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_4);
}

/**
  * @brief  Enable Floating Point Unit Inexact Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_5      LL_SYSCFG_EnableIT_FPU_IXC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableIT_FPU_IXC(void)
{
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_5);
}

/**
  * @brief  Disable Floating Point Unit Invalid operation Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_0      LL_SYSCFG_DisableIT_FPU_IOC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableIT_FPU_IOC(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_0);
}

/**
  * @brief  Disable Floating Point Unit Divide-by-zero Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_1      LL_SYSCFG_DisableIT_FPU_DZC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableIT_FPU_DZC(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_1);
}

/**
  * @brief  Disable Floating Point Unit Underflow Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_2      LL_SYSCFG_DisableIT_FPU_UFC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableIT_FPU_UFC(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_2);
}

/**
  * @brief  Disable Floating Point Unit Overflow Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_3      LL_SYSCFG_DisableIT_FPU_OFC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableIT_FPU_OFC(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_3);
}

/**
  * @brief  Disable Floating Point Unit Input denormal Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_4      LL_SYSCFG_DisableIT_FPU_IDC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableIT_FPU_IDC(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_4);
}

/**
  * @brief  Disable Floating Point Unit Inexact Interrupt
  * @rmtoll SYSCFG_CFGR1 FPU_IE_5      LL_SYSCFG_DisableIT_FPU_IXC
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_DisableIT_FPU_IXC(void)
{
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_5);
}

/**
  * @brief  Check if Floating Point Unit Invalid operation Interrupt source is enabled or disabled.
  * @rmtoll SYSCFG_CFGR1 FPU_IE_0      LL_SYSCFG_IsEnabledIT_FPU_IOC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsEnabledIT_FPU_IOC(void)
{
  return (READ_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_0) == (SYSCFG_CFGR1_FPU_IE_0));
}

/**
  * @brief  Check if Floating Point Unit Divide-by-zero Interrupt source is enabled or disabled.
  * @rmtoll SYSCFG_CFGR1 FPU_IE_1      LL_SYSCFG_IsEnabledIT_FPU_DZC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsEnabledIT_FPU_DZC(void)
{
  return (READ_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_1) == (SYSCFG_CFGR1_FPU_IE_1));
}

/**
  * @brief  Check if Floating Point Unit Underflow Interrupt source is enabled or disabled.
  * @rmtoll SYSCFG_CFGR1 FPU_IE_2      LL_SYSCFG_IsEnabledIT_FPU_UFC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsEnabledIT_FPU_UFC(void)
{
  return (READ_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_2) == (SYSCFG_CFGR1_FPU_IE_2));
}

/**
  * @brief  Check if Floating Point Unit Overflow Interrupt source is enabled or disabled.
  * @rmtoll SYSCFG_CFGR1 FPU_IE_3      LL_SYSCFG_IsEnabledIT_FPU_OFC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsEnabledIT_FPU_OFC(void)
{
  return (READ_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_3) == (SYSCFG_CFGR1_FPU_IE_3));
}

/**
  * @brief  Check if Floating Point Unit Input denormal Interrupt source is enabled or disabled.
  * @rmtoll SYSCFG_CFGR1 FPU_IE_4      LL_SYSCFG_IsEnabledIT_FPU_IDC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsEnabledIT_FPU_IDC(void)
{
  return (READ_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_4) == (SYSCFG_CFGR1_FPU_IE_4));
}

/**
  * @brief  Check if Floating Point Unit Inexact Interrupt source is enabled or disabled.
  * @rmtoll SYSCFG_CFGR1 FPU_IE_5      LL_SYSCFG_IsEnabledIT_FPU_IXC
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsEnabledIT_FPU_IXC(void)
{
  return (READ_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FPU_IE_5) == (SYSCFG_CFGR1_FPU_IE_5));
}

/**
  * @brief  Configure source input for the EXTI external interrupt.
  * @rmtoll SYSCFG_EXTICR1 EXTIx         LL_SYSCFG_SetEXTISource\n
  *         SYSCFG_EXTICR2 EXTIx         LL_SYSCFG_SetEXTISource\n
  *         SYSCFG_EXTICR3 EXTIx         LL_SYSCFG_SetEXTISource\n
  *         SYSCFG_EXTICR4 EXTIx         LL_SYSCFG_SetEXTISource
  * @param  Port This parameter can be one of the following values:
  *         @arg @ref LL_SYSCFG_EXTI_PORTA
  *         @arg @ref LL_SYSCFG_EXTI_PORTB
  *         @arg @ref LL_SYSCFG_EXTI_PORTC
  *         @arg @ref LL_SYSCFG_EXTI_PORTD
  *         @arg @ref LL_SYSCFG_EXTI_PORTE
  *         @arg @ref LL_SYSCFG_EXTI_PORTF (*)
  *         @arg @ref LL_SYSCFG_EXTI_PORTG (*)
  *         @arg @ref LL_SYSCFG_EXTI_PORTH
  *         @arg @ref LL_SYSCFG_EXTI_PORTI (*)
  *
  *         (*) value not defined in all devices
  * @param  Line This parameter can be one of the following values:
  *         @arg @ref LL_SYSCFG_EXTI_LINE0
  *         @arg @ref LL_SYSCFG_EXTI_LINE1
  *         @arg @ref LL_SYSCFG_EXTI_LINE2
  *         @arg @ref LL_SYSCFG_EXTI_LINE3
  *         @arg @ref LL_SYSCFG_EXTI_LINE4
  *         @arg @ref LL_SYSCFG_EXTI_LINE5
  *         @arg @ref LL_SYSCFG_EXTI_LINE6
  *         @arg @ref LL_SYSCFG_EXTI_LINE7
  *         @arg @ref LL_SYSCFG_EXTI_LINE8
  *         @arg @ref LL_SYSCFG_EXTI_LINE9
  *         @arg @ref LL_SYSCFG_EXTI_LINE10
  *         @arg @ref LL_SYSCFG_EXTI_LINE11
  *         @arg @ref LL_SYSCFG_EXTI_LINE12
  *         @arg @ref LL_SYSCFG_EXTI_LINE13
  *         @arg @ref LL_SYSCFG_EXTI_LINE14
  *         @arg @ref LL_SYSCFG_EXTI_LINE15
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_SetEXTISource(uint32_t Port, uint32_t Line)
{
  MODIFY_REG(SYSCFG->EXTICR[Line & 0xFFU], (Line >> 16U), Port << POSITION_VAL((Line >> 16U)));
}

/**
  * @brief  Get the configured defined for specific EXTI Line
  * @rmtoll SYSCFG_EXTICR1 EXTIx         LL_SYSCFG_GetEXTISource\n
  *         SYSCFG_EXTICR2 EXTIx         LL_SYSCFG_GetEXTISource\n
  *         SYSCFG_EXTICR3 EXTIx         LL_SYSCFG_GetEXTISource\n
  *         SYSCFG_EXTICR4 EXTIx         LL_SYSCFG_GetEXTISource
  * @param  Line This parameter can be one of the following values:
  *         @arg @ref LL_SYSCFG_EXTI_LINE0
  *         @arg @ref LL_SYSCFG_EXTI_LINE1
  *         @arg @ref LL_SYSCFG_EXTI_LINE2
  *         @arg @ref LL_SYSCFG_EXTI_LINE3
  *         @arg @ref LL_SYSCFG_EXTI_LINE4
  *         @arg @ref LL_SYSCFG_EXTI_LINE5
  *         @arg @ref LL_SYSCFG_EXTI_LINE6
  *         @arg @ref LL_SYSCFG_EXTI_LINE7
  *         @arg @ref LL_SYSCFG_EXTI_LINE8
  *         @arg @ref LL_SYSCFG_EXTI_LINE9
  *         @arg @ref LL_SYSCFG_EXTI_LINE10
  *         @arg @ref LL_SYSCFG_EXTI_LINE11
  *         @arg @ref LL_SYSCFG_EXTI_LINE12
  *         @arg @ref LL_SYSCFG_EXTI_LINE13
  *         @arg @ref LL_SYSCFG_EXTI_LINE14
  *         @arg @ref LL_SYSCFG_EXTI_LINE15
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_SYSCFG_EXTI_PORTA
  *         @arg @ref LL_SYSCFG_EXTI_PORTB
  *         @arg @ref LL_SYSCFG_EXTI_PORTC
  *         @arg @ref LL_SYSCFG_EXTI_PORTD
  *         @arg @ref LL_SYSCFG_EXTI_PORTE
  *         @arg @ref LL_SYSCFG_EXTI_PORTF (*)
  *         @arg @ref LL_SYSCFG_EXTI_PORTG (*)
  *         @arg @ref LL_SYSCFG_EXTI_PORTH
  *         @arg @ref LL_SYSCFG_EXTI_PORTI (*)
  *
  *         (*) value not defined in all devices
  */
__STATIC_INLINE uint32_t LL_SYSCFG_GetEXTISource(uint32_t Line)
{
  return (uint32_t)(READ_BIT(SYSCFG->EXTICR[Line & 0xFFU], (Line >> 16U)) >> POSITION_VAL(Line >> 16U));
}

/**
  * @brief  Enable SRAM2 Erase (starts a hardware SRAM2 erase operation. This bit is
  * automatically cleared at the end of the SRAM2 erase operation.)
  * @note This bit is write-protected: setting this bit is possible only after the
  *       correct key sequence is written in the SYSCFG_SKR register as described in 
  *       the Reference Manual.
  * @rmtoll SYSCFG_SCSR  SRAM2ER       LL_SYSCFG_EnableSRAM2Erase
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableSRAM2Erase(void)
{
  /* Starts a hardware SRAM2 erase operation*/
  SET_BIT(SYSCFG->SCSR, SYSCFG_SCSR_SRAM2ER);
}

/**
  * @brief  Check if SRAM2 erase operation is on going
  * @rmtoll SYSCFG_SCSR  SRAM2BSY      LL_SYSCFG_IsSRAM2EraseOngoing
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsSRAM2EraseOngoing(void)
{
  return (READ_BIT(SYSCFG->SCSR, SYSCFG_SCSR_SRAM2BSY) == (SYSCFG_SCSR_SRAM2BSY));
}

/**
  * @brief  Set connections to TIM1/8/15/16/17 Break inputs
  * @rmtoll SYSCFG_CFGR2 CLL           LL_SYSCFG_SetTIMBreakInputs\n
  *         SYSCFG_CFGR2 SPL           LL_SYSCFG_SetTIMBreakInputs\n
  *         SYSCFG_CFGR2 PVDL          LL_SYSCFG_SetTIMBreakInputs\n
  *         SYSCFG_CFGR2 ECCL          LL_SYSCFG_SetTIMBreakInputs
  * @param  Break This parameter can be a combination of the following values:
  *         @arg @ref LL_SYSCFG_TIMBREAK_ECC
  *         @arg @ref LL_SYSCFG_TIMBREAK_PVD
  *         @arg @ref LL_SYSCFG_TIMBREAK_SRAM2_PARITY
  *         @arg @ref LL_SYSCFG_TIMBREAK_LOCKUP
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_SetTIMBreakInputs(uint32_t Break)
{
  MODIFY_REG(SYSCFG->CFGR2, SYSCFG_CFGR2_CLL | SYSCFG_CFGR2_SPL | SYSCFG_CFGR2_PVDL | SYSCFG_CFGR2_ECCL, Break);
}

/**
  * @brief  Get connections to TIM1/8/15/16/17 Break inputs
  * @rmtoll SYSCFG_CFGR2 CLL           LL_SYSCFG_GetTIMBreakInputs\n
  *         SYSCFG_CFGR2 SPL           LL_SYSCFG_GetTIMBreakInputs\n
  *         SYSCFG_CFGR2 PVDL          LL_SYSCFG_GetTIMBreakInputs\n
  *         SYSCFG_CFGR2 ECCL          LL_SYSCFG_GetTIMBreakInputs
  * @retval Returned value can be can be a combination of the following values:
  *         @arg @ref LL_SYSCFG_TIMBREAK_ECC
  *         @arg @ref LL_SYSCFG_TIMBREAK_PVD
  *         @arg @ref LL_SYSCFG_TIMBREAK_SRAM2_PARITY
  *         @arg @ref LL_SYSCFG_TIMBREAK_LOCKUP
  */
__STATIC_INLINE uint32_t LL_SYSCFG_GetTIMBreakInputs(void)
{
  return (uint32_t)(READ_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_CLL | SYSCFG_CFGR2_SPL | SYSCFG_CFGR2_PVDL | SYSCFG_CFGR2_ECCL));
}

/**
  * @brief  Check if SRAM2 parity error detected
  * @rmtoll SYSCFG_CFGR2 SPF           LL_SYSCFG_IsActiveFlag_SP
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_SYSCFG_IsActiveFlag_SP(void)
{
  return (READ_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_SPF) == (SYSCFG_CFGR2_SPF));
}

/**
  * @brief  Clear SRAM2 parity error flag
  * @rmtoll SYSCFG_CFGR2 SPF           LL_SYSCFG_ClearFlag_SP
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_ClearFlag_SP(void)
{
  SET_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_SPF);
}

/**
  * @brief  Enable SRAM2 page write protection for Pages in range 0 to 31
  * @note Write protection is cleared only by a system reset
  * @rmtoll SYSCFG_SWPR  PxWP         LL_SYSCFG_EnableSRAM2PageWRP_0_31
  * @param  SRAM2WRP This parameter can be a combination of the following values:
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE0
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE1
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE2
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE3
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE4
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE5
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE6
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE7
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE8
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE9
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE10
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE11
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE12
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE13
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE14
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE15
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE16 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE17 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE18 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE19 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE20 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE21 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE22 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE23 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE24 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE25 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE26 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE27 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE28 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE29 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE30 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE31 (*)
  *
  *         (*) value not defined in all devices
  * @retval None
  */
/* Legacy define */
#define LL_SYSCFG_EnableSRAM2PageWRP    LL_SYSCFG_EnableSRAM2PageWRP_0_31
__STATIC_INLINE void LL_SYSCFG_EnableSRAM2PageWRP_0_31(uint32_t SRAM2WRP)
{
  SET_BIT(SYSCFG->SWPR, SRAM2WRP);
}

#if defined(SYSCFG_SWPR2_PAGE63)
/**
  * @brief  Enable SRAM2 page write protection for Pages in range 32 to 63
  * @note Write protection is cleared only by a system reset
  * @rmtoll SYSCFG_SWPR2 PxWP          LL_SYSCFG_EnableSRAM2PageWRP_32_63
  * @param  SRAM2WRP This parameter can be a combination of the following values:
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE32 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE33 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE34 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE35 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE36 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE37 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE38 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE39 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE40 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE41 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE42 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE43 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE44 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE45 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE46 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE47 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE48 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE49 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE50 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE51 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE52 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE53 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE54 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE55 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE56 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE57 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE58 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE59 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE60 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE61 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE62 (*)
  *         @arg @ref LL_SYSCFG_SRAM2WRP_PAGE63 (*)
  *
  *         (*) value not defined in all devices
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_EnableSRAM2PageWRP_32_63(uint32_t SRAM2WRP)
{
  SET_BIT(SYSCFG->SWPR2, SRAM2WRP);
}
#endif /* SYSCFG_SWPR2_PAGE63 */

/**
  * @brief  SRAM2 page write protection lock prior to erase
  * @rmtoll SYSCFG_SKR   KEY           LL_SYSCFG_LockSRAM2WRP
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_LockSRAM2WRP(void)
{
  /* Writing a wrong key reactivates the write protection */
  WRITE_REG(SYSCFG->SKR, 0x00);
}

/**
  * @brief  SRAM2 page write protection unlock prior to erase
  * @rmtoll SYSCFG_SKR   KEY           LL_SYSCFG_UnlockSRAM2WRP
  * @retval None
  */
__STATIC_INLINE void LL_SYSCFG_UnlockSRAM2WRP(void)
{
  /* unlock the write protection of the SRAM2ER bit */
  WRITE_REG(SYSCFG->SKR, 0xCA);
  WRITE_REG(SYSCFG->SKR, 0x53);
}

/**
  * @}
  */


/** @defgroup SYSTEM_LL_EF_DBGMCU DBGMCU
  * @{
  */

/**
  * @brief  Return the device identifier
  * @rmtoll DBGMCU_IDCODE DEV_ID        LL_DBGMCU_GetDeviceID
  * @retval Values between Min_Data=0x00 and Max_Data=0xFFFF (ex: device ID is 0x6415)
  */
__STATIC_INLINE uint32_t LL_DBGMCU_GetDeviceID(void)
{
  return (uint32_t)(READ_BIT(DBGMCU->IDCODE, DBGMCU_IDCODE_DEV_ID));
}

/**
  * @brief  Return the device revision identifier
  * @note This field indicates the revision of the device.
  * @rmtoll DBGMCU_IDCODE REV_ID        LL_DBGMCU_GetRevisionID
  * @retval Values between Min_Data=0x00 and Max_Data=0xFFFF
  */
__STATIC_INLINE uint32_t LL_DBGMCU_GetRevisionID(void)
{
  return (uint32_t)(READ_BIT(DBGMCU->IDCODE, DBGMCU_IDCODE_REV_ID) >> DBGMCU_IDCODE_REV_ID_Pos);
}

/**
  * @brief  Enable the Debug Module during SLEEP mode
  * @rmtoll DBGMCU_CR    DBG_SLEEP     LL_DBGMCU_EnableDBGSleepMode
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_EnableDBGSleepMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}

/**
  * @brief  Disable the Debug Module during SLEEP mode
  * @rmtoll DBGMCU_CR    DBG_SLEEP     LL_DBGMCU_DisableDBGSleepMode
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_DisableDBGSleepMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}

/**
  * @brief  Enable the Debug Module during STOP mode
  * @rmtoll DBGMCU_CR    DBG_STOP      LL_DBGMCU_EnableDBGStopMode
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_EnableDBGStopMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}

/**
  * @brief  Disable the Debug Module during STOP mode
  * @rmtoll DBGMCU_CR    DBG_STOP      LL_DBGMCU_DisableDBGStopMode
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_DisableDBGStopMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}

/**
  * @brief  Enable the Debug Module during STANDBY mode
  * @rmtoll DBGMCU_CR    DBG_STANDBY   LL_DBGMCU_EnableDBGStandbyMode
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_EnableDBGStandbyMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STANDBY);
}

/**
  * @brief  Disable the Debug Module during STANDBY mode
  * @rmtoll DBGMCU_CR    DBG_STANDBY   LL_DBGMCU_DisableDBGStandbyMode
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_DisableDBGStandbyMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STANDBY);
}

/**
  * @brief  Set Trace pin assignment control
  * @rmtoll DBGMCU_CR    TRACE_IOEN    LL_DBGMCU_SetTracePinAssignment\n
  *         DBGMCU_CR    TRACE_MODE    LL_DBGMCU_SetTracePinAssignment
  * @param  PinAssignment This parameter can be one of the following values:
  *         @arg @ref LL_DBGMCU_TRACE_NONE
  *         @arg @ref LL_DBGMCU_TRACE_ASYNCH
  *         @arg @ref LL_DBGMCU_TRACE_SYNCH_SIZE1
  *         @arg @ref LL_DBGMCU_TRACE_SYNCH_SIZE2
  *         @arg @ref LL_DBGMCU_TRACE_SYNCH_SIZE4
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_SetTracePinAssignment(uint32_t PinAssignment)
{
  MODIFY_REG(DBGMCU->CR, DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE, PinAssignment);
}

/**
  * @brief  Get Trace pin assignment control
  * @rmtoll DBGMCU_CR    TRACE_IOEN    LL_DBGMCU_GetTracePinAssignment\n
  *         DBGMCU_CR    TRACE_MODE    LL_DBGMCU_GetTracePinAssignment
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_DBGMCU_TRACE_NONE
  *         @arg @ref LL_DBGMCU_TRACE_ASYNCH
  *         @arg @ref LL_DBGMCU_TRACE_SYNCH_SIZE1
  *         @arg @ref LL_DBGMCU_TRACE_SYNCH_SIZE2
  *         @arg @ref LL_DBGMCU_TRACE_SYNCH_SIZE4
  */
__STATIC_INLINE uint32_t LL_DBGMCU_GetTracePinAssignment(void)
{
  return (uint32_t)(READ_BIT(DBGMCU->CR, DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE));
}

/**
  * @brief  Freeze APB1 peripherals (group1 peripherals)
  * @rmtoll DBGMCU_APB1FZR1 DBG_xxxx_STOP  LL_DBGMCU_APB1_GRP1_FreezePeriph
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM2_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM3_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM4_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM5_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM6_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM7_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_RTC_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_WWDG_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_IWDG_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_I2C1_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_I2C2_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_I2C3_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_CAN_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_CAN2_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_LPTIM1_STOP
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_APB1_GRP1_FreezePeriph(uint32_t Periphs)
{
  SET_BIT(DBGMCU->APB1FZR1, Periphs);
}

/**
  * @brief  Freeze APB1 peripherals (group2 peripherals)
  * @rmtoll DBGMCU_APB1FZR2 DBG_xxxx_STOP  LL_DBGMCU_APB1_GRP2_FreezePeriph
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_DBGMCU_APB1_GRP2_I2C4_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP2_LPTIM2_STOP
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_APB1_GRP2_FreezePeriph(uint32_t Periphs)
{
  SET_BIT(DBGMCU->APB1FZR2, Periphs);
}

/**
  * @brief  Unfreeze APB1 peripherals (group1 peripherals)
  * @rmtoll DBGMCU_APB1FZR1 DBG_xxxx_STOP  LL_DBGMCU_APB1_GRP1_UnFreezePeriph
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM2_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM3_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM4_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM5_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM6_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_TIM7_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_RTC_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_WWDG_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_IWDG_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_I2C1_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_I2C2_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_I2C3_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_CAN_STOP
  *         @arg @ref LL_DBGMCU_APB1_GRP1_CAN2_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP1_LPTIM1_STOP
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_APB1_GRP1_UnFreezePeriph(uint32_t Periphs)
{
  CLEAR_BIT(DBGMCU->APB1FZR1, Periphs);
}

/**
  * @brief  Unfreeze APB1 peripherals (group2 peripherals)
  * @rmtoll DBGMCU_APB1FZR2 DBG_xxxx_STOP  LL_DBGMCU_APB1_GRP2_UnFreezePeriph
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_DBGMCU_APB1_GRP2_I2C4_STOP (*)
  *         @arg @ref LL_DBGMCU_APB1_GRP2_LPTIM2_STOP
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_APB1_GRP2_UnFreezePeriph(uint32_t Periphs)
{
  CLEAR_BIT(DBGMCU->APB1FZR2, Periphs);
}

/**
  * @brief  Freeze APB2 peripherals
  * @rmtoll DBGMCU_APB2FZ DBG_TIMx_STOP  LL_DBGMCU_APB2_GRP1_FreezePeriph
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM1_STOP
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM8_STOP (*)
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM15_STOP
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM16_STOP
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM17_STOP (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_APB2_GRP1_FreezePeriph(uint32_t Periphs)
{
  SET_BIT(DBGMCU->APB2FZ, Periphs);
}

/**
  * @brief  Unfreeze APB2 peripherals
  * @rmtoll DBGMCU_APB2FZ DBG_TIMx_STOP  LL_DBGMCU_APB2_GRP1_UnFreezePeriph
  * @param  Periphs This parameter can be a combination of the following values:
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM1_STOP
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM8_STOP (*)
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM15_STOP
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM16_STOP
  *         @arg @ref LL_DBGMCU_APB2_GRP1_TIM17_STOP (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_DBGMCU_APB2_GRP1_UnFreezePeriph(uint32_t Periphs)
{
  CLEAR_BIT(DBGMCU->APB2FZ, Periphs);
}

/**
  * @}
  */

#if defined(VREFBUF)
/** @defgroup SYSTEM_LL_EF_VREFBUF VREFBUF
  * @{
  */

/**
  * @brief  Enable Internal voltage reference
  * @rmtoll VREFBUF_CSR  ENVR          LL_VREFBUF_Enable
  * @retval None
  */
__STATIC_INLINE void LL_VREFBUF_Enable(void)
{
  SET_BIT(VREFBUF->CSR, VREFBUF_CSR_ENVR);
}

/**
  * @brief  Disable Internal voltage reference
  * @rmtoll VREFBUF_CSR  ENVR          LL_VREFBUF_Disable
  * @retval None
  */
__STATIC_INLINE void LL_VREFBUF_Disable(void)
{
  CLEAR_BIT(VREFBUF->CSR, VREFBUF_CSR_ENVR);
}

/**
  * @brief  Enable high impedance (VREF+pin is high impedance)
  * @rmtoll VREFBUF_CSR  HIZ           LL_VREFBUF_EnableHIZ
  * @retval None
  */
__STATIC_INLINE void LL_VREFBUF_EnableHIZ(void)
{
  SET_BIT(VREFBUF->CSR, VREFBUF_CSR_HIZ);
}

/**
  * @brief  Disable high impedance (VREF+pin is internally connected to the voltage reference buffer output)
  * @rmtoll VREFBUF_CSR  HIZ           LL_VREFBUF_DisableHIZ
  * @retval None
  */
__STATIC_INLINE void LL_VREFBUF_DisableHIZ(void)
{
  CLEAR_BIT(VREFBUF->CSR, VREFBUF_CSR_HIZ);
}

/**
  * @brief  Set the Voltage reference scale
  * @rmtoll VREFBUF_CSR  VRS           LL_VREFBUF_SetVoltageScaling
  * @param  Scale This parameter can be one of the following values:
  *         @arg @ref LL_VREFBUF_VOLTAGE_SCALE0
  *         @arg @ref LL_VREFBUF_VOLTAGE_SCALE1
  * @retval None
  */
__STATIC_INLINE void LL_VREFBUF_SetVoltageScaling(uint32_t Scale)
{
  MODIFY_REG(VREFBUF->CSR, VREFBUF_CSR_VRS, Scale);
}

/**
  * @brief  Get the Voltage reference scale
  * @rmtoll VREFBUF_CSR  VRS           LL_VREFBUF_GetVoltageScaling
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_VREFBUF_VOLTAGE_SCALE0
  *         @arg @ref LL_VREFBUF_VOLTAGE_SCALE1
  */
__STATIC_INLINE uint32_t LL_VREFBUF_GetVoltageScaling(void)
{
  return (uint32_t)(READ_BIT(VREFBUF->CSR, VREFBUF_CSR_VRS));
}

/**
  * @brief  Check if Voltage reference buffer is ready
  * @rmtoll VREFBUF_CSR  VRR           LL_VREFBUF_IsVREFReady
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_VREFBUF_IsVREFReady(void)
{
  return (READ_BIT(VREFBUF->CSR, VREFBUF_CSR_VRR) == (VREFBUF_CSR_VRR));
}

/**
  * @brief  Get the trimming code for VREFBUF calibration
  * @rmtoll VREFBUF_CCR  TRIM          LL_VREFBUF_GetTrimming
  * @retval Between 0 and 0x3F
  */
__STATIC_INLINE uint32_t LL_VREFBUF_GetTrimming(void)
{
  return (uint32_t)(READ_BIT(VREFBUF->CCR, VREFBUF_CCR_TRIM));
}

/**
  * @brief  Set the trimming code for VREFBUF calibration (Tune the internal reference buffer voltage)
  * @rmtoll VREFBUF_CCR  TRIM          LL_VREFBUF_SetTrimming
  * @param  Value Between 0 and 0x3F
  * @retval None
  */
__STATIC_INLINE void LL_VREFBUF_SetTrimming(uint32_t Value)
{
  WRITE_REG(VREFBUF->CCR, Value);
}

/**
  * @}
  */
#endif /* VREFBUF */

/** @defgroup SYSTEM_LL_EF_FLASH FLASH
  * @{
  */

/**
  * @brief  Set FLASH Latency
  * @rmtoll FLASH_ACR    LATENCY       LL_FLASH_SetLatency
  * @param  Latency This parameter can be one of the following values:
  *         @arg @ref LL_FLASH_LATENCY_0
  *         @arg @ref LL_FLASH_LATENCY_1
  *         @arg @ref LL_FLASH_LATENCY_2
  *         @arg @ref LL_FLASH_LATENCY_3
  *         @arg @ref LL_FLASH_LATENCY_4
  *         @arg @ref LL_FLASH_LATENCY_5 (*)
  *         @arg @ref LL_FLASH_LATENCY_6 (*)
  *         @arg @ref LL_FLASH_LATENCY_7 (*)
  *         @arg @ref LL_FLASH_LATENCY_8 (*)
  *         @arg @ref LL_FLASH_LATENCY_9 (*)
  *         @arg @ref LL_FLASH_LATENCY_10 (*)
  *         @arg @ref LL_FLASH_LATENCY_11 (*)
  *         @arg @ref LL_FLASH_LATENCY_12 (*)
  *         @arg @ref LL_FLASH_LATENCY_13 (*)
  *         @arg @ref LL_FLASH_LATENCY_14 (*)
  *         @arg @ref LL_FLASH_LATENCY_15 (*)
  *
  *         (*) value not defined in all devices.
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_SetLatency(uint32_t Latency)
{
  MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, Latency);
}

/**
  * @brief  Get FLASH Latency
  * @rmtoll FLASH_ACR    LATENCY       LL_FLASH_GetLatency
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_FLASH_LATENCY_0
  *         @arg @ref LL_FLASH_LATENCY_1
  *         @arg @ref LL_FLASH_LATENCY_2
  *         @arg @ref LL_FLASH_LATENCY_3
  *         @arg @ref LL_FLASH_LATENCY_4
  *         @arg @ref LL_FLASH_LATENCY_5 (*)
  *         @arg @ref LL_FLASH_LATENCY_6 (*)
  *         @arg @ref LL_FLASH_LATENCY_7 (*)
  *         @arg @ref LL_FLASH_LATENCY_8 (*)
  *         @arg @ref LL_FLASH_LATENCY_9 (*)
  *         @arg @ref LL_FLASH_LATENCY_10 (*)
  *         @arg @ref LL_FLASH_LATENCY_11 (*)
  *         @arg @ref LL_FLASH_LATENCY_12 (*)
  *         @arg @ref LL_FLASH_LATENCY_13 (*)
  *         @arg @ref LL_FLASH_LATENCY_14 (*)
  *         @arg @ref LL_FLASH_LATENCY_15 (*)
  *
  *         (*) value not defined in all devices.
  */
__STATIC_INLINE uint32_t LL_FLASH_GetLatency(void)
{
  return (uint32_t)(READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY));
}

/**
  * @brief  Enable Prefetch
  * @rmtoll FLASH_ACR    PRFTEN        LL_FLASH_EnablePrefetch
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_EnablePrefetch(void)
{
  SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN);
}

/**
  * @brief  Disable Prefetch
  * @rmtoll FLASH_ACR    PRFTEN        LL_FLASH_DisablePrefetch
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_DisablePrefetch(void)
{
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_PRFTEN);
}

/**
  * @brief  Check if Prefetch buffer is enabled
  * @rmtoll FLASH_ACR    PRFTEN        LL_FLASH_IsPrefetchEnabled
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_FLASH_IsPrefetchEnabled(void)
{
  return (READ_BIT(FLASH->ACR, FLASH_ACR_PRFTEN) == (FLASH_ACR_PRFTEN));
}

/**
  * @brief  Enable Instruction cache
  * @rmtoll FLASH_ACR    ICEN          LL_FLASH_EnableInstCache
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_EnableInstCache(void)
{
  SET_BIT(FLASH->ACR, FLASH_ACR_ICEN);
}

/**
  * @brief  Disable Instruction cache
  * @rmtoll FLASH_ACR    ICEN          LL_FLASH_DisableInstCache
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_DisableInstCache(void)
{
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICEN);
}

/**
  * @brief  Enable Data cache
  * @rmtoll FLASH_ACR    DCEN          LL_FLASH_EnableDataCache
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_EnableDataCache(void)
{
  SET_BIT(FLASH->ACR, FLASH_ACR_DCEN);
}

/**
  * @brief  Disable Data cache
  * @rmtoll FLASH_ACR    DCEN          LL_FLASH_DisableDataCache
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_DisableDataCache(void)
{
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCEN);
}

/**
  * @brief  Enable Instruction cache reset
  * @note  bit can be written only when the instruction cache is disabled
  * @rmtoll FLASH_ACR    ICRST         LL_FLASH_EnableInstCacheReset
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_EnableInstCacheReset(void)
{
  SET_BIT(FLASH->ACR, FLASH_ACR_ICRST);
}

/**
  * @brief  Disable Instruction cache reset
  * @rmtoll FLASH_ACR    ICRST         LL_FLASH_DisableInstCacheReset
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_DisableInstCacheReset(void)
{
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICRST);
}

/**
  * @brief  Enable Data cache reset
  * @note bit can be written only when the data cache is disabled
  * @rmtoll FLASH_ACR    DCRST         LL_FLASH_EnableDataCacheReset
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_EnableDataCacheReset(void)
{
  SET_BIT(FLASH->ACR, FLASH_ACR_DCRST);
}

/**
  * @brief  Disable Data cache reset
  * @rmtoll FLASH_ACR    DCRST         LL_FLASH_DisableDataCacheReset
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_DisableDataCacheReset(void)
{
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCRST);
}

/**
  * @brief  Enable Flash Power-down mode during run mode or Low-power run mode
  * @note Flash memory can be put in power-down mode only when the code is executed
  *       from RAM
  * @note Flash must not be accessed when power down is enabled
  * @note Flash must not be put in power-down while a program or an erase operation
  *       is on-going
  * @rmtoll FLASH_ACR    RUN_PD        LL_FLASH_EnableRunPowerDown\n
  *         FLASH_PDKEYR PDKEY1        LL_FLASH_EnableRunPowerDown\n
  *         FLASH_PDKEYR PDKEY2        LL_FLASH_EnableRunPowerDown
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_EnableRunPowerDown(void)
{
  /* Following values must be written consecutively to unlock the RUN_PD bit in
     FLASH_ACR */
  WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY1);
  WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY2);
  SET_BIT(FLASH->ACR, FLASH_ACR_RUN_PD);
}

/**
  * @brief  Disable Flash Power-down mode during run mode or Low-power run mode
  * @rmtoll FLASH_ACR    RUN_PD        LL_FLASH_DisableRunPowerDown\n
  *         FLASH_PDKEYR PDKEY1        LL_FLASH_DisableRunPowerDown\n
  *         FLASH_PDKEYR PDKEY2        LL_FLASH_DisableRunPowerDown
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_DisableRunPowerDown(void)
{
  /* Following values must be written consecutively to unlock the RUN_PD bit in
     FLASH_ACR */
  WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY1);
  WRITE_REG(FLASH->PDKEYR, FLASH_PDKEY2);
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_RUN_PD);
}

/**
  * @brief  Enable Flash Power-down mode during Sleep or Low-power sleep mode
  * @note Flash must not be put in power-down while a program or an erase operation
  *       is on-going
  * @rmtoll FLASH_ACR    SLEEP_PD      LL_FLASH_EnableSleepPowerDown
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_EnableSleepPowerDown(void)
{
  SET_BIT(FLASH->ACR, FLASH_ACR_SLEEP_PD);
}

/**
  * @brief  Disable Flash Power-down mode during Sleep or Low-power sleep mode
  * @rmtoll FLASH_ACR    SLEEP_PD      LL_FLASH_DisableSleepPowerDown
  * @retval None
  */
__STATIC_INLINE void LL_FLASH_DisableSleepPowerDown(void)
{
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_SLEEP_PD);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* defined (FLASH) || defined (SYSCFG) || defined (DBGMCU) || defined (VREFBUF) */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_LL_SYSTEM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
