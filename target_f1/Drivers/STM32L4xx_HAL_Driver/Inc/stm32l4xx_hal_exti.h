/**
  ******************************************************************************
  * @file    stm32l4xx_hal_exti.h
  * @author  MCD Application Team
  * @brief   Header file of EXTI HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
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
#ifndef STM32L4xx_HAL_EXTI_H
#define STM32L4xx_HAL_EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup EXTI EXTI
  * @brief EXTI HAL module driver
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup EXTI_Exported_Types EXTI Exported Types
  * @{
  */
typedef enum
{
  HAL_EXTI_COMMON_CB_ID          = 0x00U,
  HAL_EXTI_RISING_CB_ID          = 0x01U,
  HAL_EXTI_FALLING_CB_ID         = 0x02U,
} EXTI_CallbackIDTypeDef;


/**
  * @brief  EXTI Handle structure definition
  */
typedef struct
{
  uint32_t Line;                    /*!<  Exti line number */
  void (* PendingCallback)(void);   /*!<  Exti pending callback */
} EXTI_HandleTypeDef;

/**
  * @brief  EXTI Configuration structure definition
  */
typedef struct
{
  uint32_t Line;      /*!< The Exti line to be configured. This parameter
                           can be a value of @ref EXTI_Line */
  uint32_t Mode;      /*!< The Exit Mode to be configured for a core.
                           This parameter can be a combination of @ref EXTI_Mode */
  uint32_t Trigger;   /*!< The Exti Trigger to be configured. This parameter
                           can be a value of @ref EXTI_Trigger */
  uint32_t GPIOSel;   /*!< The Exti GPIO multiplexer selection to be configured.
                           This parameter is only possible for line 0 to 15. It
                           can be a value of @ref EXTI_GPIOSel */
} EXTI_ConfigTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup EXTI_Exported_Constants EXTI Exported Constants
  * @{
  */

/** @defgroup EXTI_Line  EXTI Line
  * @{
  */
#if defined(STM32L412xx) || defined(STM32L422xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_RESERVED | EXTI_REG1              | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_RESERVED | EXTI_REG1              | 0x1Du)
#define EXTI_LINE_30                        (EXTI_RESERVED | EXTI_REG1              | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_RESERVED | EXTI_REG2              | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_RESERVED | EXTI_REG2              | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_RESERVED | EXTI_REG2              | 0x07u)
#define EXTI_LINE_40                        (EXTI_RESERVED | EXTI_REG2              | 0x08u)

#endif /* STM32L412xx || STM32L422xx */

#if defined(STM32L431xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_RESERVED | EXTI_REG1              | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_RESERVED | EXTI_REG1              | 0x1Du)
#define EXTI_LINE_30                        (EXTI_RESERVED | EXTI_REG1              | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_35                        (EXTI_RESERVED | EXTI_REG2              | 0x03u)
#define EXTI_LINE_36                        (EXTI_RESERVED | EXTI_REG2              | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_RESERVED | EXTI_REG2              | 0x07u)
#define EXTI_LINE_40                        (EXTI_RESERVED | EXTI_REG2              | 0x08u)

#endif /* STM32L431xx */

#if defined(STM32L432xx) || defined(STM32L442xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_RESERVED | EXTI_REG1              | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_RESERVED | EXTI_REG1              | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_RESERVED | EXTI_REG1              | 0x1Du)
#define EXTI_LINE_30                        (EXTI_RESERVED | EXTI_REG1              | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_RESERVED | EXTI_REG2              | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_40                        (EXTI_RESERVED | EXTI_REG2              | 0x08u)

#endif /* STM32L432xx || STM32L442xx */

#if defined(STM32L433xx) || defined(STM32L443xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_RESERVED | EXTI_REG1              | 0x1Du)
#define EXTI_LINE_30                        (EXTI_RESERVED | EXTI_REG1              | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_RESERVED | EXTI_REG2              | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_40                        (EXTI_RESERVED | EXTI_REG2              | 0x08u)

#endif /* STM32L433xx || STM32L443xx */

#if defined(STM32L451xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_RESERVED | EXTI_REG1              | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Du)
#define EXTI_LINE_30                        (EXTI_RESERVED | EXTI_REG1              | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_RESERVED | EXTI_REG2              | 0x02u)
#define EXTI_LINE_35                        (EXTI_RESERVED | EXTI_REG2              | 0x03u)
#define EXTI_LINE_36                        (EXTI_RESERVED | EXTI_REG2              | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_RESERVED | EXTI_REG2              | 0x07u)
#define EXTI_LINE_40                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x08u)

#endif /* STM32L451xx */

#if defined(STM32L452xx) || defined(STM32L462xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Du)
#define EXTI_LINE_30                        (EXTI_RESERVED | EXTI_REG1              | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_RESERVED | EXTI_REG2              | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_RESERVED | EXTI_REG2              | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_RESERVED | EXTI_REG2              | 0x07u)
#define EXTI_LINE_40                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x08u)

#endif /* STM32L452xx || STM32L462xx */

#if defined(STM32L471xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_RESERVED | EXTI_REG1              | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Du)
#define EXTI_LINE_30                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_35                        (EXTI_RESERVED | EXTI_REG2              | 0x03u)
#define EXTI_LINE_36                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_RESERVED | EXTI_REG2              | 0x07u)
#define EXTI_LINE_40                        (EXTI_RESERVED | EXTI_REG2              | 0x08u)

#endif /* STM32L471xx */

#if defined(STM32L475xx) || defined(STM32L485xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Du)
#define EXTI_LINE_30                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_RESERVED | EXTI_REG2              | 0x07u)
#define EXTI_LINE_40                        (EXTI_RESERVED | EXTI_REG2              | 0x08u)

#endif /* STM32L475xx || STM32L485xx  */

#if defined(STM32L476xx) || defined(STM32L486xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Du)
#define EXTI_LINE_30                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_40                        (EXTI_RESERVED | EXTI_REG2              | 0x08u)

#endif /*  STM32L476xx || STM32L486xx */

#if defined(STM32L496xx) || defined(STM32L4A6xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Du)
#define EXTI_LINE_30                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_40                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x08u)

#endif /*  STM32L496xx || STM32L4A6xx */

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)

