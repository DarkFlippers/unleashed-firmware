/**
  ******************************************************************************
  * @file    stm32l4xx_hal_i2c.h
  * @author  MCD Application Team
  * @brief   Header file of I2C HAL module.
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
#ifndef STM32L4xx_HAL_I2C_H
#define STM32L4xx_HAL_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup I2C
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup I2C_Exported_Types I2C Exported Types
  * @{
  */

/** @defgroup I2C_Configuration_Structure_definition I2C Configuration Structure definition
  * @brief  I2C Configuration Structure definition
  * @{
  */
typedef struct
{
  uint32_t Timing;              /*!< Specifies the I2C_TIMINGR_register value.
                                  This parameter calculated by referring to I2C initialization
                                         section in Reference manual */

  uint32_t OwnAddress1;         /*!< Specifies the first device own address.
                                  This parameter can be a 7-bit or 10-bit address. */

  uint32_t AddressingMode;      /*!< Specifies if 7-bit or 10-bit addressing mode is selected.
                                  This parameter can be a value of @ref I2C_ADDRESSING_MODE */

  uint32_t DualAddressMode;     /*!< Specifies if dual addressing mode is selected.
                                  This parameter can be a value of @ref I2C_DUAL_ADDRESSING_MODE */

  uint32_t OwnAddress2;         /*!< Specifies the second device own address if dual addressing mode is selected
                                  This parameter can be a 7-bit address. */

  uint32_t OwnAddress2Masks;    /*!< Specifies the acknowledge mask address second device own address if dual addressing mode is selected
                                  This parameter can be a value of @ref I2C_OWN_ADDRESS2_MASKS */

  uint32_t GeneralCallMode;     /*!< Specifies if general call mode is selected.
                                  This parameter can be a value of @ref I2C_GENERAL_CALL_ADDRESSING_MODE */

  uint32_t NoStretchMode;       /*!< Specifies if nostretch mode is selected.
                                  This parameter can be a value of @ref I2C_NOSTRETCH_MODE */

} I2C_InitTypeDef;

/**
  * @}
  */

/** @defgroup HAL_state_structure_definition HAL state structure definition
  * @brief  HAL State structure definition
  * @note  HAL I2C State value coding follow below described bitmap :\n
  *          b7-b6  Error information\n
  *             00 : No Error\n
  *             01 : Abort (Abort user request on going)\n
  *             10 : Timeout\n
  *             11 : Error\n
  *          b5     Peripheral initialization status\n
  *             0  : Reset (peripheral not initialized)\n
  *             1  : Init done (peripheral initialized and ready to use. HAL I2C Init function called)\n
  *          b4     (not used)\n
  *             x  : Should be set to 0\n
  *          b3\n
  *             0  : Ready or Busy (No Listen mode ongoing)\n
  *             1  : Listen (peripheral in Address Listen Mode)\n
  *          b2     Intrinsic process state\n
  *             0  : Ready\n
  *             1  : Busy (peripheral busy with some configuration or internal operations)\n
  *          b1     Rx state\n
  *             0  : Ready (no Rx operation ongoing)\n
  *             1  : Busy (Rx operation ongoing)\n
  *          b0     Tx state\n
  *             0  : Ready (no Tx operation ongoing)\n
  *             1  : Busy (Tx operation ongoing)
  * @{
  */
typedef enum
{
  HAL_I2C_STATE_RESET             = 0x00U,   /*!< Peripheral is not yet Initialized         */
  HAL_I2C_STATE_READY             = 0x20U,   /*!< Peripheral Initialized and ready for use  */
  HAL_I2C_STATE_BUSY              = 0x24U,   /*!< An internal process is ongoing            */
  HAL_I2C_STATE_BUSY_TX           = 0x21U,   /*!< Data Transmission process is ongoing      */
  HAL_I2C_STATE_BUSY_RX           = 0x22U,   /*!< Data Reception process is ongoing         */
  HAL_I2C_STATE_LISTEN            = 0x28U,   /*!< Address Listen Mode is ongoing            */
  HAL_I2C_STATE_BUSY_TX_LISTEN    = 0x29U,   /*!< Address Listen Mode and Data Transmission
                                                 process is ongoing                         */
  HAL_I2C_STATE_BUSY_RX_LISTEN    = 0x2AU,   /*!< Address Listen Mode and Data Reception
                                                 process is ongoing                         */
  HAL_I2C_STATE_ABORT             = 0x60U,   /*!< Abort user request ongoing                */
  HAL_I2C_STATE_TIMEOUT           = 0xA0U,   /*!< Timeout state                             */
  HAL_I2C_STATE_ERROR             = 0xE0U    /*!< Error                                     */

} HAL_I2C_StateTypeDef;

