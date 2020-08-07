/**
  ******************************************************************************
  * @file    stm32l4xx_hal_lcd.h
  * @author  MCD Application Team
  * @brief   Header file of LCD Controller HAL module.
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
#ifndef STM32L4xx_HAL_LCD_H
#define STM32L4xx_HAL_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(STM32L433xx) || defined(STM32L443xx) || defined(STM32L476xx) || defined(STM32L486xx) || defined(STM32L496xx) || defined(STM32L4A6xx)

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup LCD
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup LCD_Exported_Types LCD Exported Types
  * @{
  */

/**
  * @brief LCD Init structure definition
  */

typedef struct
{
  uint32_t Prescaler;       /*!< Configures the LCD Prescaler.
                                 This parameter can be one value of @ref LCD_Prescaler */
  uint32_t Divider;         /*!< Configures the LCD Divider.
                                 This parameter can be one value of @ref LCD_Divider */
  uint32_t Duty;            /*!< Configures the LCD Duty.
                                 This parameter can be one value of @ref LCD_Duty */
  uint32_t Bias;            /*!< Configures the LCD Bias.
                                 This parameter can be one value of @ref LCD_Bias */
  uint32_t VoltageSource;   /*!< Selects the LCD Voltage source.
                                 This parameter can be one value of @ref LCD_Voltage_Source */
  uint32_t Contrast;        /*!< Configures the LCD Contrast.
                                 This parameter can be one value of @ref LCD_Contrast */
  uint32_t DeadTime;        /*!< Configures the LCD Dead Time.
                                 This parameter can be one value of @ref LCD_DeadTime */
  uint32_t PulseOnDuration; /*!< Configures the LCD Pulse On Duration.
                                 This parameter can be one value of @ref LCD_PulseOnDuration */
  uint32_t HighDrive;       /*!< Enable or disable the low resistance divider.
                                 This parameter can be one value of @ref LCD_HighDrive */
  uint32_t BlinkMode;       /*!< Configures the LCD Blink Mode.
                                 This parameter can be one value of @ref LCD_BlinkMode */
  uint32_t BlinkFrequency;  /*!< Configures the LCD Blink frequency.
                                 This parameter can be one value of @ref LCD_BlinkFrequency */
  uint32_t MuxSegment;      /*!< Enable or disable mux segment.
                                 This parameter can be one value of @ref LCD_MuxSegment */
} LCD_InitTypeDef;

/**
  * @brief HAL LCD State structures definition
  */
typedef enum
{
  HAL_LCD_STATE_RESET             = 0x00,    /*!< Peripheral is not yet Initialized */
  HAL_LCD_STATE_READY             = 0x01,    /*!< Peripheral Initialized and ready for use */
  HAL_LCD_STATE_BUSY              = 0x02,    /*!< an internal process is ongoing */
  HAL_LCD_STATE_TIMEOUT           = 0x03,    /*!< Timeout state */
  HAL_LCD_STATE_ERROR             = 0x04     /*!< Error */
} HAL_LCD_StateTypeDef;

/**
  * @brief  UART handle Structure definition
  */