#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | EXTI_EVENT | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x10u)
#define EXTI_LINE_17                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x11u)
#define EXTI_LINE_18                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | EXTI_EVENT | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Du)
#define EXTI_LINE_30                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Eu)
#define EXTI_LINE_31                        (EXTI_DIRECT   | EXTI_REG1 | EXTI_EVENT | 0x1Fu)
#define EXTI_LINE_32                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x00u)
#define EXTI_LINE_33                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x01u)
#define EXTI_LINE_34                        (EXTI_RESERVED | EXTI_REG2              | 0x02u)
#define EXTI_LINE_35                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x03u)
#define EXTI_LINE_36                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x04u)
#define EXTI_LINE_37                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x05u)
#define EXTI_LINE_38                        (EXTI_CONFIG   | EXTI_REG2 | EXTI_EVENT | 0x06u)
#define EXTI_LINE_39                        (EXTI_RESERVED | EXTI_REG2              | 0x07u)
#define EXTI_LINE_40                        (EXTI_DIRECT   | EXTI_REG2 | EXTI_EVENT | 0x08u)

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @}
  */

/** @defgroup EXTI_Mode  EXTI Mode
  * @{
  */
#define EXTI_MODE_NONE                      0x00000000u
#define EXTI_MODE_INTERRUPT                 0x00000001u
#define EXTI_MODE_EVENT                     0x00000002u
/**
  * @}
  */

/** @defgroup EXTI_Trigger  EXTI Trigger
  * @{
  */
#define EXTI_TRIGGER_NONE                   0x00000000u
#define EXTI_TRIGGER_RISING                 0x00000001u
#define EXTI_TRIGGER_FALLING                0x00000002u
#define EXTI_TRIGGER_RISING_FALLING         (EXTI_TRIGGER_RISING | EXTI_TRIGGER_FALLING)
/**
  * @}
  */

/** @defgroup EXTI_GPIOSel  EXTI GPIOSel
  * @brief
  * @{
  */
