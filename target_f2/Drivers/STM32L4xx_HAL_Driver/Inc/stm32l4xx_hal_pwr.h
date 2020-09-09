/**
  ******************************************************************************
  * @file    stm32l4xx_hal_pwr.h
  * @author  MCD Application Team
  * @brief   Header file of PWR HAL module.
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
#ifndef __STM32L4xx_HAL_PWR_H
#define __STM32L4xx_HAL_PWR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup PWR
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup PWR_Exported_Types PWR Exported Types
  * @{
  */

/**
  * @brief  PWR PVD configuration structure definition
  */
typedef struct
{
  uint32_t PVDLevel;   /*!< PVDLevel: Specifies the PVD detection level.
                            This parameter can be a value of @ref PWR_PVD_detection_level. */

  uint32_t Mode;      /*!< Mode: Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref PWR_PVD_Mode. */
}PWR_PVDTypeDef;


/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup PWR_Exported_Constants PWR Exported Constants
  * @{
  */


/** @defgroup PWR_PVD_detection_level Programmable Voltage Detection levels
  * @{
  */
#define PWR_PVDLEVEL_0                  PWR_CR2_PLS_LEV0  /*!< PVD threshold around 2.0 V */
#define PWR_PVDLEVEL_1                  PWR_CR2_PLS_LEV1  /*!< PVD threshold around 2.2 V */
#define PWR_PVDLEVEL_2                  PWR_CR2_PLS_LEV2  /*!< PVD threshold around 2.4 V */
#define PWR_PVDLEVEL_3                  PWR_CR2_PLS_LEV3  /*!< PVD threshold around 2.5 V */
#define PWR_PVDLEVEL_4                  PWR_CR2_PLS_LEV4  /*!< PVD threshold around 2.6 V */
#define PWR_PVDLEVEL_5                  PWR_CR2_PLS_LEV5  /*!< PVD threshold around 2.8 V */
#define PWR_PVDLEVEL_6                  PWR_CR2_PLS_LEV6  /*!< PVD threshold around 2.9 V */
#define PWR_PVDLEVEL_7                  PWR_CR2_PLS_LEV7  /*!< External input analog voltage (compared internally to VREFINT) */
/**
  * @}
  */

/** @defgroup PWR_PVD_Mode  PWR PVD interrupt and event mode
  * @{
  */
#define PWR_PVD_MODE_NORMAL                 ((uint32_t)0x00000000)   /*!< Basic mode is used */
#define PWR_PVD_MODE_IT_RISING              ((uint32_t)0x00010001)   /*!< External Interrupt Mode with Rising edge trigger detection */
#define PWR_PVD_MODE_IT_FALLING             ((uint32_t)0x00010002)   /*!< External Interrupt Mode with Falling edge trigger detection */
#define PWR_PVD_MODE_IT_RISING_FALLING      ((uint32_t)0x00010003)   /*!< External Interrupt Mode with Rising/Falling edge trigger detection */
#define PWR_PVD_MODE_EVENT_RISING           ((uint32_t)0x00020001)   /*!< Event Mode with Rising edge trigger detection */
#define PWR_PVD_MODE_EVENT_FALLING          ((uint32_t)0x00020002)   /*!< Event Mode with Falling edge trigger detection */
#define PWR_PVD_MODE_EVENT_RISING_FALLING   ((uint32_t)0x00020003)   /*!< Event Mode with Rising/Falling edge trigger detection */
/**
  * @}
  */




/** @defgroup PWR_Regulator_state_in_SLEEP_STOP_mode  PWR regulator mode
  * @{
  */
#define PWR_MAINREGULATOR_ON            ((uint32_t)0x00000000) /*!< Regulator in main mode      */
#define PWR_LOWPOWERREGULATOR_ON        PWR_CR1_LPR            /*!< Regulator in low-power mode */
/**
  * @}
  */

/** @defgroup PWR_SLEEP_mode_entry  PWR SLEEP mode entry
  * @{
  */
#define PWR_SLEEPENTRY_WFI              ((uint8_t)0x01)        /*!< Wait For Interruption instruction to enter Sleep mode */
#define PWR_SLEEPENTRY_WFE              ((uint8_t)0x02)        /*!< Wait For Event instruction to enter Sleep mode        */
/**
  * @}
  */

/** @defgroup PWR_STOP_mode_entry  PWR STOP mode entry
  * @{
  */
#define PWR_STOPENTRY_WFI               ((uint8_t)0x01)       /*!< Wait For Interruption instruction to enter Stop mode */
#define PWR_STOPENTRY_WFE               ((uint8_t)0x02)       /*!< Wait For Event instruction to enter Stop mode        */
/**
  * @}
  */


/** @defgroup PWR_PVD_EXTI_LINE  PWR PVD external interrupt line
  * @{
  */
#define PWR_EXTI_LINE_PVD  ((uint32_t)0x00010000)   /*!< External interrupt line 16 Connected to the PVD EXTI Line */
/**
  * @}
  */

/** @defgroup PWR_PVD_EVENT_LINE  PWR PVD event line
  * @{
  */
#define PWR_EVENT_LINE_PVD  ((uint32_t)0x00010000)  /*!< Event line 16 Connected to the PVD Event Line */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup PWR_Exported_Macros  PWR Exported Macros
  * @{
  */

/** @brief  Check whether or not a specific PWR flag is set.
  * @param  __FLAG__: specifies the flag to check.
  *           This parameter can be one of the following values:
  *            @arg @ref PWR_FLAG_WUF1 Wake Up Flag 1. Indicates that a wakeup event
  *                  was received from the WKUP pin 1.
  *            @arg @ref PWR_FLAG_WUF2 Wake Up Flag 2. Indicates that a wakeup event
  *                  was received from the WKUP pin 2.
  *            @arg @ref PWR_FLAG_WUF3 Wake Up Flag 3. Indicates that a wakeup event
  *                  was received from the WKUP pin 3.
  *            @arg @ref PWR_FLAG_WUF4 Wake Up Flag 4. Indicates that a wakeup event
  *                  was received from the WKUP pin 4.
  *            @arg @ref PWR_FLAG_WUF5 Wake Up Flag 5. Indicates that a wakeup event
  *                  was received from the WKUP pin 5.
  *            @arg @ref PWR_FLAG_SB StandBy Flag. Indicates that the system
  *                  entered StandBy mode.
  *            @arg @ref PWR_FLAG_EXT_SMPS External SMPS Ready Flag. When available on device, indicates
  *                 that external switch can be closed to connect to the external SMPS, when the Range 2
  *                 of internal regulator is ready.
  *            @arg @ref PWR_FLAG_WUFI Wake-Up Flag Internal. Set when a wakeup is detected on
  *                 the internal wakeup line.
  *            @arg @ref PWR_FLAG_REGLPS Low Power Regulator Started. Indicates whether or not the
  *                 low-power regulator is ready.
  *            @arg @ref PWR_FLAG_REGLPF Low Power Regulator Flag. Indicates whether the
  *                 regulator is ready in main mode or is in low-power mode.
  *            @arg @ref PWR_FLAG_VOSF Voltage Scaling Flag. Indicates whether the regulator is ready
  *                 in the selected voltage range or is still changing to the required voltage level.
  *            @arg @ref PWR_FLAG_PVDO Power Voltage Detector Output. Indicates whether VDD voltage is
  *                  below or above the selected PVD threshold.
  *            @arg @ref PWR_FLAG_PVMO1 Peripheral Voltage Monitoring Output 1. Indicates whether VDDUSB voltage is
  *                  is below or above PVM1 threshold (applicable when USB feature is supported).
  @if STM32L486xx
  *            @arg @ref PWR_FLAG_PVMO2 Peripheral Voltage Monitoring Output 2. Indicates whether VDDIO2 voltage is
  *                  is below or above PVM2 threshold (applicable when VDDIO2 is present on device).
  @endif
  *            @arg @ref PWR_FLAG_PVMO3 Peripheral Voltage Monitoring Output 3. Indicates whether VDDA voltage is
  *                  is below or above PVM3 threshold.
  *            @arg @ref PWR_FLAG_PVMO4 Peripheral Voltage Monitoring Output 4. Indicates whether VDDA voltage is
  *                  is below or above PVM4 threshold.
  *
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_PWR_GET_FLAG(__FLAG__)  ( ((((uint8_t)(__FLAG__)) >> 5U) == 1)  ?\
                                      (PWR->SR1 & (1U << ((__FLAG__) & 31U))) :\
                                      (PWR->SR2 & (1U << ((__FLAG__) & 31U))) )

/** @brief  Clear a specific PWR flag.
  * @param  __FLAG__: specifies the flag to clear.
  *          This parameter can be one of the following values:
  *            @arg @ref PWR_FLAG_WUF1 Wake Up Flag 1. Indicates that a wakeup event
  *                  was received from the WKUP pin 1.
  *            @arg @ref PWR_FLAG_WUF2 Wake Up Flag 2. Indicates that a wakeup event
  *                  was received from the WKUP pin 2.
  *            @arg @ref PWR_FLAG_WUF3 Wake Up Flag 3. Indicates that a wakeup event
  *                  was received from the WKUP pin 3.
  *            @arg @ref PWR_FLAG_WUF4 Wake Up Flag 4. Indicates that a wakeup event
  *                  was received from the WKUP pin 4.
  *            @arg @ref PWR_FLAG_WUF5 Wake Up Flag 5. Indicates that a wakeup event
  *                  was received from the WKUP pin 5.
  *            @arg @ref PWR_FLAG_WU Encompasses all five Wake Up Flags.
  *            @arg @ref PWR_FLAG_SB Standby Flag. Indicates that the system
  *                  entered Standby mode.
  * @retval None
  */
#define __HAL_PWR_CLEAR_FLAG(__FLAG__)   ( (((uint8_t)(__FLAG__)) == PWR_FLAG_WU) ?\
                                         (PWR->SCR  = (__FLAG__)) :\
                                         (PWR->SCR = (1U << ((__FLAG__) & 31U))) )
/**
  * @brief Enable the PVD Extended Interrupt Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_IT()   SET_BIT(EXTI->IMR1, PWR_EXTI_LINE_PVD)

/**
  * @brief Disable the PVD Extended Interrupt Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_IT()  CLEAR_BIT(EXTI->IMR1, PWR_EXTI_LINE_PVD)

/**
  * @brief Enable the PVD Event Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_EVENT()   SET_BIT(EXTI->EMR1, PWR_EVENT_LINE_PVD)

/**
  * @brief Disable the PVD Event Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_EVENT()  CLEAR_BIT(EXTI->EMR1, PWR_EVENT_LINE_PVD)

/**
  * @brief Enable the PVD Extended Interrupt Rising Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_RISING_EDGE()   SET_BIT(EXTI->RTSR1, PWR_EXTI_LINE_PVD)

/**
  * @brief Disable the PVD Extended Interrupt Rising Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_RISING_EDGE()  CLEAR_BIT(EXTI->RTSR1, PWR_EXTI_LINE_PVD)

/**
  * @brief Enable the PVD Extended Interrupt Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_FALLING_EDGE()   SET_BIT(EXTI->FTSR1, PWR_EXTI_LINE_PVD)


/**
  * @brief Disable the PVD Extended Interrupt Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_FALLING_EDGE()  CLEAR_BIT(EXTI->FTSR1, PWR_EXTI_LINE_PVD)


/**
  * @brief  Enable the PVD Extended Interrupt Rising & Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_RISING_FALLING_EDGE()  \
  do {                                                   \
    __HAL_PWR_PVD_EXTI_ENABLE_RISING_EDGE();             \
    __HAL_PWR_PVD_EXTI_ENABLE_FALLING_EDGE();            \
  } while(0)

/**
  * @brief Disable the PVD Extended Interrupt Rising & Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_RISING_FALLING_EDGE()  \
  do {                                                    \
    __HAL_PWR_PVD_EXTI_DISABLE_RISING_EDGE();             \
    __HAL_PWR_PVD_EXTI_DISABLE_FALLING_EDGE();            \
  } while(0)

/**
  * @brief  Generate a Software interrupt on selected EXTI line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_GENERATE_SWIT() SET_BIT(EXTI->SWIER1, PWR_EXTI_LINE_PVD)

/**
  * @brief Check whether or not the PVD EXTI interrupt flag is set.
  * @retval EXTI PVD Line Status.
  */
#define __HAL_PWR_PVD_EXTI_GET_FLAG()  (EXTI->PR1 & PWR_EXTI_LINE_PVD)

/**
  * @brief Clear the PVD EXTI interrupt flag.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_CLEAR_FLAG()  WRITE_REG(EXTI->PR1, PWR_EXTI_LINE_PVD)

/**
  * @}
  */


/* Private macros --------------------------------------------------------*/
/** @addtogroup  PWR_Private_Macros   PWR Private Macros
  * @{
  */

#define IS_PWR_PVD_LEVEL(LEVEL) (((LEVEL) == PWR_PVDLEVEL_0) || ((LEVEL) == PWR_PVDLEVEL_1)|| \
                                 ((LEVEL) == PWR_PVDLEVEL_2) || ((LEVEL) == PWR_PVDLEVEL_3)|| \
                                 ((LEVEL) == PWR_PVDLEVEL_4) || ((LEVEL) == PWR_PVDLEVEL_5)|| \
                                 ((LEVEL) == PWR_PVDLEVEL_6) || ((LEVEL) == PWR_PVDLEVEL_7))