typedef struct
{
  LCD_TypeDef                   *Instance;  /* LCD registers base address */

  LCD_InitTypeDef               Init;       /* LCD communication parameters */

  HAL_LockTypeDef               Lock;       /* Locking object */

  __IO HAL_LCD_StateTypeDef     State;      /* LCD communication state */

  __IO uint32_t                 ErrorCode;  /* LCD Error code */

} LCD_HandleTypeDef;
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup LCD_Exported_Constants LCD Exported Constants
  * @{
  */

/** @defgroup  LCD_ErrorCode LCD Error Code
  * @{
  */
#define HAL_LCD_ERROR_NONE       (0x00000000U)    /*!< No error */
#define HAL_LCD_ERROR_FCRSF      (0x00000001U)    /*!< Synchro flag timeout error */
#define HAL_LCD_ERROR_UDR        (0x00000002U)    /*!< Update display request flag timeout error */
#define HAL_LCD_ERROR_UDD        (0x00000004U)    /*!< Update display done flag timeout error */
#define HAL_LCD_ERROR_ENS        (0x00000008U)    /*!< LCD enabled status flag timeout error */
#define HAL_LCD_ERROR_RDY        (0x00000010U)    /*!< LCD Booster ready timeout error */
/**
  * @}
  */

/** @defgroup LCD_Prescaler LCD Prescaler
  * @{
  */
#define LCD_PRESCALER_1        (0x00000000U)  /*!< CLKPS = LCDCLK        */
#define LCD_PRESCALER_2        (0x00400000U)  /*!< CLKPS = LCDCLK/2      */
#define LCD_PRESCALER_4        (0x00800000U)  /*!< CLKPS = LCDCLK/4      */
#define LCD_PRESCALER_8        (0x00C00000U)  /*!< CLKPS = LCDCLK/8      */
#define LCD_PRESCALER_16       (0x01000000U)  /*!< CLKPS = LCDCLK/16     */
#define LCD_PRESCALER_32       (0x01400000U)  /*!< CLKPS = LCDCLK/32     */
#define LCD_PRESCALER_64       (0x01800000U)  /*!< CLKPS = LCDCLK/64     */
#define LCD_PRESCALER_128      (0x01C00000U)  /*!< CLKPS = LCDCLK/128    */
#define LCD_PRESCALER_256      (0x02000000U)  /*!< CLKPS = LCDCLK/256    */
#define LCD_PRESCALER_512      (0x02400000U)  /*!< CLKPS = LCDCLK/512    */
#define LCD_PRESCALER_1024     (0x02800000U)  /*!< CLKPS = LCDCLK/1024   */
#define LCD_PRESCALER_2048     (0x02C00000U)  /*!< CLKPS = LCDCLK/2048   */
#define LCD_PRESCALER_4096     (0x03000000U)  /*!< CLKPS = LCDCLK/4096   */
#define LCD_PRESCALER_8192     (0x03400000U)  /*!< CLKPS = LCDCLK/8192   */
#define LCD_PRESCALER_16384    (0x03800000U)  /*!< CLKPS = LCDCLK/16384  */
#define LCD_PRESCALER_32768    (0x03C00000U)  /*!< CLKPS = LCDCLK/32768  */
/**
  * @}
  */

/** @defgroup LCD_Divider LCD Divider
  * @{
  */
#define LCD_DIVIDER_16    (0x00000000U)  /*!< LCD frequency = CLKPS/16 */
#define LCD_DIVIDER_17    (0x00040000U)  /*!< LCD frequency = CLKPS/17 */
#define LCD_DIVIDER_18    (0x00080000U)  /*!< LCD frequency = CLKPS/18 */
#define LCD_DIVIDER_19    (0x000C0000U)  /*!< LCD frequency = CLKPS/19 */
#define LCD_DIVIDER_20    (0x00100000U)  /*!< LCD frequency = CLKPS/20 */
#define LCD_DIVIDER_21    (0x00140000U)  /*!< LCD frequency = CLKPS/21 */
#define LCD_DIVIDER_22    (0x00180000U)  /*!< LCD frequency = CLKPS/22 */
#define LCD_DIVIDER_23    (0x001C0000U)  /*!< LCD frequency = CLKPS/23 */
#define LCD_DIVIDER_24    (0x00200000U)  /*!< LCD frequency = CLKPS/24 */
#define LCD_DIVIDER_25    (0x00240000U)  /*!< LCD frequency = CLKPS/25 */
#define LCD_DIVIDER_26    (0x00280000U)  /*!< LCD frequency = CLKPS/26 */
#define LCD_DIVIDER_27    (0x002C0000U)  /*!< LCD frequency = CLKPS/27 */
#define LCD_DIVIDER_28    (0x00300000U)  /*!< LCD frequency = CLKPS/28 */
#define LCD_DIVIDER_29    (0x00340000U)  /*!< LCD frequency = CLKPS/29 */
#define LCD_DIVIDER_30    (0x00380000U)  /*!< LCD frequency = CLKPS/30 */
#define LCD_DIVIDER_31    (0x003C0000U)  /*!< LCD frequency = CLKPS/31 */
/**
  * @}
  */


/** @defgroup LCD_Duty LCD Duty
  * @{
  */
#define LCD_DUTY_STATIC                 (0x00000000U)                     /*!< Static duty */
#define LCD_DUTY_1_2                    (LCD_CR_DUTY_0)                   /*!< 1/2 duty    */
#define LCD_DUTY_1_3                    (LCD_CR_DUTY_1)                   /*!< 1/3 duty    */
#define LCD_DUTY_1_4                    ((LCD_CR_DUTY_1 | LCD_CR_DUTY_0)) /*!< 1/4 duty    */
#define LCD_DUTY_1_8                    (LCD_CR_DUTY_2)                   /*!< 1/8 duty    */
/**
  * @}
  */


/** @defgroup LCD_Bias LCD Bias
  * @{
  */
#define LCD_BIAS_1_4                    (0x00000000U) /*!< 1/4 Bias */
#define LCD_BIAS_1_2                    LCD_CR_BIAS_0 /*!< 1/2 Bias */
#define LCD_BIAS_1_3                    LCD_CR_BIAS_1 /*!< 1/3 Bias */
/**
  * @}
  */

/** @defgroup LCD_Voltage_Source LCD Voltage Source
  * @{
  */
#define LCD_VOLTAGESOURCE_INTERNAL      (0x00000000U)  /*!< Internal voltage source for the LCD */
#define LCD_VOLTAGESOURCE_EXTERNAL      LCD_CR_VSEL    /*!< External voltage source for the LCD */
/**
  * @}
  */

/** @defgroup LCD_Interrupts LCD Interrupts
  * @{
  */
#define LCD_IT_SOF                      LCD_FCR_SOFIE
#define LCD_IT_UDD                      LCD_FCR_UDDIE
/**
  * @}
  */

/** @defgroup LCD_PulseOnDuration LCD Pulse On Duration
  * @{
  */
#define LCD_PULSEONDURATION_0           (0x00000000U)                   /*!< Pulse ON duration = 0 pulse  */
#define LCD_PULSEONDURATION_1           (LCD_FCR_PON_0)                 /*!< Pulse ON duration = 1/CK_PS  */
#define LCD_PULSEONDURATION_2           (LCD_FCR_PON_1)                 /*!< Pulse ON duration = 2/CK_PS  */
#define LCD_PULSEONDURATION_3           (LCD_FCR_PON_1 | LCD_FCR_PON_0) /*!< Pulse ON duration = 3/CK_PS  */
#define LCD_PULSEONDURATION_4           (LCD_FCR_PON_2)                 /*!< Pulse ON duration = 4/CK_PS  */
#define LCD_PULSEONDURATION_5           (LCD_FCR_PON_2 | LCD_FCR_PON_0) /*!< Pulse ON duration = 5/CK_PS  */
#define LCD_PULSEONDURATION_6           (LCD_FCR_PON_2 | LCD_FCR_PON_1) /*!< Pulse ON duration = 6/CK_PS  */
#define LCD_PULSEONDURATION_7           (LCD_FCR_PON)                   /*!< Pulse ON duration = 7/CK_PS  */
/**
  * @}
  */


/** @defgroup LCD_DeadTime LCD Dead Time
  * @{
  */
#define LCD_DEADTIME_0                  (0x00000000U)                     /*!< No dead Time                                  */
#define LCD_DEADTIME_1                  (LCD_FCR_DEAD_0)                  /*!< One Phase between different couple of Frame   */
#define LCD_DEADTIME_2                  (LCD_FCR_DEAD_1)                  /*!< Two Phase between different couple of Frame   */
#define LCD_DEADTIME_3                  (LCD_FCR_DEAD_1 | LCD_FCR_DEAD_0) /*!< Three Phase between different couple of Frame */
#define LCD_DEADTIME_4                  (LCD_FCR_DEAD_2)                  /*!< Four Phase between different couple of Frame  */
#define LCD_DEADTIME_5                  (LCD_FCR_DEAD_2 | LCD_FCR_DEAD_0) /*!< Five Phase between different couple of Frame  */
#define LCD_DEADTIME_6                  (LCD_FCR_DEAD_2 | LCD_FCR_DEAD_1) /*!< Six Phase between different couple of Frame   */
#define LCD_DEADTIME_7                  (LCD_FCR_DEAD)                    /*!< Seven Phase between different couple of Frame */
/**
  * @}
  */

/** @defgroup LCD_BlinkMode LCD Blink Mode
  * @{
  */
#define LCD_BLINKMODE_OFF               (0x00000000U)           /*!< Blink disabled                                     */
#define LCD_BLINKMODE_SEG0_COM0         (LCD_FCR_BLINK_0)       /*!< Blink enabled on SEG[0], COM[0] (1 pixel)          */
#define LCD_BLINKMODE_SEG0_ALLCOM       (LCD_FCR_BLINK_1)       /*!< Blink enabled on SEG[0], all COM (up to
                                                                    8 pixels according to the programmed duty)          */