#define EXTI_GPIOA                          0x00000000u
#define EXTI_GPIOB                          0x00000001u
#define EXTI_GPIOC                          0x00000002u
#define EXTI_GPIOD                          0x00000003u
#define EXTI_GPIOE                          0x00000004u
#define EXTI_GPIOF                          0x00000005u
#define EXTI_GPIOG                          0x00000005u
#define EXTI_GPIOH                          0x00000007u
#define EXTI_GPIOI                          0x00000008u
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup EXTI_Exported_Macros EXTI Exported Macros
  * @{
  */

/**
  * @}
  */

/* Private constants --------------------------------------------------------*/
/** @defgroup EXTI_Private_Constants EXTI Private Constants
  * @{
  */
/**
  * @brief  EXTI Line property definition
  */
#define EXTI_PROPERTY_SHIFT                  24u
#define EXTI_DIRECT                         (0x01uL << EXTI_PROPERTY_SHIFT)
#define EXTI_CONFIG                         (0x02uL << EXTI_PROPERTY_SHIFT)
#define EXTI_GPIO                           ((0x04uL << EXTI_PROPERTY_SHIFT) | EXTI_CONFIG)
#define EXTI_RESERVED                       (0x08uL << EXTI_PROPERTY_SHIFT)
#define EXTI_PROPERTY_MASK                  (EXTI_DIRECT | EXTI_CONFIG | EXTI_GPIO)

/**
  * @brief  EXTI Event presence definition
  */
#define EXTI_EVENT_PRESENCE_SHIFT           28u
#define EXTI_EVENT                          (0x01uL << EXTI_EVENT_PRESENCE_SHIFT)
#define EXTI_EVENT_PRESENCE_MASK            (EXTI_EVENT)

/**
  * @brief  EXTI Register and bit usage
  */
#define EXTI_REG_SHIFT                      16u
#define EXTI_REG1                           (0x00uL << EXTI_REG_SHIFT)
#define EXTI_REG2                           (0x01uL << EXTI_REG_SHIFT)
#define EXTI_REG_MASK                       (EXTI_REG1 | EXTI_REG2)
#define EXTI_PIN_MASK                       0x0000001Fu

/**
  * @brief  EXTI Mask for interrupt & event mode
  */
#define EXTI_MODE_MASK                      (EXTI_MODE_EVENT | EXTI_MODE_INTERRUPT)

/**
  * @brief  EXTI Mask for trigger possibilities
  */
#define EXTI_TRIGGER_MASK                   (EXTI_TRIGGER_RISING | EXTI_TRIGGER_FALLING)

/**
  * @brief  EXTI Line number
  */
#define EXTI_LINE_NB                        41u

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup EXTI_Private_Macros EXTI Private Macros
  * @{
  */
#define IS_EXTI_LINE(__LINE__)          ((((__LINE__) & ~(EXTI_PROPERTY_MASK | EXTI_EVENT_PRESENCE_MASK | EXTI_REG_MASK | EXTI_PIN_MASK)) == 0x00u) && \
                                        ((((__LINE__) & EXTI_PROPERTY_MASK) == EXTI_DIRECT)   || \
                                         (((__LINE__) & EXTI_PROPERTY_MASK) == EXTI_CONFIG)   || \
                                         (((__LINE__) & EXTI_PROPERTY_MASK) == EXTI_GPIO))    && \
                                         (((__LINE__) & (EXTI_REG_MASK | EXTI_PIN_MASK))      < \
                                         (((EXTI_LINE_NB / 32u) << EXTI_REG_SHIFT) | (EXTI_LINE_NB % 32u))))

#define IS_EXTI_MODE(__LINE__)          ((((__LINE__) & EXTI_MODE_MASK) != 0x00u) && \
                                         (((__LINE__) & ~EXTI_MODE_MASK) == 0x00u))

#define IS_EXTI_TRIGGER(__LINE__)       (((__LINE__) & ~EXTI_TRIGGER_MASK) == 0x00u)

#define IS_EXTI_PENDING_EDGE(__LINE__)  ((__LINE__) == EXTI_TRIGGER_RISING_FALLING)

#define IS_EXTI_CONFIG_LINE(__LINE__)   (((__LINE__) & EXTI_CONFIG) != 0x00u)