#define IS_PWR_PVD_MODE(MODE)  (((MODE) == PWR_PVD_MODE_NORMAL)              ||\
                                ((MODE) == PWR_PVD_MODE_IT_RISING)           ||\
                                ((MODE) == PWR_PVD_MODE_IT_FALLING)          ||\
                                ((MODE) == PWR_PVD_MODE_IT_RISING_FALLING)   ||\
                                ((MODE) == PWR_PVD_MODE_EVENT_RISING)        ||\
                                ((MODE) == PWR_PVD_MODE_EVENT_FALLING)       ||\
                                ((MODE) == PWR_PVD_MODE_EVENT_RISING_FALLING))

#define IS_PWR_REGULATOR(REGULATOR)      (((REGULATOR) == PWR_MAINREGULATOR_ON) || \
                                          ((REGULATOR) == PWR_LOWPOWERREGULATOR_ON))

#define IS_PWR_SLEEP_ENTRY(ENTRY) (((ENTRY) == PWR_SLEEPENTRY_WFI) || ((ENTRY) == PWR_SLEEPENTRY_WFE))

#define IS_PWR_STOP_ENTRY(ENTRY) (((ENTRY) == PWR_STOPENTRY_WFI) || ((ENTRY) == PWR_STOPENTRY_WFE) )