#define LCD_BLINKMODE_ALLSEG_ALLCOM     (LCD_FCR_BLINK)         /*!< Blink enabled on all SEG and all COM (all pixels)  */
/**
  * @}
  */

/** @defgroup LCD_BlinkFrequency LCD Blink Frequency
  * @{
  */
#define LCD_BLINKFREQUENCY_DIV8         (0x00000000U)                         /*!< The Blink frequency = fLCD/8    */
#define LCD_BLINKFREQUENCY_DIV16        (LCD_FCR_BLINKF_0)                    /*!< The Blink frequency = fLCD/16   */
#define LCD_BLINKFREQUENCY_DIV32        (LCD_FCR_BLINKF_1)                    /*!< The Blink frequency = fLCD/32   */
#define LCD_BLINKFREQUENCY_DIV64        (LCD_FCR_BLINKF_1 | LCD_FCR_BLINKF_0) /*!< The Blink frequency = fLCD/64   */
#define LCD_BLINKFREQUENCY_DIV128       (LCD_FCR_BLINKF_2)                    /*!< The Blink frequency = fLCD/128  */
#define LCD_BLINKFREQUENCY_DIV256       (LCD_FCR_BLINKF_2 |LCD_FCR_BLINKF_0)  /*!< The Blink frequency = fLCD/256  */
#define LCD_BLINKFREQUENCY_DIV512       (LCD_FCR_BLINKF_2 |LCD_FCR_BLINKF_1)  /*!< The Blink frequency = fLCD/512  */
#define LCD_BLINKFREQUENCY_DIV1024      (LCD_FCR_BLINKF)                      /*!< The Blink frequency = fLCD/1024 */
/**
  * @}
  */

/** @defgroup LCD_Contrast LCD Contrast
  * @{
  */