/**
  * @}
  */

/** @defgroup HAL_mode_structure_definition HAL mode structure definition
  * @brief  HAL Mode structure definition
  * @note  HAL I2C Mode value coding follow below described bitmap :\n
  *          b7     (not used)\n
  *             x  : Should be set to 0\n
  *          b6\n
  *             0  : None\n
  *             1  : Memory (HAL I2C communication is in Memory Mode)\n
  *          b5\n
  *             0  : None\n
  *             1  : Slave (HAL I2C communication is in Slave Mode)\n
  *          b4\n
  *             0  : None\n
  *             1  : Master (HAL I2C communication is in Master Mode)\n
  *          b3-b2-b1-b0  (not used)\n
  *             xxxx : Should be set to 0000
  * @{
  */
typedef enum
{
  HAL_I2C_MODE_NONE               = 0x00U,   /*!< No I2C communication on going             */
  HAL_I2C_MODE_MASTER             = 0x10U,   /*!< I2C communication is in Master Mode       */
  HAL_I2C_MODE_SLAVE              = 0x20U,   /*!< I2C communication is in Slave Mode        */
  HAL_I2C_MODE_MEM                = 0x40U    /*!< I2C communication is in Memory Mode       */

} HAL_I2C_ModeTypeDef;

/**
  * @}
  */

/** @defgroup I2C_Error_Code_definition I2C Error Code definition
  * @brief  I2C Error Code definition
  * @{
  */
#define HAL_I2C_ERROR_NONE      (0x00000000U)    /*!< No error              */
#define HAL_I2C_ERROR_BERR      (0x00000001U)    /*!< BERR error            */
#define HAL_I2C_ERROR_ARLO      (0x00000002U)    /*!< ARLO error            */
#define HAL_I2C_ERROR_AF        (0x00000004U)    /*!< ACKF error            */
#define HAL_I2C_ERROR_OVR       (0x00000008U)    /*!< OVR error             */
#define HAL_I2C_ERROR_DMA       (0x00000010U)    /*!< DMA transfer error    */
#define HAL_I2C_ERROR_TIMEOUT   (0x00000020U)    /*!< Timeout error         */
#define HAL_I2C_ERROR_SIZE      (0x00000040U)    /*!< Size Management error */
#define HAL_I2C_ERROR_DMA_PARAM (0x00000080U)    /*!< DMA Parameter Error   */
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
#define HAL_I2C_ERROR_INVALID_CALLBACK  (0x00000100U)    /*!< Invalid Callback error */
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
#define HAL_I2C_ERROR_INVALID_PARAM     (0x00000200U)    /*!< Invalid Parameters error  */
/**
  * @}
  */

/** @defgroup I2C_handle_Structure_definition I2C handle Structure definition
  * @brief  I2C handle Structure definition
  * @{
  */