#if defined(STM32L412xx) || defined(STM32L422xx)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOD) || \
                                         ((__PORT__) == EXTI_GPIOH))

#endif /* STM32L412xx || STM32L422xx */

#if defined(STM32L431xx) || defined(STM32L433xx) || defined(STM32L443xx)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOD) || \
                                         ((__PORT__) == EXTI_GPIOE) || \
                                         ((__PORT__) == EXTI_GPIOH))

#endif /* STM32L431xx || STM32L433xx || STM32L443xx */

#if defined(STM32L432xx) || defined(STM32L442xx)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOH))

#endif /* STM32L432xx || STM32L442xx */

#if defined(STM32L451xx) || defined(STM32L452xx) || defined(STM32L462xx)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOD) || \
                                         ((__PORT__) == EXTI_GPIOE) || \
                                         ((__PORT__) == EXTI_GPIOH))

#endif /* STM32L451xx || STM32L452xx || STM32L462xx */

#if defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOD) || \
                                         ((__PORT__) == EXTI_GPIOE) || \
                                         ((__PORT__) == EXTI_GPIOF) || \
                                         ((__PORT__) == EXTI_GPIOG) || \
                                         ((__PORT__) == EXTI_GPIOH))

#endif /* STM32L471xx || STM32L475xx || STM32L476xx || STM32L485xx || STM32L486xx */

#if defined(STM32L496xx) || defined(STM32L4A6xx)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOD) || \
                                         ((__PORT__) == EXTI_GPIOE) || \
                                         ((__PORT__) == EXTI_GPIOF) || \
                                         ((__PORT__) == EXTI_GPIOG) || \
                                         ((__PORT__) == EXTI_GPIOH) || \
                                         ((__PORT__) == EXTI_GPIOI))

#endif /* STM32L496xx || STM32L4A6xx */

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOD) || \
                                         ((__PORT__) == EXTI_GPIOE) || \
                                         ((__PORT__) == EXTI_GPIOF) || \
                                         ((__PORT__) == EXTI_GPIOG) || \
                                         ((__PORT__) == EXTI_GPIOH) || \
                                         ((__PORT__) == EXTI_GPIOI))

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

#define IS_EXTI_GPIO_PIN(__PIN__)       ((__PIN__) < 16u)
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup EXTI_Exported_Functions EXTI Exported Functions
  * @brief    EXTI Exported Functions
  * @{
  */

/** @defgroup EXTI_Exported_Functions_Group1 Configuration functions
  * @brief    Configuration functions
  * @{
  */
/* Configuration functions ****************************************************/
HAL_StatusTypeDef HAL_EXTI_SetConfigLine(EXTI_HandleTypeDef *hexti, EXTI_ConfigTypeDef *pExtiConfig);
HAL_StatusTypeDef HAL_EXTI_GetConfigLine(EXTI_HandleTypeDef *hexti, EXTI_ConfigTypeDef *pExtiConfig);
HAL_StatusTypeDef HAL_EXTI_ClearConfigLine(EXTI_HandleTypeDef *hexti);
HAL_StatusTypeDef HAL_EXTI_RegisterCallback(EXTI_HandleTypeDef *hexti, EXTI_CallbackIDTypeDef CallbackID, void (*pPendingCbfn)(void));
HAL_StatusTypeDef HAL_EXTI_GetHandle(EXTI_HandleTypeDef *hexti, uint32_t ExtiLine);
/**
  * @}
  */

/** @defgroup EXTI_Exported_Functions_Group2 IO operation functions
  * @brief    IO operation functions
  * @{
  */
/* IO operation functions *****************************************************/
void              HAL_EXTI_IRQHandler(EXTI_HandleTypeDef *hexti);
uint32_t          HAL_EXTI_GetPending(EXTI_HandleTypeDef *hexti, uint32_t Edge);
void              HAL_EXTI_ClearPending(EXTI_HandleTypeDef *hexti, uint32_t Edge);
void              HAL_EXTI_GenerateSWI(EXTI_HandleTypeDef *hexti);

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

#endif /* STM32L4xx_HAL_EXTI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