#define LCD_CONTRASTLEVEL_0               (0x00000000U)                 /*!< Maximum Voltage = 2.60V    */
#define LCD_CONTRASTLEVEL_1               (LCD_FCR_CC_0)                /*!< Maximum Voltage = 2.73V    */
#define LCD_CONTRASTLEVEL_2               (LCD_FCR_CC_1)                /*!< Maximum Voltage = 2.86V    */
#define LCD_CONTRASTLEVEL_3               (LCD_FCR_CC_1 | LCD_FCR_CC_0) /*!< Maximum Voltage = 2.99V    */
#define LCD_CONTRASTLEVEL_4               (LCD_FCR_CC_2)                /*!< Maximum Voltage = 3.12V    */
#define LCD_CONTRASTLEVEL_5               (LCD_FCR_CC_2 | LCD_FCR_CC_0) /*!< Maximum Voltage = 3.26V    */
#define LCD_CONTRASTLEVEL_6               (LCD_FCR_CC_2 | LCD_FCR_CC_1) /*!< Maximum Voltage = 3.40V    */
#define LCD_CONTRASTLEVEL_7               (LCD_FCR_CC)                  /*!< Maximum Voltage = 3.55V    */
/**
  * @}
  */

/** @defgroup LCD_RAMRegister LCD RAMRegister
  * @{
  */
#define LCD_RAM_REGISTER0               (0x00000000U) /*!< LCD RAM Register 0  */
#define LCD_RAM_REGISTER1               (0x00000001U) /*!< LCD RAM Register 1  */
#define LCD_RAM_REGISTER2               (0x00000002U) /*!< LCD RAM Register 2  */
#define LCD_RAM_REGISTER3               (0x00000003U) /*!< LCD RAM Register 3  */
#define LCD_RAM_REGISTER4               (0x00000004U) /*!< LCD RAM Register 4  */
#define LCD_RAM_REGISTER5               (0x00000005U) /*!< LCD RAM Register 5  */
#define LCD_RAM_REGISTER6               (0x00000006U) /*!< LCD RAM Register 6  */
#define LCD_RAM_REGISTER7               (0x00000007U) /*!< LCD RAM Register 7  */
#define LCD_RAM_REGISTER8               (0x00000008U) /*!< LCD RAM Register 8  */
#define LCD_RAM_REGISTER9               (0x00000009U) /*!< LCD RAM Register 9  */
#define LCD_RAM_REGISTER10              (0x0000000AU) /*!< LCD RAM Register 10 */
#define LCD_RAM_REGISTER11              (0x0000000BU) /*!< LCD RAM Register 11 */
#define LCD_RAM_REGISTER12              (0x0000000CU) /*!< LCD RAM Register 12 */
#define LCD_RAM_REGISTER13              (0x0000000DU) /*!< LCD RAM Register 13 */
#define LCD_RAM_REGISTER14              (0x0000000EU) /*!< LCD RAM Register 14 */
#define LCD_RAM_REGISTER15              (0x0000000FU) /*!< LCD RAM Register 15 */
/**
  * @}
  */

/** @defgroup LCD_HighDrive LCD High Drive
  * @{
  */

#define LCD_HIGHDRIVE_DISABLE             ((uint32_t)0x00000000)        /*!< High drive disabled */
#define LCD_HIGHDRIVE_ENABLE              (LCD_FCR_HD)                  /*!< High drive enabled  */
/**
  * @}
  */

/** @defgroup LCD_MuxSegment LCD Mux Segment
  * @{
  */

#define LCD_MUXSEGMENT_DISABLE            (0x00000000U)                 /*!< SEG pin multiplexing disabled                 */
#define LCD_MUXSEGMENT_ENABLE             (LCD_CR_MUX_SEG)              /*!< SEG[31:28] are multiplexed with SEG[43:40]    */
/**
  * @}
  */

/** @defgroup LCD_Flag_Definition LCD Flags Definition
  * @{
  */