typedef struct __I2C_HandleTypeDef
{
  I2C_TypeDef                *Instance;      /*!< I2C registers base address                */

  I2C_InitTypeDef            Init;           /*!< I2C communication parameters              */

  uint8_t                    *pBuffPtr;      /*!< Pointer to I2C transfer buffer            */

  uint16_t                   XferSize;       /*!< I2C transfer size                         */

  __IO uint16_t              XferCount;      /*!< I2C transfer counter                      */

  __IO uint32_t              XferOptions;    /*!< I2C sequantial transfer options, this parameter can
                                                  be a value of @ref I2C_XFEROPTIONS */

  __IO uint32_t              PreviousState;  /*!< I2C communication Previous state          */

  HAL_StatusTypeDef(*XferISR)(struct __I2C_HandleTypeDef *hi2c, uint32_t ITFlags, uint32_t ITSources);  /*!< I2C transfer IRQ handler function pointer */

  DMA_HandleTypeDef          *hdmatx;        /*!< I2C Tx DMA handle parameters              */

  DMA_HandleTypeDef          *hdmarx;        /*!< I2C Rx DMA handle parameters              */

  HAL_LockTypeDef            Lock;           /*!< I2C locking object                        */

  __IO HAL_I2C_StateTypeDef  State;          /*!< I2C communication state                   */

  __IO HAL_I2C_ModeTypeDef   Mode;           /*!< I2C communication mode                    */

  __IO uint32_t              ErrorCode;      /*!< I2C Error code                            */

  __IO uint32_t              AddrEventCount; /*!< I2C Address Event counter                 */

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
  void (* MasterTxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);           /*!< I2C Master Tx Transfer completed callback */
  void (* MasterRxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);           /*!< I2C Master Rx Transfer completed callback */
  void (* SlaveTxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);            /*!< I2C Slave Tx Transfer completed callback  */
  void (* SlaveRxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);            /*!< I2C Slave Rx Transfer completed callback  */
  void (* ListenCpltCallback)(struct __I2C_HandleTypeDef *hi2c);             /*!< I2C Listen Complete callback              */
  void (* MemTxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Memory Tx Transfer completed callback */
  void (* MemRxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Memory Rx Transfer completed callback */
  void (* ErrorCallback)(struct __I2C_HandleTypeDef *hi2c);                  /*!< I2C Error callback                        */
  void (* AbortCpltCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Abort callback                        */

  void (* AddrCallback)(struct __I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode);  /*!< I2C Slave Address Match callback */

  void (* MspInitCallback)(struct __I2C_HandleTypeDef *hi2c);                /*!< I2C Msp Init callback                     */
  void (* MspDeInitCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Msp DeInit callback                   */

#endif  /* USE_HAL_I2C_REGISTER_CALLBACKS */
} I2C_HandleTypeDef;

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL I2C Callback ID enumeration definition
  */
typedef enum
{
  HAL_I2C_MASTER_TX_COMPLETE_CB_ID      = 0x00U,    /*!< I2C Master Tx Transfer completed callback ID  */
  HAL_I2C_MASTER_RX_COMPLETE_CB_ID      = 0x01U,    /*!< I2C Master Rx Transfer completed callback ID  */
  HAL_I2C_SLAVE_TX_COMPLETE_CB_ID       = 0x02U,    /*!< I2C Slave Tx Transfer completed callback ID   */
  HAL_I2C_SLAVE_RX_COMPLETE_CB_ID       = 0x03U,    /*!< I2C Slave Rx Transfer completed callback ID   */
  HAL_I2C_LISTEN_COMPLETE_CB_ID         = 0x04U,    /*!< I2C Listen Complete callback ID               */
  HAL_I2C_MEM_TX_COMPLETE_CB_ID         = 0x05U,    /*!< I2C Memory Tx Transfer callback ID            */
  HAL_I2C_MEM_RX_COMPLETE_CB_ID         = 0x06U,    /*!< I2C Memory Rx Transfer completed callback ID  */
  HAL_I2C_ERROR_CB_ID                   = 0x07U,    /*!< I2C Error callback ID                         */
  HAL_I2C_ABORT_CB_ID                   = 0x08U,    /*!< I2C Abort callback ID                         */

  HAL_I2C_MSPINIT_CB_ID                 = 0x09U,    /*!< I2C Msp Init callback ID                      */
  HAL_I2C_MSPDEINIT_CB_ID               = 0x0AU     /*!< I2C Msp DeInit callback ID                    */

} HAL_I2C_CallbackIDTypeDef;

/**
  * @brief  HAL I2C Callback pointer definition
  */
typedef  void (*pI2C_CallbackTypeDef)(I2C_HandleTypeDef *hi2c); /*!< pointer to an I2C callback function */
typedef  void (*pI2C_AddrCallbackTypeDef)(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode); /*!< pointer to an I2C Address Match callback function */

#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
/**
  * @}
  */

/**
  * @}
  */
/* Exported constants --------------------------------------------------------*/

/** @defgroup I2C_Exported_Constants I2C Exported Constants
  * @{
  */

/** @defgroup I2C_XFEROPTIONS  I2C Sequential Transfer Options
  * @{
  */
#define I2C_FIRST_FRAME                 ((uint32_t)I2C_SOFTEND_MODE)
#define I2C_FIRST_AND_NEXT_FRAME        ((uint32_t)(I2C_RELOAD_MODE | I2C_SOFTEND_MODE))
#define I2C_NEXT_FRAME                  ((uint32_t)(I2C_RELOAD_MODE | I2C_SOFTEND_MODE))
#define I2C_FIRST_AND_LAST_FRAME        ((uint32_t)I2C_AUTOEND_MODE)
#define I2C_LAST_FRAME                  ((uint32_t)I2C_AUTOEND_MODE)
#define I2C_LAST_FRAME_NO_STOP          ((uint32_t)I2C_SOFTEND_MODE)

/* List of XferOptions in usage of :
 * 1- Restart condition in all use cases (direction change or not)
 */
#define  I2C_OTHER_FRAME                (0x000000AAU)
#define  I2C_OTHER_AND_LAST_FRAME       (0x0000AA00U)
/**
  * @}
  */

/** @defgroup I2C_ADDRESSING_MODE I2C Addressing Mode
  * @{
  */
#define I2C_ADDRESSINGMODE_7BIT         (0x00000001U)
#define I2C_ADDRESSINGMODE_10BIT        (0x00000002U)
/**
  * @}
  */

/** @defgroup I2C_DUAL_ADDRESSING_MODE I2C Dual Addressing Mode
  * @{
  */
#define I2C_DUALADDRESS_DISABLE         (0x00000000U)
#define I2C_DUALADDRESS_ENABLE          I2C_OAR2_OA2EN
/**
  * @}
  */

/** @defgroup I2C_OWN_ADDRESS2_MASKS I2C Own Address2 Masks
  * @{
  */
#define I2C_OA2_NOMASK                  ((uint8_t)0x00U)
#define I2C_OA2_MASK01                  ((uint8_t)0x01U)
#define I2C_OA2_MASK02                  ((uint8_t)0x02U)
#define I2C_OA2_MASK03                  ((uint8_t)0x03U)
#define I2C_OA2_MASK04                  ((uint8_t)0x04U)
#define I2C_OA2_MASK05                  ((uint8_t)0x05U)
#define I2C_OA2_MASK06                  ((uint8_t)0x06U)
#define I2C_OA2_MASK07                  ((uint8_t)0x07U)
/**
  * @}
  */

/** @defgroup I2C_GENERAL_CALL_ADDRESSING_MODE I2C General Call Addressing Mode
  * @{
  */
#define I2C_GENERALCALL_DISABLE         (0x00000000U)
#define I2C_GENERALCALL_ENABLE          I2C_CR1_GCEN
/**
  * @}
  */

/** @defgroup I2C_NOSTRETCH_MODE I2C No-Stretch Mode
  * @{
  */
#define I2C_NOSTRETCH_DISABLE           (0x00000000U)
#define I2C_NOSTRETCH_ENABLE            I2C_CR1_NOSTRETCH
/**
  * @}
  */

/** @defgroup I2C_MEMORY_ADDRESS_SIZE I2C Memory Address Size
  * @{
  */
#define I2C_MEMADD_SIZE_8BIT            (0x00000001U)
#define I2C_MEMADD_SIZE_16BIT           (0x00000002U)
/**
  * @}
  */

/** @defgroup I2C_XFERDIRECTION I2C Transfer Direction Master Point of View
  * @{
  */
#define I2C_DIRECTION_TRANSMIT          (0x00000000U)
#define I2C_DIRECTION_RECEIVE           (0x00000001U)
/**
  * @}
  */

/** @defgroup I2C_RELOAD_END_MODE I2C Reload End Mode
  * @{
  */
#define  I2C_RELOAD_MODE                I2C_CR2_RELOAD
#define  I2C_AUTOEND_MODE               I2C_CR2_AUTOEND
#define  I2C_SOFTEND_MODE               (0x00000000U)
/**
  * @}
  */

/** @defgroup I2C_START_STOP_MODE I2C Start or Stop Mode
  * @{
  */
#define  I2C_NO_STARTSTOP               (0x00000000U)
#define  I2C_GENERATE_STOP              (uint32_t)(0x80000000U | I2C_CR2_STOP)
#define  I2C_GENERATE_START_READ        (uint32_t)(0x80000000U | I2C_CR2_START | I2C_CR2_RD_WRN)
#define  I2C_GENERATE_START_WRITE       (uint32_t)(0x80000000U | I2C_CR2_START)
/**
  * @}
  */

/** @defgroup I2C_Interrupt_configuration_definition I2C Interrupt configuration definition
  * @brief I2C Interrupt definition
  *        Elements values convention: 0xXXXXXXXX
  *           - XXXXXXXX  : Interrupt control mask
  * @{
  */
#define I2C_IT_ERRI                     I2C_CR1_ERRIE
#define I2C_IT_TCI                      I2C_CR1_TCIE
#define I2C_IT_STOPI                    I2C_CR1_STOPIE
#define I2C_IT_NACKI                    I2C_CR1_NACKIE
#define I2C_IT_ADDRI                    I2C_CR1_ADDRIE
#define I2C_IT_RXI                      I2C_CR1_RXIE
#define I2C_IT_TXI                      I2C_CR1_TXIE
/**
  * @}
  */

/** @defgroup I2C_Flag_definition I2C Flag definition
  * @{
  */
#define I2C_FLAG_TXE                    I2C_ISR_TXE
#define I2C_FLAG_TXIS                   I2C_ISR_TXIS
#define I2C_FLAG_RXNE                   I2C_ISR_RXNE
#define I2C_FLAG_ADDR                   I2C_ISR_ADDR
#define I2C_FLAG_AF                     I2C_ISR_NACKF
#define I2C_FLAG_STOPF                  I2C_ISR_STOPF
#define I2C_FLAG_TC                     I2C_ISR_TC
#define I2C_FLAG_TCR                    I2C_ISR_TCR
#define I2C_FLAG_BERR                   I2C_ISR_BERR
#define I2C_FLAG_ARLO                   I2C_ISR_ARLO
#define I2C_FLAG_OVR                    I2C_ISR_OVR
#define I2C_FLAG_PECERR                 I2C_ISR_PECERR
#define I2C_FLAG_TIMEOUT                I2C_ISR_TIMEOUT
#define I2C_FLAG_ALERT                  I2C_ISR_ALERT
#define I2C_FLAG_BUSY                   I2C_ISR_BUSY
#define I2C_FLAG_DIR                    I2C_ISR_DIR
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/

/** @defgroup I2C_Exported_Macros I2C Exported Macros
  * @{
  */

/** @brief Reset I2C handle state.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @retval None
  */
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
#define __HAL_I2C_RESET_HANDLE_STATE(__HANDLE__)                do{                                                   \
                                                                    (__HANDLE__)->State = HAL_I2C_STATE_RESET;       \
                                                                    (__HANDLE__)->MspInitCallback = NULL;            \
                                                                    (__HANDLE__)->MspDeInitCallback = NULL;          \
                                                                  } while(0)
#else
#define __HAL_I2C_RESET_HANDLE_STATE(__HANDLE__)                ((__HANDLE__)->State = HAL_I2C_STATE_RESET)
#endif

/** @brief  Enable the specified I2C interrupt.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @param  __INTERRUPT__ specifies the interrupt source to enable.
  *        This parameter can be one of the following values:
  *            @arg @ref I2C_IT_ERRI  Errors interrupt enable
  *            @arg @ref I2C_IT_TCI   Transfer complete interrupt enable
  *            @arg @ref I2C_IT_STOPI STOP detection interrupt enable
  *            @arg @ref I2C_IT_NACKI NACK received interrupt enable
  *            @arg @ref I2C_IT_ADDRI Address match interrupt enable
  *            @arg @ref I2C_IT_RXI   RX interrupt enable
  *            @arg @ref I2C_IT_TXI   TX interrupt enable
  *
  * @retval None
  */
#define __HAL_I2C_ENABLE_IT(__HANDLE__, __INTERRUPT__)          ((__HANDLE__)->Instance->CR1 |= (__INTERRUPT__))

/** @brief  Disable the specified I2C interrupt.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @param  __INTERRUPT__ specifies the interrupt source to disable.
  *        This parameter can be one of the following values:
  *            @arg @ref I2C_IT_ERRI  Errors interrupt enable
  *            @arg @ref I2C_IT_TCI   Transfer complete interrupt enable
  *            @arg @ref I2C_IT_STOPI STOP detection interrupt enable
  *            @arg @ref I2C_IT_NACKI NACK received interrupt enable
  *            @arg @ref I2C_IT_ADDRI Address match interrupt enable
  *            @arg @ref I2C_IT_RXI   RX interrupt enable
  *            @arg @ref I2C_IT_TXI   TX interrupt enable
  *
  * @retval None
  */
#define __HAL_I2C_DISABLE_IT(__HANDLE__, __INTERRUPT__)         ((__HANDLE__)->Instance->CR1 &= (~(__INTERRUPT__)))

/** @brief  Check whether the specified I2C interrupt source is enabled or not.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @param  __INTERRUPT__ specifies the I2C interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg @ref I2C_IT_ERRI  Errors interrupt enable
  *            @arg @ref I2C_IT_TCI   Transfer complete interrupt enable
  *            @arg @ref I2C_IT_STOPI STOP detection interrupt enable
  *            @arg @ref I2C_IT_NACKI NACK received interrupt enable
  *            @arg @ref I2C_IT_ADDRI Address match interrupt enable
  *            @arg @ref I2C_IT_RXI   RX interrupt enable
  *            @arg @ref I2C_IT_TXI   TX interrupt enable
  *
  * @retval The new state of __INTERRUPT__ (SET or RESET).
  */
#define __HAL_I2C_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)      ((((__HANDLE__)->Instance->CR1 & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)

/** @brief  Check whether the specified I2C flag is set or not.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @param  __FLAG__ specifies the flag to check.
  *        This parameter can be one of the following values:
  *            @arg @ref I2C_FLAG_TXE     Transmit data register empty
  *            @arg @ref I2C_FLAG_TXIS    Transmit interrupt status
  *            @arg @ref I2C_FLAG_RXNE    Receive data register not empty
  *            @arg @ref I2C_FLAG_ADDR    Address matched (slave mode)
  *            @arg @ref I2C_FLAG_AF      Acknowledge failure received flag
  *            @arg @ref I2C_FLAG_STOPF   STOP detection flag
  *            @arg @ref I2C_FLAG_TC      Transfer complete (master mode)
  *            @arg @ref I2C_FLAG_TCR     Transfer complete reload
  *            @arg @ref I2C_FLAG_BERR    Bus error
  *            @arg @ref I2C_FLAG_ARLO    Arbitration lost
  *            @arg @ref I2C_FLAG_OVR     Overrun/Underrun
  *            @arg @ref I2C_FLAG_PECERR  PEC error in reception
  *            @arg @ref I2C_FLAG_TIMEOUT Timeout or Tlow detection flag
  *            @arg @ref I2C_FLAG_ALERT   SMBus alert
  *            @arg @ref I2C_FLAG_BUSY    Bus busy
  *            @arg @ref I2C_FLAG_DIR     Transfer direction (slave mode)
  *
  * @retval The new state of __FLAG__ (SET or RESET).
  */
#define I2C_FLAG_MASK  (0x0001FFFFU)
#define __HAL_I2C_GET_FLAG(__HANDLE__, __FLAG__) (((((__HANDLE__)->Instance->ISR) & (__FLAG__)) == (__FLAG__)) ? SET : RESET)

/** @brief  Clear the I2C pending flags which are cleared by writing 1 in a specific bit.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @param  __FLAG__ specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg @ref I2C_FLAG_TXE     Transmit data register empty
  *            @arg @ref I2C_FLAG_ADDR    Address matched (slave mode)
  *            @arg @ref I2C_FLAG_AF      Acknowledge failure received flag
  *            @arg @ref I2C_FLAG_STOPF   STOP detection flag
  *            @arg @ref I2C_FLAG_BERR    Bus error
  *            @arg @ref I2C_FLAG_ARLO    Arbitration lost
  *            @arg @ref I2C_FLAG_OVR     Overrun/Underrun
  *            @arg @ref I2C_FLAG_PECERR  PEC error in reception
  *            @arg @ref I2C_FLAG_TIMEOUT Timeout or Tlow detection flag
  *            @arg @ref I2C_FLAG_ALERT   SMBus alert
  *
  * @retval None
  */
#define __HAL_I2C_CLEAR_FLAG(__HANDLE__, __FLAG__) (((__FLAG__) == I2C_FLAG_TXE) ? ((__HANDLE__)->Instance->ISR |= (__FLAG__)) \
                                                                                 : ((__HANDLE__)->Instance->ICR = (__FLAG__)))

/** @brief  Enable the specified I2C peripheral.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @retval None
  */
#define __HAL_I2C_ENABLE(__HANDLE__)                            (SET_BIT((__HANDLE__)->Instance->CR1,  I2C_CR1_PE))

/** @brief  Disable the specified I2C peripheral.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @retval None
  */
#define __HAL_I2C_DISABLE(__HANDLE__)                           (CLEAR_BIT((__HANDLE__)->Instance->CR1, I2C_CR1_PE))

/** @brief  Generate a Non-Acknowledge I2C peripheral in Slave mode.
  * @param  __HANDLE__ specifies the I2C Handle.
  * @retval None
  */
#define __HAL_I2C_GENERATE_NACK(__HANDLE__)                     (SET_BIT((__HANDLE__)->Instance->CR2, I2C_CR2_NACK))
/**
  * @}
  */

/* Include I2C HAL Extended module */
#include "stm32l4xx_hal_i2c_ex.h"

/* Exported functions --------------------------------------------------------*/
/** @addtogroup I2C_Exported_Functions
  * @{
  */

/** @addtogroup I2C_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
/* Initialization and de-initialization functions******************************/
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c);

/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef HAL_I2C_RegisterCallback(I2C_HandleTypeDef *hi2c, HAL_I2C_CallbackIDTypeDef CallbackID, pI2C_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_I2C_UnRegisterCallback(I2C_HandleTypeDef *hi2c, HAL_I2C_CallbackIDTypeDef CallbackID);

HAL_StatusTypeDef HAL_I2C_RegisterAddrCallback(I2C_HandleTypeDef *hi2c, pI2C_AddrCallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_I2C_UnRegisterAddrCallback(I2C_HandleTypeDef *hi2c);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @addtogroup I2C_Exported_Functions_Group2 Input and Output operation functions
  * @{
  */
/* IO operation functions  ****************************************************/
/******* Blocking mode: Polling */
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Slave_Transmit(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Slave_Receive(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

/******* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Slave_Transmit_IT(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Slave_Receive_IT(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

HAL_StatusTypeDef HAL_I2C_Master_Seq_Transmit_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
HAL_StatusTypeDef HAL_I2C_Master_Seq_Receive_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
HAL_StatusTypeDef HAL_I2C_Slave_Seq_Transmit_IT(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
HAL_StatusTypeDef HAL_I2C_Slave_Seq_Receive_IT(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
HAL_StatusTypeDef HAL_I2C_EnableListen_IT(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_DisableListen_IT(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_Master_Abort_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress);

/******* Non-Blocking mode: DMA */
HAL_StatusTypeDef HAL_I2C_Master_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Master_Receive_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Slave_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Slave_Receive_DMA(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Write_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

HAL_StatusTypeDef HAL_I2C_Master_Seq_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
HAL_StatusTypeDef HAL_I2C_Master_Seq_Receive_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
HAL_StatusTypeDef HAL_I2C_Slave_Seq_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
HAL_StatusTypeDef HAL_I2C_Slave_Seq_Receive_DMA(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions);
/**
  * @}
  */

/** @addtogroup I2C_IRQ_Handler_and_Callbacks IRQ Handler and Callbacks
 * @{
 */
/******* I2C IRQHandler and Callbacks used in non blocking modes (Interrupt and DMA) */
void HAL_I2C_EV_IRQHandler(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ER_IRQHandler(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode);
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c);
/**
  * @}
  */

/** @addtogroup I2C_Exported_Functions_Group3 Peripheral State, Mode and Error functions
  * @{
  */
/* Peripheral State, Mode and Error functions  *********************************/
HAL_I2C_StateTypeDef HAL_I2C_GetState(I2C_HandleTypeDef *hi2c);
HAL_I2C_ModeTypeDef  HAL_I2C_GetMode(I2C_HandleTypeDef *hi2c);
uint32_t             HAL_I2C_GetError(I2C_HandleTypeDef *hi2c);

/**
  * @}
  */

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup I2C_Private_Constants I2C Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup I2C_Private_Macro I2C Private Macros
  * @{
  */

#define IS_I2C_ADDRESSING_MODE(MODE)    (((MODE) == I2C_ADDRESSINGMODE_7BIT) || \
                                         ((MODE) == I2C_ADDRESSINGMODE_10BIT))

#define IS_I2C_DUAL_ADDRESS(ADDRESS)    (((ADDRESS) == I2C_DUALADDRESS_DISABLE) || \
                                         ((ADDRESS) == I2C_DUALADDRESS_ENABLE))

#define IS_I2C_OWN_ADDRESS2_MASK(MASK)  (((MASK) == I2C_OA2_NOMASK)  || \
                                         ((MASK) == I2C_OA2_MASK01) || \
                                         ((MASK) == I2C_OA2_MASK02) || \
                                         ((MASK) == I2C_OA2_MASK03) || \
                                         ((MASK) == I2C_OA2_MASK04) || \
                                         ((MASK) == I2C_OA2_MASK05) || \
                                         ((MASK) == I2C_OA2_MASK06) || \
                                         ((MASK) == I2C_OA2_MASK07))

#define IS_I2C_GENERAL_CALL(CALL)       (((CALL) == I2C_GENERALCALL_DISABLE) || \
                                         ((CALL) == I2C_GENERALCALL_ENABLE))

#define IS_I2C_NO_STRETCH(STRETCH)      (((STRETCH) == I2C_NOSTRETCH_DISABLE) || \
                                         ((STRETCH) == I2C_NOSTRETCH_ENABLE))

#define IS_I2C_MEMADD_SIZE(SIZE)        (((SIZE) == I2C_MEMADD_SIZE_8BIT) || \
                                         ((SIZE) == I2C_MEMADD_SIZE_16BIT))

#define IS_TRANSFER_MODE(MODE)          (((MODE) == I2C_RELOAD_MODE)   || \
                                         ((MODE) == I2C_AUTOEND_MODE) || \
                                         ((MODE) == I2C_SOFTEND_MODE))

#define IS_TRANSFER_REQUEST(REQUEST)    (((REQUEST) == I2C_GENERATE_STOP)        || \
                                         ((REQUEST) == I2C_GENERATE_START_READ)  || \
                                         ((REQUEST) == I2C_GENERATE_START_WRITE) || \
                                         ((REQUEST) == I2C_NO_STARTSTOP))