/**
  * @}
  */

/* Include PWR HAL Extended module */
#include "stm32l4xx_hal_pwr_ex.h"

/* Exported functions --------------------------------------------------------*/

/** @addtogroup PWR_Exported_Functions PWR Exported Functions
  * @{
  */

/** @addtogroup PWR_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */

/* Initialization and de-initialization functions *******************************/
void HAL_PWR_DeInit(void);
void HAL_PWR_EnableBkUpAccess(void);
void HAL_PWR_DisableBkUpAccess(void);

/**
  * @}
  */

/** @addtogroup PWR_Exported_Functions_Group2 Peripheral Control functions
  * @{
  */

/* Peripheral Control functions  ************************************************/
HAL_StatusTypeDef HAL_PWR_ConfigPVD(PWR_PVDTypeDef *sConfigPVD);
void HAL_PWR_EnablePVD(void);
void HAL_PWR_DisablePVD(void);


/* WakeUp pins configuration functions ****************************************/
void HAL_PWR_EnableWakeUpPin(uint32_t WakeUpPinPolarity);
void HAL_PWR_DisableWakeUpPin(uint32_t WakeUpPinx);

/* Low Power modes configuration functions ************************************/
void HAL_PWR_EnterSLEEPMode(uint32_t Regulator, uint8_t SLEEPEntry);
void HAL_PWR_EnterSTOPMode(uint32_t Regulator, uint8_t STOPEntry);
void HAL_PWR_EnterSTANDBYMode(void);

void HAL_PWR_EnableSleepOnExit(void);
void HAL_PWR_DisableSleepOnExit(void);
void HAL_PWR_EnableSEVOnPend(void);
void HAL_PWR_DisableSEVOnPend(void);

void HAL_PWR_PVDCallback(void);


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


#endif /* __STM32L4xx_HAL_PWR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