#define LCD_FLAG_ENS                    LCD_SR_ENS   /*!< LCD enabled status */
#define LCD_FLAG_SOF                    LCD_SR_SOF   /*!< Start of frame flag */
#define LCD_FLAG_UDR                    LCD_SR_UDR   /*!< Update display request */
#define LCD_FLAG_UDD                    LCD_SR_UDD   /*!< Update display done */
#define LCD_FLAG_RDY                    LCD_SR_RDY   /*!< Ready flag */
#define LCD_FLAG_FCRSF                  LCD_SR_FCRSR /*!< LCD Frame Control Register Synchronization flag */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup LCD_Exported_Macros LCD Exported Macros
  * @{
  */

/** @brief Reset LCD handle state.
  * @param __HANDLE__ specifies the LCD Handle.
  * @retval None
  */
#define __HAL_LCD_RESET_HANDLE_STATE(__HANDLE__)  ((__HANDLE__)->State = HAL_LCD_STATE_RESET)

/** @brief  Enable the LCD peripheral.
  * @param __HANDLE__ specifies the LCD Handle.
  * @retval None
  */
#define __HAL_LCD_ENABLE(__HANDLE__)              SET_BIT((__HANDLE__)->Instance->CR, LCD_CR_LCDEN)

/** @brief  Disable the LCD peripheral.
  * @param __HANDLE__ specifies the LCD Handle.
  * @retval None
  */
#define __HAL_LCD_DISABLE(__HANDLE__)             CLEAR_BIT((__HANDLE__)->Instance->CR, LCD_CR_LCDEN)

/** @brief  Enable the low resistance divider.
  * @param __HANDLE__ specifies the LCD Handle.
  * @note   Displays with high internal resistance may need a longer drive time to
  *         achieve satisfactory contrast. This function is useful in this case if
  *         some additional power consumption can be tolerated.
  * @note   When this mode is enabled, the PulseOn Duration (PON) have to be
  *         programmed to 1/CK_PS (LCD_PULSEONDURATION_1).
  * @retval None
  */
#define __HAL_LCD_HIGHDRIVER_ENABLE(__HANDLE__)                       \
                  do {                                                \
                    SET_BIT((__HANDLE__)->Instance->FCR, LCD_FCR_HD); \
                    LCD_WaitForSynchro(__HANDLE__);                   \
                  } while(0)

/** @brief  Disable the low resistance divider.
  * @param __HANDLE__ specifies the LCD Handle.
  * @retval None
  */
#define __HAL_LCD_HIGHDRIVER_DISABLE(__HANDLE__)                        \
                  do {                                                  \
                    CLEAR_BIT((__HANDLE__)->Instance->FCR, LCD_FCR_HD); \
                    LCD_WaitForSynchro(__HANDLE__);                     \
                  } while(0)

/** @brief  Enable the voltage output buffer for higher driving capability.
  * @param __HANDLE__ specifies the LCD Handle.
  * @retval None
  */
#define __HAL_LCD_VOLTAGE_BUFFER_ENABLE(__HANDLE__)   SET_BIT((__HANDLE__)->Instance->CR, LCD_CR_BUFEN)

/** @brief  Disable the voltage output buffer for higher driving capability.
  * @param __HANDLE__ specifies the LCD Handle.
  * @retval None
  */
#define __HAL_LCD_VOLTAGE_BUFFER_DISABLE(__HANDLE__)  CLEAR_BIT((__HANDLE__)->Instance->CR, LCD_CR_BUFEN)

/**
  * @brief  Configure the LCD pulse on duration.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __DURATION__ specifies the LCD pulse on duration in terms of
  *         CK_PS (prescaled LCD clock period) pulses.
  *   This parameter can be one of the following values:
  *     @arg LCD_PULSEONDURATION_0: 0 pulse
  *     @arg LCD_PULSEONDURATION_1: Pulse ON duration = 1/CK_PS
  *     @arg LCD_PULSEONDURATION_2: Pulse ON duration = 2/CK_PS
  *     @arg LCD_PULSEONDURATION_3: Pulse ON duration = 3/CK_PS
  *     @arg LCD_PULSEONDURATION_4: Pulse ON duration = 4/CK_PS
  *     @arg LCD_PULSEONDURATION_5: Pulse ON duration = 5/CK_PS
  *     @arg LCD_PULSEONDURATION_6: Pulse ON duration = 6/CK_PS
  *     @arg LCD_PULSEONDURATION_7: Pulse ON duration = 7/CK_PS
  * @retval None
  */
#define __HAL_LCD_PULSEONDURATION_CONFIG(__HANDLE__, __DURATION__)                        \
                  do {                                                                    \
                    MODIFY_REG((__HANDLE__)->Instance->FCR, LCD_FCR_PON, (__DURATION__)); \
                    LCD_WaitForSynchro(__HANDLE__);                                       \
                  } while(0)

/**
  * @brief  Configure the LCD dead time.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __DEADTIME__ specifies the LCD dead time.
  *   This parameter can be one of the following values:
  *     @arg LCD_DEADTIME_0: No dead Time
  *     @arg LCD_DEADTIME_1: One Phase between different couple of Frame
  *     @arg LCD_DEADTIME_2: Two Phase between different couple of Frame
  *     @arg LCD_DEADTIME_3: Three Phase between different couple of Frame
  *     @arg LCD_DEADTIME_4: Four Phase between different couple of Frame
  *     @arg LCD_DEADTIME_5: Five Phase between different couple of Frame
  *     @arg LCD_DEADTIME_6: Six Phase between different couple of Frame
  *     @arg LCD_DEADTIME_7: Seven Phase between different couple of Frame
  * @retval None
  */
#define __HAL_LCD_DEADTIME_CONFIG(__HANDLE__, __DEADTIME__)                                \
                  do {                                                                     \
                    MODIFY_REG((__HANDLE__)->Instance->FCR, LCD_FCR_DEAD, (__DEADTIME__)); \
                    LCD_WaitForSynchro(__HANDLE__);                                        \
                  } while(0)

/**
  * @brief  Configure the LCD contrast.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __CONTRAST__ specifies the LCD Contrast.
  *   This parameter can be one of the following values:
  *     @arg LCD_CONTRASTLEVEL_0: Maximum Voltage = 2.60V
  *     @arg LCD_CONTRASTLEVEL_1: Maximum Voltage = 2.73V
  *     @arg LCD_CONTRASTLEVEL_2: Maximum Voltage = 2.86V
  *     @arg LCD_CONTRASTLEVEL_3: Maximum Voltage = 2.99V
  *     @arg LCD_CONTRASTLEVEL_4: Maximum Voltage = 3.12V
  *     @arg LCD_CONTRASTLEVEL_5: Maximum Voltage = 3.25V
  *     @arg LCD_CONTRASTLEVEL_6: Maximum Voltage = 3.38V
  *     @arg LCD_CONTRASTLEVEL_7: Maximum Voltage = 3.51V
  * @retval None
  */
#define __HAL_LCD_CONTRAST_CONFIG(__HANDLE__, __CONTRAST__)                           \
              do {                                                                    \
                MODIFY_REG((__HANDLE__)->Instance->FCR, LCD_FCR_CC, (__CONTRAST__));  \
                LCD_WaitForSynchro(__HANDLE__);                                       \
              } while(0)

/**
  * @brief  Configure the LCD Blink mode and Blink frequency.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __BLINKMODE__ specifies the LCD blink mode.
  *   This parameter can be one of the following values:
  *     @arg LCD_BLINKMODE_OFF:           Blink disabled
  *     @arg LCD_BLINKMODE_SEG0_COM0:     Blink enabled on SEG[0], COM[0] (1 pixel)
  *     @arg LCD_BLINKMODE_SEG0_ALLCOM:   Blink enabled on SEG[0], all COM (up to 8
  *                                       pixels according to the programmed duty)
  *     @arg LCD_BLINKMODE_ALLSEG_ALLCOM: Blink enabled on all SEG and all COM
  *                                       (all pixels)
  * @param __BLINKFREQUENCY__ specifies the LCD blink frequency.
  *     @arg LCD_BLINKFREQUENCY_DIV8:    The Blink frequency = fLcd/8
  *     @arg LCD_BLINKFREQUENCY_DIV16:   The Blink frequency = fLcd/16
  *     @arg LCD_BLINKFREQUENCY_DIV32:   The Blink frequency = fLcd/32
  *     @arg LCD_BLINKFREQUENCY_DIV64:   The Blink frequency = fLcd/64
  *     @arg LCD_BLINKFREQUENCY_DIV128:  The Blink frequency = fLcd/128
  *     @arg LCD_BLINKFREQUENCY_DIV256:  The Blink frequency = fLcd/256
  *     @arg LCD_BLINKFREQUENCY_DIV512:  The Blink frequency = fLcd/512
  *     @arg LCD_BLINKFREQUENCY_DIV1024: The Blink frequency = fLcd/1024
  * @retval None
  */
#define __HAL_LCD_BLINK_CONFIG(__HANDLE__, __BLINKMODE__, __BLINKFREQUENCY__)         \
              do {                                                                    \
                MODIFY_REG((__HANDLE__)->Instance->FCR, (LCD_FCR_BLINKF | LCD_FCR_BLINK), ((__BLINKMODE__) | (__BLINKFREQUENCY__))); \
                LCD_WaitForSynchro(__HANDLE__);                                       \
              } while(0)

/** @brief  Enable the specified LCD interrupt.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __INTERRUPT__ specifies the LCD interrupt source to be enabled.
  *          This parameter can be one of the following values:
  *     @arg LCD_IT_SOF: Start of Frame Interrupt
  *     @arg LCD_IT_UDD: Update Display Done Interrupt
  * @retval None
  */
#define __HAL_LCD_ENABLE_IT(__HANDLE__, __INTERRUPT__)                      \
                  do {                                                      \
                    SET_BIT((__HANDLE__)->Instance->FCR, (__INTERRUPT__));  \
                    LCD_WaitForSynchro(__HANDLE__);                         \
                  } while(0)

/** @brief  Disable the specified LCD interrupt.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __INTERRUPT__ specifies the LCD interrupt source to be disabled.
  *          This parameter can be one of the following values:
  *     @arg LCD_IT_SOF: Start of Frame Interrupt
  *     @arg LCD_IT_UDD: Update Display Done Interrupt
  * @retval None
  */
#define __HAL_LCD_DISABLE_IT(__HANDLE__, __INTERRUPT__)                      \
                  do {                                                       \
                    CLEAR_BIT((__HANDLE__)->Instance->FCR, (__INTERRUPT__)); \
                    LCD_WaitForSynchro(__HANDLE__);                          \
                  } while(0)

/** @brief  Check whether the specified LCD interrupt source is enabled or not.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __IT__ specifies the LCD interrupt source to check.
  *        This parameter can be one of the following values:
  *        @arg LCD_IT_SOF: Start of Frame Interrupt
  *        @arg LCD_IT_UDD: Update Display Done Interrupt.
  * @note If the device is in STOP mode (PCLK not provided) UDD will not
  *          generate an interrupt even if UDDIE = 1.
  *          If the display is not enabled the UDD interrupt will never occur.
  * @retval The state of __IT__ (TRUE or FALSE).
  */
#define __HAL_LCD_GET_IT_SOURCE(__HANDLE__, __IT__) (((__HANDLE__)->Instance->FCR) & (__IT__))

/** @brief  Check whether the specified LCD flag is set or not.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __FLAG__ specifies the flag to check.
  *        This parameter can be one of the following values:
  *        @arg LCD_FLAG_ENS: LCD Enabled flag. It indicates the LCD controller status.
  * @note  The ENS bit is set immediately when the LCDEN bit in the LCD_CR
  *             goes from 0 to 1. On deactivation it reflects the real status of
  *             LCD so it becomes 0 at the end of the last displayed frame.
  *        @arg LCD_FLAG_SOF: Start of Frame flag. This flag is set by hardware at
  *             the beginning of a new frame, at the same time as the display data is
  *             updated.
  *        @arg LCD_FLAG_UDR: Update Display Request flag.
  *        @arg LCD_FLAG_UDD: Update Display Done flag.
  *        @arg LCD_FLAG_RDY: Step_up converter Ready flag. It indicates the status
  *             of the step-up converter.
  *        @arg LCD_FLAG_FCRSF: LCD Frame Control Register Synchronization Flag.
  *             This flag is set by hardware each time the LCD_FCR register is updated
  *             in the LCDCLK domain.
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_LCD_GET_FLAG(__HANDLE__, __FLAG__)    (((__HANDLE__)->Instance->SR & (__FLAG__)) == (__FLAG__))

/** @brief  Clear the specified LCD pending flag.
  * @param __HANDLE__ specifies the LCD Handle.
  * @param __FLAG__ specifies the flag to clear.
  *        This parameter can be any combination of the following values:
  *        @arg LCD_FLAG_SOF: Start of Frame Interrupt
  *        @arg LCD_FLAG_UDD: Update Display Done Interrupt
  * @retval None
  */
#define __HAL_LCD_CLEAR_FLAG(__HANDLE__, __FLAG__)  WRITE_REG((__HANDLE__)->Instance->CLR, (__FLAG__))

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @addtogroup LCD_Exported_Functions
  * @{
  */

/* Initialization/de-initialization methods  **********************************/
/** @addtogroup LCD_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef    HAL_LCD_DeInit(LCD_HandleTypeDef *hlcd);
HAL_StatusTypeDef    HAL_LCD_Init(LCD_HandleTypeDef *hlcd);
void                 HAL_LCD_MspInit(LCD_HandleTypeDef *hlcd);
void                 HAL_LCD_MspDeInit(LCD_HandleTypeDef *hlcd);
/**
  * @}
  */

/* IO operation methods *******************************************************/
/** @addtogroup LCD_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef    HAL_LCD_Write(LCD_HandleTypeDef *hlcd, uint32_t RAMRegisterIndex, uint32_t RAMRegisterMask, uint32_t Data);
HAL_StatusTypeDef    HAL_LCD_Clear(LCD_HandleTypeDef *hlcd);
HAL_StatusTypeDef    HAL_LCD_UpdateDisplayRequest(LCD_HandleTypeDef *hlcd);
/**
  * @}
  */

/* Peripheral State methods  **************************************************/
/** @addtogroup LCD_Exported_Functions_Group3
  * @{
  */
HAL_LCD_StateTypeDef HAL_LCD_GetState(LCD_HandleTypeDef *hlcd);
uint32_t             HAL_LCD_GetError(LCD_HandleTypeDef *hlcd);
/**
  * @}
  */

/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/** @defgroup LCD_Private_Macros LCD Private Macros
  * @{
  */

#define IS_LCD_PRESCALER(__PRESCALER__) (((__PRESCALER__) == LCD_PRESCALER_1)     || \
                                         ((__PRESCALER__) == LCD_PRESCALER_2)     || \
                                         ((__PRESCALER__) == LCD_PRESCALER_4)     || \
                                         ((__PRESCALER__) == LCD_PRESCALER_8)     || \
                                         ((__PRESCALER__) == LCD_PRESCALER_16)    || \
                                         ((__PRESCALER__) == LCD_PRESCALER_32)    || \
                                         ((__PRESCALER__) == LCD_PRESCALER_64)    || \
                                         ((__PRESCALER__) == LCD_PRESCALER_128)   || \
                                         ((__PRESCALER__) == LCD_PRESCALER_256)   || \
                                         ((__PRESCALER__) == LCD_PRESCALER_512)   || \
                                         ((__PRESCALER__) == LCD_PRESCALER_1024)  || \
                                         ((__PRESCALER__) == LCD_PRESCALER_2048)  || \
                                         ((__PRESCALER__) == LCD_PRESCALER_4096)  || \
                                         ((__PRESCALER__) == LCD_PRESCALER_8192)  || \
                                         ((__PRESCALER__) == LCD_PRESCALER_16384) || \
                                         ((__PRESCALER__) == LCD_PRESCALER_32768))

#define IS_LCD_DIVIDER(__DIVIDER__) (((__DIVIDER__) == LCD_DIVIDER_16) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_17) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_18) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_19) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_20) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_21) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_22) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_23) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_24) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_25) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_26) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_27) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_28) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_29) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_30) || \
                                     ((__DIVIDER__) == LCD_DIVIDER_31))