#define IS_I2C_TRANSFER_OPTIONS_REQUEST(REQUEST)  (((REQUEST) == I2C_FIRST_FRAME)          || \
                                                   ((REQUEST) == I2C_FIRST_AND_NEXT_FRAME) || \
                                                   ((REQUEST) == I2C_NEXT_FRAME)           || \
                                                   ((REQUEST) == I2C_FIRST_AND_LAST_FRAME) || \
                                                   ((REQUEST) == I2C_LAST_FRAME)           || \
                                                   ((REQUEST) == I2C_LAST_FRAME_NO_STOP)   || \
                                                   IS_I2C_TRANSFER_OTHER_OPTIONS_REQUEST(REQUEST))

#define IS_I2C_TRANSFER_OTHER_OPTIONS_REQUEST(REQUEST) (((REQUEST) == I2C_OTHER_FRAME)     || \
                                                        ((REQUEST) == I2C_OTHER_AND_LAST_FRAME))

#define I2C_RESET_CR2(__HANDLE__)                 ((__HANDLE__)->Instance->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN)))

#define I2C_GET_ADDR_MATCH(__HANDLE__)            ((uint16_t)(((__HANDLE__)->Instance->ISR & I2C_ISR_ADDCODE) >> 16U))
#define I2C_GET_DIR(__HANDLE__)                   ((uint8_t)(((__HANDLE__)->Instance->ISR & I2C_ISR_DIR) >> 16U))
#define I2C_GET_STOP_MODE(__HANDLE__)             ((__HANDLE__)->Instance->CR2 & I2C_CR2_AUTOEND)
#define I2C_GET_OWN_ADDRESS1(__HANDLE__)          ((uint16_t)((__HANDLE__)->Instance->OAR1 & I2C_OAR1_OA1))
#define I2C_GET_OWN_ADDRESS2(__HANDLE__)          ((uint16_t)((__HANDLE__)->Instance->OAR2 & I2C_OAR2_OA2))