#define IS_LCD_DUTY(__DUTY__) (((__DUTY__) == LCD_DUTY_STATIC)  || \
                               ((__DUTY__) == LCD_DUTY_1_2)     || \
                               ((__DUTY__) == LCD_DUTY_1_3)     || \
                               ((__DUTY__) == LCD_DUTY_1_4)     || \
                               ((__DUTY__) == LCD_DUTY_1_8))

#define IS_LCD_BIAS(__BIAS__) (((__BIAS__) == LCD_BIAS_1_4) || \
                               ((__BIAS__) == LCD_BIAS_1_2) || \
                               ((__BIAS__) == LCD_BIAS_1_3))

#define IS_LCD_VOLTAGE_SOURCE(SOURCE) (((SOURCE) == LCD_VOLTAGESOURCE_INTERNAL) || \
                                       ((SOURCE) == LCD_VOLTAGESOURCE_EXTERNAL))


#define IS_LCD_PULSE_ON_DURATION(__DURATION__) (((__DURATION__) == LCD_PULSEONDURATION_0) || \
                                                ((__DURATION__) == LCD_PULSEONDURATION_1) || \
                                                ((__DURATION__) == LCD_PULSEONDURATION_2) || \
                                                ((__DURATION__) == LCD_PULSEONDURATION_3) || \
                                                ((__DURATION__) == LCD_PULSEONDURATION_4) || \
                                                ((__DURATION__) == LCD_PULSEONDURATION_5) || \
                                                ((__DURATION__) == LCD_PULSEONDURATION_6) || \
                                                ((__DURATION__) == LCD_PULSEONDURATION_7))

#define IS_LCD_DEAD_TIME(__TIME__) (((__TIME__) == LCD_DEADTIME_0) || \
                                    ((__TIME__) == LCD_DEADTIME_1) || \
                                    ((__TIME__) == LCD_DEADTIME_2) || \
                                    ((__TIME__) == LCD_DEADTIME_3) || \
                                    ((__TIME__) == LCD_DEADTIME_4) || \
                                    ((__TIME__) == LCD_DEADTIME_5) || \
                                    ((__TIME__) == LCD_DEADTIME_6) || \
                                    ((__TIME__) == LCD_DEADTIME_7))

#define IS_LCD_BLINK_MODE(__MODE__) (((__MODE__) == LCD_BLINKMODE_OFF)            || \
                                     ((__MODE__) == LCD_BLINKMODE_SEG0_COM0)      || \
                                     ((__MODE__) == LCD_BLINKMODE_SEG0_ALLCOM)    || \
                                     ((__MODE__) == LCD_BLINKMODE_ALLSEG_ALLCOM))

#define IS_LCD_BLINK_FREQUENCY(__FREQUENCY__) (((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV8)   || \
                                               ((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV16)  || \
                                               ((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV32)  || \
                                               ((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV64)  || \
                                               ((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV128) || \
                                               ((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV256) || \
                                               ((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV512) || \
                                               ((__FREQUENCY__) == LCD_BLINKFREQUENCY_DIV1024))

#define IS_LCD_CONTRAST(__CONTRAST__) (((__CONTRAST__) == LCD_CONTRASTLEVEL_0) || \
                                       ((__CONTRAST__) == LCD_CONTRASTLEVEL_1) || \
                                       ((__CONTRAST__) == LCD_CONTRASTLEVEL_2) || \
                                       ((__CONTRAST__) == LCD_CONTRASTLEVEL_3) || \
                                       ((__CONTRAST__) == LCD_CONTRASTLEVEL_4) || \
                                       ((__CONTRAST__) == LCD_CONTRASTLEVEL_5) || \
                                       ((__CONTRAST__) == LCD_CONTRASTLEVEL_6) || \
                                       ((__CONTRAST__) == LCD_CONTRASTLEVEL_7))

#define IS_LCD_RAM_REGISTER(__REGISTER__) (((__REGISTER__) == LCD_RAM_REGISTER0)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER1)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER2)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER3)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER4)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER5)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER6)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER7)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER8)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER9)  || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER10) || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER11) || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER12) || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER13) || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER14) || \
                                           ((__REGISTER__) == LCD_RAM_REGISTER15))

#define IS_LCD_HIGH_DRIVE(__VALUE__) (((__VALUE__) == LCD_HIGHDRIVE_DISABLE) || \
                                      ((__VALUE__) == LCD_HIGHDRIVE_ENABLE))

#define IS_LCD_MUX_SEGMENT(__VALUE__) (((__VALUE__) == LCD_MUXSEGMENT_ENABLE) || \
                                       ((__VALUE__) == LCD_MUXSEGMENT_DISABLE))

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @addtogroup LCD_Private_Functions
  * @{
  */

HAL_StatusTypeDef     LCD_WaitForSynchro(LCD_HandleTypeDef *hlcd);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* STM32L433xx || STM32L443xx || STM32L476xx || STM32L486xx || STM32L496xx || STM32L4A6xx */

#ifdef __cplusplus
}
#endif

#endif /* STM32L4xx_HAL_LCD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