#define IS_I2C_OWN_ADDRESS1(ADDRESS1)             ((ADDRESS1) <= 0x000003FFU)
#define IS_I2C_OWN_ADDRESS2(ADDRESS2)             ((ADDRESS2) <= (uint16_t)0x00FFU)

#define I2C_MEM_ADD_MSB(__ADDRESS__)              ((uint8_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)(0xFF00U))) >> 8U)))
#define I2C_MEM_ADD_LSB(__ADDRESS__)              ((uint8_t)((uint16_t)((__ADDRESS__) & (uint16_t)(0x00FFU))))

#define I2C_GENERATE_START(__ADDMODE__,__ADDRESS__) (((__ADDMODE__) == I2C_ADDRESSINGMODE_7BIT) ? (uint32_t)((((uint32_t)(__ADDRESS__) & (I2C_CR2_SADD)) | (I2C_CR2_START) | (I2C_CR2_AUTOEND)) & (~I2C_CR2_RD_WRN)) : \
                                                          (uint32_t)((((uint32_t)(__ADDRESS__) & (I2C_CR2_SADD)) | (I2C_CR2_ADD10) | (I2C_CR2_START)) & (~I2C_CR2_RD_WRN)))

#define I2C_CHECK_FLAG(__ISR__, __FLAG__)         ((((__ISR__) & ((__FLAG__) & I2C_FLAG_MASK)) == ((__FLAG__) & I2C_FLAG_MASK)) ? SET : RESET)
#define I2C_CHECK_IT_SOURCE(__CR1__, __IT__)      ((((__CR1__) & (__IT__)) == (__IT__)) ? SET : RESET)
/**
  * @}
  */

/* Private Functions ---------------------------------------------------------*/
/** @defgroup I2C_Private_Functions I2C Private Functions
  * @{
  */
/* Private functions are defined in stm32l4xx_hal_i2c.c file */
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


#endif /* STM32L4xx_HAL_I2C_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
