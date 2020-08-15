/**
  ******************************************************************************
  * @file    stm32l4xx_hal_smbus.c
  * @author  MCD Application Team
  * @brief   SMBUS HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the System Management Bus (SMBus) peripheral,
  *          based on I2C principles of operation :
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral State and Errors functions
  *
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
    [..]
    The SMBUS HAL driver can be used as follows:

    (#) Declare a SMBUS_HandleTypeDef handle structure, for example:
        SMBUS_HandleTypeDef  hsmbus;

    (#)Initialize the SMBUS low level resources by implementing the @ref HAL_SMBUS_MspInit() API:
        (##) Enable the SMBUSx interface clock
        (##) SMBUS pins configuration
            (+++) Enable the clock for the SMBUS GPIOs
            (+++) Configure SMBUS pins as alternate function open-drain
        (##) NVIC configuration if you need to use interrupt process
            (+++) Configure the SMBUSx interrupt priority
            (+++) Enable the NVIC SMBUS IRQ Channel

    (#) Configure the Communication Clock Timing, Bus Timeout, Own Address1, Master Addressing mode,
        Dual Addressing mode, Own Address2, Own Address2 Mask, General call, Nostretch mode,
        Peripheral mode and Packet Error Check mode in the hsmbus Init structure.

    (#) Initialize the SMBUS registers by calling the @ref HAL_SMBUS_Init() API:
        (++) These API's configures also the low level Hardware GPIO, CLOCK, CORTEX...etc)
             by calling the customized @ref HAL_SMBUS_MspInit(&hsmbus) API.

    (#) To check if target device is ready for communication, use the function @ref HAL_SMBUS_IsDeviceReady()

    (#) For SMBUS IO operations, only one mode of operations is available within this driver

    *** Interrupt mode IO operation ***
    ===================================
    [..]
      (+) Transmit in master/host SMBUS mode an amount of data in non-blocking mode using @ref HAL_SMBUS_Master_Transmit_IT()
      (++) At transmission end of transfer @ref HAL_SMBUS_MasterTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_SMBUS_MasterTxCpltCallback()
      (+) Receive in master/host SMBUS mode an amount of data in non-blocking mode using @ref HAL_SMBUS_Master_Receive_IT()
      (++) At reception end of transfer @ref HAL_SMBUS_MasterRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_SMBUS_MasterRxCpltCallback()
      (+) Abort a master/host SMBUS process communication with Interrupt using @ref HAL_SMBUS_Master_Abort_IT()
      (++) The associated previous transfer callback is called at the end of abort process
      (++) mean @ref HAL_SMBUS_MasterTxCpltCallback() in case of previous state was master transmit
      (++) mean @ref HAL_SMBUS_MasterRxCpltCallback() in case of previous state was master receive
      (+) Enable/disable the Address listen mode in slave/device or host/slave SMBUS mode
           using @ref HAL_SMBUS_EnableListen_IT() @ref HAL_SMBUS_DisableListen_IT()
      (++) When address slave/device SMBUS match, @ref HAL_SMBUS_AddrCallback() is executed and user can
           add his own code to check the Address Match Code and the transmission direction request by master/host (Write/Read).
      (++) At Listen mode end @ref HAL_SMBUS_ListenCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_SMBUS_ListenCpltCallback()
      (+) Transmit in slave/device SMBUS mode an amount of data in non-blocking mode using @ref HAL_SMBUS_Slave_Transmit_IT()
      (++) At transmission end of transfer @ref HAL_SMBUS_SlaveTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_SMBUS_SlaveTxCpltCallback()
      (+) Receive in slave/device SMBUS mode an amount of data in non-blocking mode using @ref HAL_SMBUS_Slave_Receive_IT()
      (++) At reception end of transfer @ref HAL_SMBUS_SlaveRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_SMBUS_SlaveRxCpltCallback()
      (+) Enable/Disable the SMBUS alert mode using @ref HAL_SMBUS_EnableAlert_IT() @ref HAL_SMBUS_DisableAlert_IT()
      (++) When SMBUS Alert is generated @ref HAL_SMBUS_ErrorCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_SMBUS_ErrorCallback()
           to check the Alert Error Code using function @ref HAL_SMBUS_GetError()
      (+) Get HAL state machine or error values using @ref HAL_SMBUS_GetState() or @ref HAL_SMBUS_GetError()
      (+) In case of transfer Error, @ref HAL_SMBUS_ErrorCallback() function is executed and user can
           add his own code by customization of function pointer @ref HAL_SMBUS_ErrorCallback()
           to check the Error Code using function @ref HAL_SMBUS_GetError()

     *** SMBUS HAL driver macros list ***
     ==================================
     [..]
       Below the list of most used macros in SMBUS HAL driver.

      (+) @ref __HAL_SMBUS_ENABLE:      Enable the SMBUS peripheral
      (+) @ref __HAL_SMBUS_DISABLE:     Disable the SMBUS peripheral
      (+) @ref __HAL_SMBUS_GET_FLAG:    Check whether the specified SMBUS flag is set or not
      (+) @ref __HAL_SMBUS_CLEAR_FLAG:  Clear the specified SMBUS pending flag
      (+) @ref __HAL_SMBUS_ENABLE_IT:   Enable the specified SMBUS interrupt
      (+) @ref __HAL_SMBUS_DISABLE_IT:  Disable the specified SMBUS interrupt

     *** Callback registration ***
     =============================================
    [..]
     The compilation flag USE_HAL_SMBUS_REGISTER_CALLBACKS when set to 1
     allows the user to configure dynamically the driver callbacks.
     Use Functions @ref HAL_SMBUS_RegisterCallback() or @ref HAL_SMBUS_RegisterAddrCallback()
     to register an interrupt callback.
    [..]
     Function @ref HAL_SMBUS_RegisterCallback() allows to register following callbacks:
       (+) MasterTxCpltCallback : callback for Master transmission end of transfer.
       (+) MasterRxCpltCallback : callback for Master reception end of transfer.
       (+) SlaveTxCpltCallback  : callback for Slave transmission end of transfer.
       (+) SlaveRxCpltCallback  : callback for Slave reception end of transfer.
       (+) ListenCpltCallback   : callback for end of listen mode.
       (+) ErrorCallback        : callback for error detection.
       (+) MspInitCallback      : callback for Msp Init.
       (+) MspDeInitCallback    : callback for Msp DeInit.
     This function takes as parameters the HAL peripheral handle, the Callback ID
     and a pointer to the user callback function.
    [..]
     For specific callback AddrCallback use dedicated register callbacks : @ref HAL_SMBUS_RegisterAddrCallback.
    [..]
     Use function @ref HAL_SMBUS_UnRegisterCallback to reset a callback to the default
     weak function.
     @ref HAL_SMBUS_UnRegisterCallback takes as parameters the HAL peripheral handle,
     and the Callback ID.
     This function allows to reset following callbacks:
       (+) MasterTxCpltCallback : callback for Master transmission end of transfer.
       (+) MasterRxCpltCallback : callback for Master reception end of transfer.
       (+) SlaveTxCpltCallback  : callback for Slave transmission end of transfer.
       (+) SlaveRxCpltCallback  : callback for Slave reception end of transfer.
       (+) ListenCpltCallback   : callback for end of listen mode.
       (+) ErrorCallback        : callback for error detection.
       (+) MspInitCallback      : callback for Msp Init.
       (+) MspDeInitCallback    : callback for Msp DeInit.
    [..]
     For callback AddrCallback use dedicated register callbacks : @ref HAL_SMBUS_UnRegisterAddrCallback.
    [..]
     By default, after the @ref HAL_SMBUS_Init() and when the state is @ref HAL_I2C_STATE_RESET
     all callbacks are set to the corresponding weak functions:
     examples @ref HAL_SMBUS_MasterTxCpltCallback(), @ref HAL_SMBUS_MasterRxCpltCallback().
     Exception done for MspInit and MspDeInit functions that are
     reset to the legacy weak functions in the @ref HAL_SMBUS_Init()/ @ref HAL_SMBUS_DeInit() only when
     these callbacks are null (not registered beforehand).
     If MspInit or MspDeInit are not null, the @ref HAL_SMBUS_Init()/ @ref HAL_SMBUS_DeInit()
     keep and use the user MspInit/MspDeInit callbacks (registered beforehand) whatever the state.
    [..]
     Callbacks can be registered/unregistered in @ref HAL_I2C_STATE_READY state only.
     Exception done MspInit/MspDeInit functions that can be registered/unregistered
     in @ref HAL_I2C_STATE_READY or @ref HAL_I2C_STATE_RESET state,
     thus registered (user) MspInit/DeInit callbacks can be used during the Init/DeInit.
     Then, the user first registers the MspInit/MspDeInit user callbacks
     using @ref HAL_SMBUS_RegisterCallback() before calling @ref HAL_SMBUS_DeInit()
     or @ref HAL_SMBUS_Init() function.
    [..]
     When the compilation flag USE_HAL_SMBUS_REGISTER_CALLBACKS is set to 0 or
     not defined, the callback registration feature is not available and all callbacks
     are set to the corresponding weak functions.

     [..]
       (@) You can refer to the SMBUS HAL driver header file for more useful macros

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

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup SMBUS SMBUS
  * @brief SMBUS HAL module driver
  * @{
  */

#ifdef HAL_SMBUS_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup SMBUS_Private_Define SMBUS Private Constants
  * @{
  */
#define TIMING_CLEAR_MASK   (0xF0FFFFFFUL)     /*!< SMBUS TIMING clear register Mask */
#define HAL_TIMEOUT_ADDR    (10000U)           /*!< 10 s  */
#define HAL_TIMEOUT_BUSY    (25U)              /*!< 25 ms */
#define HAL_TIMEOUT_DIR     (25U)              /*!< 25 ms */
#define HAL_TIMEOUT_RXNE    (25U)              /*!< 25 ms */
#define HAL_TIMEOUT_STOPF   (25U)              /*!< 25 ms */
#define HAL_TIMEOUT_TC      (25U)              /*!< 25 ms */
#define HAL_TIMEOUT_TCR     (25U)              /*!< 25 ms */
#define HAL_TIMEOUT_TXIS    (25U)              /*!< 25 ms */
#define MAX_NBYTE_SIZE      255U
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @addtogroup SMBUS_Private_Functions SMBUS Private Functions
  * @{
  */
static HAL_StatusTypeDef SMBUS_WaitOnFlagUntilTimeout(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t Flag, FlagStatus Status, uint32_t Timeout);

static void SMBUS_Enable_IRQ(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t InterruptRequest);
static void SMBUS_Disable_IRQ(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t InterruptRequest);
static HAL_StatusTypeDef SMBUS_Master_ISR(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t StatusFlags);
static HAL_StatusTypeDef SMBUS_Slave_ISR(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t StatusFlags);

static void SMBUS_ConvertOtherXferOptions(struct __SMBUS_HandleTypeDef *hsmbus);

static void SMBUS_ITErrorHandler(struct __SMBUS_HandleTypeDef *hsmbus);

static void SMBUS_TransferConfig(struct __SMBUS_HandleTypeDef *hsmbus,  uint16_t DevAddress, uint8_t Size, uint32_t Mode, uint32_t Request);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup SMBUS_Exported_Functions SMBUS Exported Functions
  * @{
  */

/** @defgroup SMBUS_Exported_Functions_Group1 Initialization and de-initialization functions
 *  @brief    Initialization and Configuration functions
 *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]  This subsection provides a set of functions allowing to initialize and
          deinitialize the SMBUSx peripheral:

      (+) User must Implement HAL_SMBUS_MspInit() function in which he configures
          all related peripherals resources (CLOCK, GPIO, IT and NVIC ).

      (+) Call the function HAL_SMBUS_Init() to configure the selected device with
          the selected configuration:
        (++) Clock Timing
        (++) Bus Timeout
        (++) Analog Filer mode
        (++) Own Address 1
        (++) Addressing mode (Master, Slave)
        (++) Dual Addressing mode
        (++) Own Address 2
        (++) Own Address 2 Mask
        (++) General call mode
        (++) Nostretch mode
        (++) Packet Error Check mode
        (++) Peripheral mode


      (+) Call the function HAL_SMBUS_DeInit() to restore the default configuration
          of the selected SMBUSx peripheral.

      (+) Enable/Disable Analog/Digital filters with HAL_SMBUS_ConfigAnalogFilter() and
          HAL_SMBUS_ConfigDigitalFilter().

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the SMBUS according to the specified parameters
  *         in the SMBUS_InitTypeDef and initialize the associated handle.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_Init(SMBUS_HandleTypeDef *hsmbus)
{
  /* Check the SMBUS handle allocation */
  if (hsmbus == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_SMBUS_ALL_INSTANCE(hsmbus->Instance));
  assert_param(IS_SMBUS_ANALOG_FILTER(hsmbus->Init.AnalogFilter));
  assert_param(IS_SMBUS_OWN_ADDRESS1(hsmbus->Init.OwnAddress1));
  assert_param(IS_SMBUS_ADDRESSING_MODE(hsmbus->Init.AddressingMode));
  assert_param(IS_SMBUS_DUAL_ADDRESS(hsmbus->Init.DualAddressMode));
  assert_param(IS_SMBUS_OWN_ADDRESS2(hsmbus->Init.OwnAddress2));
  assert_param(IS_SMBUS_OWN_ADDRESS2_MASK(hsmbus->Init.OwnAddress2Masks));
  assert_param(IS_SMBUS_GENERAL_CALL(hsmbus->Init.GeneralCallMode));
  assert_param(IS_SMBUS_NO_STRETCH(hsmbus->Init.NoStretchMode));
  assert_param(IS_SMBUS_PEC(hsmbus->Init.PacketErrorCheckMode));
  assert_param(IS_SMBUS_PERIPHERAL_MODE(hsmbus->Init.PeripheralMode));

  if (hsmbus->State == HAL_SMBUS_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hsmbus->Lock = HAL_UNLOCKED;

#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
    hsmbus->MasterTxCpltCallback = HAL_SMBUS_MasterTxCpltCallback; /* Legacy weak MasterTxCpltCallback */
    hsmbus->MasterRxCpltCallback = HAL_SMBUS_MasterRxCpltCallback; /* Legacy weak MasterRxCpltCallback */
    hsmbus->SlaveTxCpltCallback  = HAL_SMBUS_SlaveTxCpltCallback;  /* Legacy weak SlaveTxCpltCallback  */
    hsmbus->SlaveRxCpltCallback  = HAL_SMBUS_SlaveRxCpltCallback;  /* Legacy weak SlaveRxCpltCallback  */
    hsmbus->ListenCpltCallback   = HAL_SMBUS_ListenCpltCallback;   /* Legacy weak ListenCpltCallback   */
    hsmbus->ErrorCallback        = HAL_SMBUS_ErrorCallback;        /* Legacy weak ErrorCallback        */
    hsmbus->AddrCallback         = HAL_SMBUS_AddrCallback;         /* Legacy weak AddrCallback         */

    if (hsmbus->MspInitCallback == NULL)
    {
      hsmbus->MspInitCallback = HAL_SMBUS_MspInit; /* Legacy weak MspInit  */
    }

    /* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
    hsmbus->MspInitCallback(hsmbus);
#else
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    HAL_SMBUS_MspInit(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
  }

  hsmbus->State = HAL_SMBUS_STATE_BUSY;

  /* Disable the selected SMBUS peripheral */
  __HAL_SMBUS_DISABLE(hsmbus);

  /*---------------------------- SMBUSx TIMINGR Configuration ------------------------*/
  /* Configure SMBUSx: Frequency range */
  hsmbus->Instance->TIMINGR = hsmbus->Init.Timing & TIMING_CLEAR_MASK;

  /*---------------------------- SMBUSx TIMEOUTR Configuration ------------------------*/
  /* Configure SMBUSx: Bus Timeout  */
  hsmbus->Instance->TIMEOUTR &= ~I2C_TIMEOUTR_TIMOUTEN;
  hsmbus->Instance->TIMEOUTR &= ~I2C_TIMEOUTR_TEXTEN;
  hsmbus->Instance->TIMEOUTR = hsmbus->Init.SMBusTimeout;

  /*---------------------------- SMBUSx OAR1 Configuration -----------------------*/
  /* Configure SMBUSx: Own Address1 and ack own address1 mode */
  hsmbus->Instance->OAR1 &= ~I2C_OAR1_OA1EN;

  if (hsmbus->Init.OwnAddress1 != 0UL)
  {
    if (hsmbus->Init.AddressingMode == SMBUS_ADDRESSINGMODE_7BIT)
    {
      hsmbus->Instance->OAR1 = (I2C_OAR1_OA1EN | hsmbus->Init.OwnAddress1);
    }
    else /* SMBUS_ADDRESSINGMODE_10BIT */
    {
      hsmbus->Instance->OAR1 = (I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE | hsmbus->Init.OwnAddress1);
    }
  }

  /*---------------------------- SMBUSx CR2 Configuration ------------------------*/
  /* Configure SMBUSx: Addressing Master mode */
  if (hsmbus->Init.AddressingMode == SMBUS_ADDRESSINGMODE_10BIT)
  {
    hsmbus->Instance->CR2 = (I2C_CR2_ADD10);
  }
  /* Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process) */
  /* AUTOEND and NACK bit will be manage during Transfer process */
  hsmbus->Instance->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);

  /*---------------------------- SMBUSx OAR2 Configuration -----------------------*/
  /* Configure SMBUSx: Dual mode and Own Address2 */
  hsmbus->Instance->OAR2 = (hsmbus->Init.DualAddressMode | hsmbus->Init.OwnAddress2 | (hsmbus->Init.OwnAddress2Masks << 8U));

  /*---------------------------- SMBUSx CR1 Configuration ------------------------*/
  /* Configure SMBUSx: Generalcall and NoStretch mode */
  hsmbus->Instance->CR1 = (hsmbus->Init.GeneralCallMode | hsmbus->Init.NoStretchMode | hsmbus->Init.PacketErrorCheckMode | hsmbus->Init.PeripheralMode | hsmbus->Init.AnalogFilter);

  /* Enable Slave Byte Control only in case of Packet Error Check is enabled and SMBUS Peripheral is set in Slave mode */
  if ((hsmbus->Init.PacketErrorCheckMode == SMBUS_PEC_ENABLE)
      && ((hsmbus->Init.PeripheralMode == SMBUS_PERIPHERAL_MODE_SMBUS_SLAVE) || (hsmbus->Init.PeripheralMode == SMBUS_PERIPHERAL_MODE_SMBUS_SLAVE_ARP)))
  {
    hsmbus->Instance->CR1 |= I2C_CR1_SBC;
  }

  /* Enable the selected SMBUS peripheral */
  __HAL_SMBUS_ENABLE(hsmbus);

  hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;
  hsmbus->PreviousState = HAL_SMBUS_STATE_READY;
  hsmbus->State = HAL_SMBUS_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  DeInitialize the SMBUS peripheral.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_DeInit(SMBUS_HandleTypeDef *hsmbus)
{
  /* Check the SMBUS handle allocation */
  if (hsmbus == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_SMBUS_ALL_INSTANCE(hsmbus->Instance));

  hsmbus->State = HAL_SMBUS_STATE_BUSY;

  /* Disable the SMBUS Peripheral Clock */
  __HAL_SMBUS_DISABLE(hsmbus);

#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
  if (hsmbus->MspDeInitCallback == NULL)
  {
    hsmbus->MspDeInitCallback = HAL_SMBUS_MspDeInit; /* Legacy weak MspDeInit  */
  }

  /* DeInit the low level hardware: GPIO, CLOCK, NVIC */
  hsmbus->MspDeInitCallback(hsmbus);
#else
  /* DeInit the low level hardware: GPIO, CLOCK, NVIC */
  HAL_SMBUS_MspDeInit(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */

  hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;
  hsmbus->PreviousState =  HAL_SMBUS_STATE_RESET;
  hsmbus->State = HAL_SMBUS_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hsmbus);

  return HAL_OK;
}

/**
  * @brief Initialize the SMBUS MSP.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_MspInit(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_MspInit could be implemented in the user file
   */
}

/**
  * @brief DeInitialize the SMBUS MSP.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_MspDeInit(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief  Configure Analog noise filter.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  AnalogFilter This parameter can be one of the following values:
  *         @arg @ref SMBUS_ANALOGFILTER_ENABLE
  *         @arg @ref SMBUS_ANALOGFILTER_DISABLE
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_ConfigAnalogFilter(SMBUS_HandleTypeDef *hsmbus, uint32_t AnalogFilter)
{
  /* Check the parameters */
  assert_param(IS_SMBUS_ALL_INSTANCE(hsmbus->Instance));
  assert_param(IS_SMBUS_ANALOG_FILTER(AnalogFilter));

  if (hsmbus->State == HAL_SMBUS_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsmbus);

    hsmbus->State = HAL_SMBUS_STATE_BUSY;

    /* Disable the selected SMBUS peripheral */
    __HAL_SMBUS_DISABLE(hsmbus);

    /* Reset ANOFF bit */
    hsmbus->Instance->CR1 &= ~(I2C_CR1_ANFOFF);

    /* Set analog filter bit*/
    hsmbus->Instance->CR1 |= AnalogFilter;

    __HAL_SMBUS_ENABLE(hsmbus);

    hsmbus->State = HAL_SMBUS_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Configure Digital noise filter.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  DigitalFilter Coefficient of digital noise filter between Min_Data=0x00 and Max_Data=0x0F.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_ConfigDigitalFilter(SMBUS_HandleTypeDef *hsmbus, uint32_t DigitalFilter)
{
  uint32_t tmpreg;

  /* Check the parameters */
  assert_param(IS_SMBUS_ALL_INSTANCE(hsmbus->Instance));
  assert_param(IS_SMBUS_DIGITAL_FILTER(DigitalFilter));

  if (hsmbus->State == HAL_SMBUS_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsmbus);

    hsmbus->State = HAL_SMBUS_STATE_BUSY;

    /* Disable the selected SMBUS peripheral */
    __HAL_SMBUS_DISABLE(hsmbus);

    /* Get the old register value */
    tmpreg = hsmbus->Instance->CR1;

    /* Reset I2C DNF bits [11:8] */
    tmpreg &= ~(I2C_CR1_DNF);

    /* Set I2Cx DNF coefficient */
    tmpreg |= DigitalFilter << I2C_CR1_DNF_Pos;

    /* Store the new register value */
    hsmbus->Instance->CR1 = tmpreg;

    __HAL_SMBUS_ENABLE(hsmbus);

    hsmbus->State = HAL_SMBUS_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User SMBUS Callback
  *         To be used instead of the weak predefined callback
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_SMBUS_MASTER_TX_COMPLETE_CB_ID Master Tx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_MASTER_RX_COMPLETE_CB_ID Master Rx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_SLAVE_TX_COMPLETE_CB_ID Slave Tx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_SLAVE_RX_COMPLETE_CB_ID Slave Rx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_LISTEN_COMPLETE_CB_ID Listen Complete callback ID
  *          @arg @ref HAL_SMBUS_ERROR_CB_ID Error callback ID
  *          @arg @ref HAL_SMBUS_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_SMBUS_MSPDEINIT_CB_ID MspDeInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_RegisterCallback(SMBUS_HandleTypeDef *hsmbus, HAL_SMBUS_CallbackIDTypeDef CallbackID, pSMBUS_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hsmbus);

  if (HAL_SMBUS_STATE_READY == hsmbus->State)
  {
    switch (CallbackID)
    {
      case HAL_SMBUS_MASTER_TX_COMPLETE_CB_ID :
        hsmbus->MasterTxCpltCallback = pCallback;
        break;

      case HAL_SMBUS_MASTER_RX_COMPLETE_CB_ID :
        hsmbus->MasterRxCpltCallback = pCallback;
        break;

      case HAL_SMBUS_SLAVE_TX_COMPLETE_CB_ID :
        hsmbus->SlaveTxCpltCallback = pCallback;
        break;

      case HAL_SMBUS_SLAVE_RX_COMPLETE_CB_ID :
        hsmbus->SlaveRxCpltCallback = pCallback;
        break;

      case HAL_SMBUS_LISTEN_COMPLETE_CB_ID :
        hsmbus->ListenCpltCallback = pCallback;
        break;

      case HAL_SMBUS_ERROR_CB_ID :
        hsmbus->ErrorCallback = pCallback;
        break;

      case HAL_SMBUS_MSPINIT_CB_ID :
        hsmbus->MspInitCallback = pCallback;
        break;

      case HAL_SMBUS_MSPDEINIT_CB_ID :
        hsmbus->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_SMBUS_STATE_RESET == hsmbus->State)
  {
    switch (CallbackID)
    {
      case HAL_SMBUS_MSPINIT_CB_ID :
        hsmbus->MspInitCallback = pCallback;
        break;

      case HAL_SMBUS_MSPDEINIT_CB_ID :
        hsmbus->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsmbus);
  return status;
}

/**
  * @brief  Unregister an SMBUS Callback
  *         SMBUS callback is redirected to the weak predefined callback
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_SMBUS_MASTER_TX_COMPLETE_CB_ID Master Tx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_MASTER_RX_COMPLETE_CB_ID Master Rx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_SLAVE_TX_COMPLETE_CB_ID Slave Tx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_SLAVE_RX_COMPLETE_CB_ID Slave Rx Transfer completed callback ID
  *          @arg @ref HAL_SMBUS_LISTEN_COMPLETE_CB_ID Listen Complete callback ID
  *          @arg @ref HAL_SMBUS_ERROR_CB_ID Error callback ID
  *          @arg @ref HAL_SMBUS_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_SMBUS_MSPDEINIT_CB_ID MspDeInit callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_UnRegisterCallback(SMBUS_HandleTypeDef *hsmbus, HAL_SMBUS_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hsmbus);

  if (HAL_SMBUS_STATE_READY == hsmbus->State)
  {
    switch (CallbackID)
    {
      case HAL_SMBUS_MASTER_TX_COMPLETE_CB_ID :
        hsmbus->MasterTxCpltCallback = HAL_SMBUS_MasterTxCpltCallback; /* Legacy weak MasterTxCpltCallback */
        break;

      case HAL_SMBUS_MASTER_RX_COMPLETE_CB_ID :
        hsmbus->MasterRxCpltCallback = HAL_SMBUS_MasterRxCpltCallback; /* Legacy weak MasterRxCpltCallback */
        break;

      case HAL_SMBUS_SLAVE_TX_COMPLETE_CB_ID :
        hsmbus->SlaveTxCpltCallback = HAL_SMBUS_SlaveTxCpltCallback;   /* Legacy weak SlaveTxCpltCallback  */
        break;

      case HAL_SMBUS_SLAVE_RX_COMPLETE_CB_ID :
        hsmbus->SlaveRxCpltCallback = HAL_SMBUS_SlaveRxCpltCallback;   /* Legacy weak SlaveRxCpltCallback  */
        break;

      case HAL_SMBUS_LISTEN_COMPLETE_CB_ID :
        hsmbus->ListenCpltCallback = HAL_SMBUS_ListenCpltCallback;     /* Legacy weak ListenCpltCallback   */
        break;

      case HAL_SMBUS_ERROR_CB_ID :
        hsmbus->ErrorCallback = HAL_SMBUS_ErrorCallback;               /* Legacy weak ErrorCallback        */
        break;

      case HAL_SMBUS_MSPINIT_CB_ID :
        hsmbus->MspInitCallback = HAL_SMBUS_MspInit;                   /* Legacy weak MspInit              */
        break;

      case HAL_SMBUS_MSPDEINIT_CB_ID :
        hsmbus->MspDeInitCallback = HAL_SMBUS_MspDeInit;               /* Legacy weak MspDeInit            */
        break;

      default :
        /* Update the error code */
        hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_SMBUS_STATE_RESET == hsmbus->State)
  {
    switch (CallbackID)
    {
      case HAL_SMBUS_MSPINIT_CB_ID :
        hsmbus->MspInitCallback = HAL_SMBUS_MspInit;                   /* Legacy weak MspInit              */
        break;

      case HAL_SMBUS_MSPDEINIT_CB_ID :
        hsmbus->MspDeInitCallback = HAL_SMBUS_MspDeInit;               /* Legacy weak MspDeInit            */
        break;

      default :
        /* Update the error code */
        hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsmbus);
  return status;
}

/**
  * @brief  Register the Slave Address Match SMBUS Callback
  *         To be used instead of the weak HAL_SMBUS_AddrCallback() predefined callback
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  pCallback pointer to the Address Match Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_RegisterAddrCallback(SMBUS_HandleTypeDef *hsmbus, pSMBUS_AddrCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }
  /* Process locked */
  __HAL_LOCK(hsmbus);

  if (HAL_SMBUS_STATE_READY == hsmbus->State)
  {
    hsmbus->AddrCallback = pCallback;
  }
  else
  {
    /* Update the error code */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsmbus);
  return status;
}

/**
  * @brief  UnRegister the Slave Address Match SMBUS Callback
  *         Info Ready SMBUS Callback is redirected to the weak HAL_SMBUS_AddrCallback() predefined callback
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_UnRegisterAddrCallback(SMBUS_HandleTypeDef *hsmbus)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hsmbus);

  if (HAL_SMBUS_STATE_READY == hsmbus->State)
  {
    hsmbus->AddrCallback = HAL_SMBUS_AddrCallback; /* Legacy weak AddrCallback  */
  }
  else
  {
    /* Update the error code */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsmbus);
  return status;
}

#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup SMBUS_Exported_Functions_Group2 Input and Output operation functions
 *  @brief   Data transfers functions
 *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to manage the SMBUS data
    transfers.

    (#) Blocking mode function to check if device is ready for usage is :
        (++) HAL_SMBUS_IsDeviceReady()

    (#) There is only one mode of transfer:
       (++) Non-Blocking mode : The communication is performed using Interrupts.
            These functions return the status of the transfer startup.
            The end of the data processing will be indicated through the
            dedicated SMBUS IRQ when using Interrupt mode.

    (#) Non-Blocking mode functions with Interrupt are :
        (++) HAL_SMBUS_Master_Transmit_IT()
        (++) HAL_SMBUS_Master_Receive_IT()
        (++) HAL_SMBUS_Slave_Transmit_IT()
        (++) HAL_SMBUS_Slave_Receive_IT()
        (++) HAL_SMBUS_EnableListen_IT() or alias HAL_SMBUS_EnableListen_IT()
        (++) HAL_SMBUS_DisableListen_IT()
        (++) HAL_SMBUS_EnableAlert_IT()
        (++) HAL_SMBUS_DisableAlert_IT()

    (#) A set of Transfer Complete Callbacks are provided in non-Blocking mode:
        (++) HAL_SMBUS_MasterTxCpltCallback()
        (++) HAL_SMBUS_MasterRxCpltCallback()
        (++) HAL_SMBUS_SlaveTxCpltCallback()
        (++) HAL_SMBUS_SlaveRxCpltCallback()
        (++) HAL_SMBUS_AddrCallback()
        (++) HAL_SMBUS_ListenCpltCallback()
        (++) HAL_SMBUS_ErrorCallback()

@endverbatim
  * @{
  */

/**
  * @brief  Transmit in master/host SMBUS mode an amount of data in non-blocking mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  XferOptions Options of Transfer, value of @ref SMBUS_XferOptions_definition
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_Master_Transmit_IT(SMBUS_HandleTypeDef *hsmbus, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions)
{
  uint32_t tmp;

  /* Check the parameters */
  assert_param(IS_SMBUS_TRANSFER_OPTIONS_REQUEST(XferOptions));

  if (hsmbus->State == HAL_SMBUS_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsmbus);

    hsmbus->State = HAL_SMBUS_STATE_MASTER_BUSY_TX;
    hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;
    /* Prepare transfer parameters */
    hsmbus->pBuffPtr = pData;
    hsmbus->XferCount = Size;
    hsmbus->XferOptions = XferOptions;

    /* In case of Quick command, remove autoend mode */
    /* Manage the stop generation by software */
    if (hsmbus->pBuffPtr == NULL)
    {
      hsmbus->XferOptions &= ~SMBUS_AUTOEND_MODE;
    }

    if (Size > MAX_NBYTE_SIZE)
    {
      hsmbus->XferSize = MAX_NBYTE_SIZE;
    }
    else
    {
      hsmbus->XferSize = Size;
    }

    /* Send Slave Address */
    /* Set NBYTES to write and reload if size > MAX_NBYTE_SIZE and generate RESTART */
    if ((hsmbus->XferSize < hsmbus->XferCount) && (hsmbus->XferSize == MAX_NBYTE_SIZE))
    {
      SMBUS_TransferConfig(hsmbus, DevAddress, (uint8_t)hsmbus->XferSize, SMBUS_RELOAD_MODE | (hsmbus->XferOptions & SMBUS_SENDPEC_MODE), SMBUS_GENERATE_START_WRITE);
    }
    else
    {
      /* If transfer direction not change, do not generate Restart Condition */
      /* Mean Previous state is same as current state */

      /* Store current volatile XferOptions, misra rule */
      tmp = hsmbus->XferOptions;

      if ((hsmbus->PreviousState == HAL_SMBUS_STATE_MASTER_BUSY_TX) && (IS_SMBUS_TRANSFER_OTHER_OPTIONS_REQUEST(tmp) == 0))
      {
        SMBUS_TransferConfig(hsmbus, DevAddress, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_NO_STARTSTOP);
      }
      /* Else transfer direction change, so generate Restart with new transfer direction */
      else
      {
        /* Convert OTHER_xxx XferOptions if any */
        SMBUS_ConvertOtherXferOptions(hsmbus);

        /* Handle Transfer */
        SMBUS_TransferConfig(hsmbus, DevAddress, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_GENERATE_START_WRITE);
      }

      /* If PEC mode is enable, size to transmit manage by SW part should be Size-1 byte, corresponding to PEC byte */
      /* PEC byte is automatically sent by HW block, no need to manage it in Transmit process */
      if (SMBUS_GET_PEC_MODE(hsmbus) != 0UL)
      {
        hsmbus->XferSize--;
        hsmbus->XferCount--;
      }
    }

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    /* Note : The SMBUS interrupts must be enabled after unlocking current process
              to avoid the risk of SMBUS interrupt handle execution before current
              process unlock */
    SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_TX);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Receive in master/host SMBUS mode an amount of data in non-blocking mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  XferOptions Options of Transfer, value of @ref SMBUS_XferOptions_definition
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_Master_Receive_IT(SMBUS_HandleTypeDef *hsmbus, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions)
{
  uint32_t tmp;

  /* Check the parameters */
  assert_param(IS_SMBUS_TRANSFER_OPTIONS_REQUEST(XferOptions));

  if (hsmbus->State == HAL_SMBUS_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsmbus);

    hsmbus->State = HAL_SMBUS_STATE_MASTER_BUSY_RX;
    hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;

    /* Prepare transfer parameters */
    hsmbus->pBuffPtr = pData;
    hsmbus->XferCount = Size;
    hsmbus->XferOptions = XferOptions;

    /* In case of Quick command, remove autoend mode */
    /* Manage the stop generation by software */
    if (hsmbus->pBuffPtr == NULL)
    {
      hsmbus->XferOptions &= ~SMBUS_AUTOEND_MODE;
    }

    if (Size > MAX_NBYTE_SIZE)
    {
      hsmbus->XferSize = MAX_NBYTE_SIZE;
    }
    else
    {
      hsmbus->XferSize = Size;
    }

    /* Send Slave Address */
    /* Set NBYTES to write and reload if size > MAX_NBYTE_SIZE and generate RESTART */
    if ((hsmbus->XferSize < hsmbus->XferCount) && (hsmbus->XferSize == MAX_NBYTE_SIZE))
    {
      SMBUS_TransferConfig(hsmbus, DevAddress, (uint8_t)hsmbus->XferSize, SMBUS_RELOAD_MODE  | (hsmbus->XferOptions & SMBUS_SENDPEC_MODE), SMBUS_GENERATE_START_READ);
    }
    else
    {
      /* If transfer direction not change, do not generate Restart Condition */
      /* Mean Previous state is same as current state */

      /* Store current volatile XferOptions, Misra rule */
      tmp = hsmbus->XferOptions;

      if ((hsmbus->PreviousState == HAL_SMBUS_STATE_MASTER_BUSY_RX) && (IS_SMBUS_TRANSFER_OTHER_OPTIONS_REQUEST(tmp) == 0))
      {
        SMBUS_TransferConfig(hsmbus, DevAddress, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_NO_STARTSTOP);
      }
      /* Else transfer direction change, so generate Restart with new transfer direction */
      else
      {
        /* Convert OTHER_xxx XferOptions if any */
        SMBUS_ConvertOtherXferOptions(hsmbus);

        /* Handle Transfer */
        SMBUS_TransferConfig(hsmbus, DevAddress, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_GENERATE_START_READ);
      }
    }

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    /* Note : The SMBUS interrupts must be enabled after unlocking current process
              to avoid the risk of SMBUS interrupt handle execution before current
              process unlock */
    SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_RX);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Abort a master/host SMBUS process communication with Interrupt.
  * @note   This abort can be called only if state is ready
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_Master_Abort_IT(SMBUS_HandleTypeDef *hsmbus, uint16_t DevAddress)
{
  if (hsmbus->State == HAL_SMBUS_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hsmbus);

    /* Keep the same state as previous */
    /* to perform as well the call of the corresponding end of transfer callback */
    if (hsmbus->PreviousState == HAL_SMBUS_STATE_MASTER_BUSY_TX)
    {
      hsmbus->State = HAL_SMBUS_STATE_MASTER_BUSY_TX;
    }
    else if (hsmbus->PreviousState == HAL_SMBUS_STATE_MASTER_BUSY_RX)
    {
      hsmbus->State = HAL_SMBUS_STATE_MASTER_BUSY_RX;
    }
    else
    {
      /* Wrong usage of abort function */
      /* This function should be used only in case of abort monitored by master device */
      return HAL_ERROR;
    }
    hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;

    /* Set NBYTES to 1 to generate a dummy read on SMBUS peripheral */
    /* Set AUTOEND mode, this will generate a NACK then STOP condition to abort the current transfer */
    SMBUS_TransferConfig(hsmbus, DevAddress, 1, SMBUS_AUTOEND_MODE, SMBUS_NO_STARTSTOP);

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    /* Note : The SMBUS interrupts must be enabled after unlocking current process
              to avoid the risk of SMBUS interrupt handle execution before current
              process unlock */
    if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_TX)
    {
      SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_TX);
    }
    else if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_RX)
    {
      SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_RX);
    }
    else
    {
      /* Nothing to do */
    }

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Transmit in slave/device SMBUS mode an amount of data in non-blocking mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  XferOptions Options of Transfer, value of @ref SMBUS_XferOptions_definition
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_Slave_Transmit_IT(SMBUS_HandleTypeDef *hsmbus, uint8_t *pData, uint16_t Size, uint32_t XferOptions)
{
  /* Check the parameters */
  assert_param(IS_SMBUS_TRANSFER_OPTIONS_REQUEST(XferOptions));

  if ((hsmbus->State & HAL_SMBUS_STATE_LISTEN) == HAL_SMBUS_STATE_LISTEN)
  {
    if ((pData == NULL) || (Size == 0UL))
    {
      hsmbus->ErrorCode = HAL_SMBUS_ERROR_INVALID_PARAM;
      return HAL_ERROR;
    }

    /* Disable Interrupts, to prevent preemption during treatment in case of multicall */
    SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_ADDR | SMBUS_IT_TX);

    /* Process Locked */
    __HAL_LOCK(hsmbus);

    hsmbus->State = (HAL_SMBUS_STATE_SLAVE_BUSY_TX | HAL_SMBUS_STATE_LISTEN);
    hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;

    /* Set SBC bit to manage Acknowledge at each bit */
    hsmbus->Instance->CR1 |= I2C_CR1_SBC;

    /* Enable Address Acknowledge */
    hsmbus->Instance->CR2 &= ~I2C_CR2_NACK;

    /* Prepare transfer parameters */
    hsmbus->pBuffPtr = pData;
    hsmbus->XferCount = Size;
    hsmbus->XferOptions = XferOptions;

    /* Convert OTHER_xxx XferOptions if any */
    SMBUS_ConvertOtherXferOptions(hsmbus);

    if (Size > MAX_NBYTE_SIZE)
    {
      hsmbus->XferSize = MAX_NBYTE_SIZE;
    }
    else
    {
      hsmbus->XferSize = Size;
    }

    /* Set NBYTES to write and reload if size > MAX_NBYTE_SIZE and generate RESTART */
    if ((hsmbus->XferSize < hsmbus->XferCount) && (hsmbus->XferSize == MAX_NBYTE_SIZE))
    {
      SMBUS_TransferConfig(hsmbus, 0, (uint8_t)hsmbus->XferSize, SMBUS_RELOAD_MODE | (hsmbus->XferOptions & SMBUS_SENDPEC_MODE), SMBUS_NO_STARTSTOP);
    }
    else
    {
      /* Set NBYTE to transmit */
      SMBUS_TransferConfig(hsmbus, 0, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_NO_STARTSTOP);

      /* If PEC mode is enable, size to transmit should be Size-1 byte, corresponding to PEC byte */
      /* PEC byte is automatically sent by HW block, no need to manage it in Transmit process */
      if (SMBUS_GET_PEC_MODE(hsmbus) != 0UL)
      {
        hsmbus->XferSize--;
        hsmbus->XferCount--;
      }
    }

    /* Clear ADDR flag after prepare the transfer parameters */
    /* This action will generate an acknowledge to the HOST */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_ADDR);

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    /* Note : The SMBUS interrupts must be enabled after unlocking current process
              to avoid the risk of SMBUS interrupt handle execution before current
              process unlock */
    /* REnable ADDR interrupt */
    SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_TX | SMBUS_IT_ADDR);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Receive in slave/device SMBUS mode an amount of data in non-blocking mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  XferOptions Options of Transfer, value of @ref SMBUS_XferOptions_definition
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_Slave_Receive_IT(SMBUS_HandleTypeDef *hsmbus, uint8_t *pData, uint16_t Size, uint32_t XferOptions)
{
  /* Check the parameters */
  assert_param(IS_SMBUS_TRANSFER_OPTIONS_REQUEST(XferOptions));

  if ((hsmbus->State & HAL_SMBUS_STATE_LISTEN) == HAL_SMBUS_STATE_LISTEN)
  {
    if ((pData == NULL) || (Size == 0UL))
    {
      hsmbus->ErrorCode = HAL_SMBUS_ERROR_INVALID_PARAM;
      return HAL_ERROR;
    }

    /* Disable Interrupts, to prevent preemption during treatment in case of multicall */
    SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_ADDR | SMBUS_IT_RX);

    /* Process Locked */
    __HAL_LOCK(hsmbus);

    hsmbus->State = (HAL_SMBUS_STATE_SLAVE_BUSY_RX | HAL_SMBUS_STATE_LISTEN);
    hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;

    /* Set SBC bit to manage Acknowledge at each bit */
    hsmbus->Instance->CR1 |= I2C_CR1_SBC;

    /* Enable Address Acknowledge */
    hsmbus->Instance->CR2 &= ~I2C_CR2_NACK;

    /* Prepare transfer parameters */
    hsmbus->pBuffPtr = pData;
    hsmbus->XferSize = Size;
    hsmbus->XferCount = Size;
    hsmbus->XferOptions = XferOptions;

    /* Convert OTHER_xxx XferOptions if any */
    SMBUS_ConvertOtherXferOptions(hsmbus);

    /* Set NBYTE to receive */
    /* If XferSize equal "1", or XferSize equal "2" with PEC requested (mean 1 data byte + 1 PEC byte */
    /* no need to set RELOAD bit mode, a ACK will be automatically generated in that case */
    /* else need to set RELOAD bit mode to generate an automatic ACK at each byte Received */
    /* This RELOAD bit will be reset for last BYTE to be receive in SMBUS_Slave_ISR */
    if (((SMBUS_GET_PEC_MODE(hsmbus) != 0UL) && (hsmbus->XferSize == 2U)) || (hsmbus->XferSize == 1U))
    {
      SMBUS_TransferConfig(hsmbus, 0, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_NO_STARTSTOP);
    }
    else
    {
      SMBUS_TransferConfig(hsmbus, 0, 1, hsmbus->XferOptions | SMBUS_RELOAD_MODE, SMBUS_NO_STARTSTOP);
    }

    /* Clear ADDR flag after prepare the transfer parameters */
    /* This action will generate an acknowledge to the HOST */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_ADDR);

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    /* Note : The SMBUS interrupts must be enabled after unlocking current process
              to avoid the risk of SMBUS interrupt handle execution before current
              process unlock */
    /* REnable ADDR interrupt */
    SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_RX | SMBUS_IT_ADDR);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Enable the Address listen mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_EnableListen_IT(SMBUS_HandleTypeDef *hsmbus)
{
  hsmbus->State = HAL_SMBUS_STATE_LISTEN;

  /* Enable the Address Match interrupt */
  SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_ADDR);

  return HAL_OK;
}

/**
  * @brief  Disable the Address listen mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_DisableListen_IT(SMBUS_HandleTypeDef *hsmbus)
{
  /* Disable Address listen mode only if a transfer is not ongoing */
  if (hsmbus->State == HAL_SMBUS_STATE_LISTEN)
  {
    hsmbus->State = HAL_SMBUS_STATE_READY;

    /* Disable the Address Match interrupt */
    SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_ADDR);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Enable the SMBUS alert mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUSx peripheral.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_EnableAlert_IT(SMBUS_HandleTypeDef *hsmbus)
{
  /* Enable SMBus alert */
  hsmbus->Instance->CR1 |= I2C_CR1_ALERTEN;

  /* Clear ALERT flag */
  __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_ALERT);

  /* Enable Alert Interrupt */
  SMBUS_Enable_IRQ(hsmbus, SMBUS_IT_ALERT);

  return HAL_OK;
}
/**
  * @brief  Disable the SMBUS alert mode with Interrupt.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUSx peripheral.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_DisableAlert_IT(SMBUS_HandleTypeDef *hsmbus)
{
  /* Enable SMBus alert */
  hsmbus->Instance->CR1 &= ~I2C_CR1_ALERTEN;

  /* Disable Alert Interrupt */
  SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_ALERT);

  return HAL_OK;
}

/**
  * @brief  Check if target device is ready for communication.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @param  Trials Number of trials
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SMBUS_IsDeviceReady(SMBUS_HandleTypeDef *hsmbus, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
  uint32_t tickstart;

  __IO uint32_t SMBUS_Trials = 0UL;

  FlagStatus tmp1;
  FlagStatus tmp2;

  if (hsmbus->State == HAL_SMBUS_STATE_READY)
  {
    if (__HAL_SMBUS_GET_FLAG(hsmbus, SMBUS_FLAG_BUSY) != RESET)
    {
      return HAL_BUSY;
    }

    /* Process Locked */
    __HAL_LOCK(hsmbus);

    hsmbus->State = HAL_SMBUS_STATE_BUSY;
    hsmbus->ErrorCode = HAL_SMBUS_ERROR_NONE;

    do
    {
      /* Generate Start */
      hsmbus->Instance->CR2 = SMBUS_GENERATE_START(hsmbus->Init.AddressingMode, DevAddress);

      /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
      /* Wait until STOPF flag is set or a NACK flag is set*/
      tickstart = HAL_GetTick();

      tmp1 = __HAL_SMBUS_GET_FLAG(hsmbus, SMBUS_FLAG_STOPF);
      tmp2 = __HAL_SMBUS_GET_FLAG(hsmbus, SMBUS_FLAG_AF);

      while ((tmp1 == RESET) && (tmp2 == RESET))
      {
        if (Timeout != HAL_MAX_DELAY)
        {
          if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0UL))
          {
            /* Device is ready */
            hsmbus->State = HAL_SMBUS_STATE_READY;

            /* Update SMBUS error code */
            hsmbus->ErrorCode |= HAL_SMBUS_ERROR_HALTIMEOUT;

            /* Process Unlocked */
            __HAL_UNLOCK(hsmbus);
            return HAL_ERROR;
          }
        }

        tmp1 = __HAL_SMBUS_GET_FLAG(hsmbus, SMBUS_FLAG_STOPF);
        tmp2 = __HAL_SMBUS_GET_FLAG(hsmbus, SMBUS_FLAG_AF);
      }

      /* Check if the NACKF flag has not been set */
      if (__HAL_SMBUS_GET_FLAG(hsmbus, SMBUS_FLAG_AF) == RESET)
      {
        /* Wait until STOPF flag is reset */
        if (SMBUS_WaitOnFlagUntilTimeout(hsmbus, SMBUS_FLAG_STOPF, RESET, Timeout) != HAL_OK)
        {
          return HAL_ERROR;
        }

        /* Clear STOP Flag */
        __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_STOPF);

        /* Device is ready */
        hsmbus->State = HAL_SMBUS_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(hsmbus);

        return HAL_OK;
      }
      else
      {
        /* Wait until STOPF flag is reset */
        if (SMBUS_WaitOnFlagUntilTimeout(hsmbus, SMBUS_FLAG_STOPF, RESET, Timeout) != HAL_OK)
        {
          return HAL_ERROR;
        }

        /* Clear NACK Flag */
        __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_AF);

        /* Clear STOP Flag, auto generated with autoend*/
        __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_STOPF);
      }

      /* Check if the maximum allowed number of trials has been reached */
      if (SMBUS_Trials == Trials)
      {
        /* Generate Stop */
        hsmbus->Instance->CR2 |= I2C_CR2_STOP;

        /* Wait until STOPF flag is reset */
        if (SMBUS_WaitOnFlagUntilTimeout(hsmbus, SMBUS_FLAG_STOPF, RESET, Timeout) != HAL_OK)
        {
          return HAL_ERROR;
        }

        /* Clear STOP Flag */
        __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_STOPF);
      }

      /* Increment Trials */
      SMBUS_Trials++;
    }
    while (SMBUS_Trials < Trials);

    hsmbus->State = HAL_SMBUS_STATE_READY;

    /* Update SMBUS error code */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_HALTIMEOUT;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    return HAL_ERROR;
  }
  else
  {
    return HAL_BUSY;
  }
}
/**
  * @}
  */

/** @defgroup SMBUS_IRQ_Handler_and_Callbacks IRQ Handler and Callbacks
 * @{
 */

/**
  * @brief  Handle SMBUS event interrupt request.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
void HAL_SMBUS_EV_IRQHandler(SMBUS_HandleTypeDef *hsmbus)
{
  /* Use a local variable to store the current ISR flags */
  /* This action will avoid a wrong treatment due to ISR flags change during interrupt handler */
  uint32_t tmpisrvalue = READ_REG(hsmbus->Instance->ISR);
  uint32_t tmpcr1value = READ_REG(hsmbus->Instance->CR1);

  /* SMBUS in mode Transmitter ---------------------------------------------------*/
  if ((SMBUS_CHECK_IT_SOURCE(tmpcr1value, (SMBUS_IT_TCI | SMBUS_IT_STOPI | SMBUS_IT_NACKI | SMBUS_IT_TXI)) != RESET) && ((SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_TXIS) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_TCR) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_TC) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_STOPF) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_AF) != RESET)))
  {
    /* Slave mode selected */
    if ((hsmbus->State & HAL_SMBUS_STATE_SLAVE_BUSY_TX) == HAL_SMBUS_STATE_SLAVE_BUSY_TX)
    {
      (void)SMBUS_Slave_ISR(hsmbus, tmpisrvalue);
    }
    /* Master mode selected */
    else if ((hsmbus->State & HAL_SMBUS_STATE_MASTER_BUSY_TX) == HAL_SMBUS_STATE_MASTER_BUSY_TX)
    {
      (void)SMBUS_Master_ISR(hsmbus, tmpisrvalue);
    }
    else
    {
      /* Nothing to do */
    }
  }

  /* SMBUS in mode Receiver ----------------------------------------------------*/
  if ((SMBUS_CHECK_IT_SOURCE(tmpcr1value, (SMBUS_IT_TCI | SMBUS_IT_STOPI | SMBUS_IT_NACKI | SMBUS_IT_RXI)) != RESET) && ((SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_RXNE) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_TCR) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_TC) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_STOPF) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_AF) != RESET)))
  {
    /* Slave mode selected */
    if ((hsmbus->State & HAL_SMBUS_STATE_SLAVE_BUSY_RX) == HAL_SMBUS_STATE_SLAVE_BUSY_RX)
    {
      (void)SMBUS_Slave_ISR(hsmbus, tmpisrvalue);
    }
    /* Master mode selected */
    else if ((hsmbus->State & HAL_SMBUS_STATE_MASTER_BUSY_RX) == HAL_SMBUS_STATE_MASTER_BUSY_RX)
    {
      (void)SMBUS_Master_ISR(hsmbus, tmpisrvalue);
    }
    else
    {
      /* Nothing to do */
    }
  }

  /* SMBUS in mode Listener Only --------------------------------------------------*/
  if (((SMBUS_CHECK_IT_SOURCE(tmpcr1value, SMBUS_IT_ADDRI) != RESET) || (SMBUS_CHECK_IT_SOURCE(tmpcr1value, SMBUS_IT_STOPI) != RESET) || (SMBUS_CHECK_IT_SOURCE(tmpcr1value, SMBUS_IT_NACKI) != RESET)) && ((SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_ADDR) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_STOPF) != RESET) || (SMBUS_CHECK_FLAG(tmpisrvalue, SMBUS_FLAG_AF) != RESET)))
  {
    if ((hsmbus->State & HAL_SMBUS_STATE_LISTEN) == HAL_SMBUS_STATE_LISTEN)
    {
      (void)SMBUS_Slave_ISR(hsmbus, tmpisrvalue);
    }
  }
}

/**
  * @brief  Handle SMBUS error interrupt request.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
void HAL_SMBUS_ER_IRQHandler(SMBUS_HandleTypeDef *hsmbus)
{
  SMBUS_ITErrorHandler(hsmbus);
}

/**
  * @brief  Master Tx Transfer completed callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_MasterTxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_MasterTxCpltCallback() could be implemented in the user file
   */
}

/**
  * @brief  Master Rx Transfer completed callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_MasterRxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_MasterRxCpltCallback() could be implemented in the user file
   */
}

/** @brief  Slave Tx Transfer completed callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_SlaveTxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_SlaveTxCpltCallback() could be implemented in the user file
   */
}

/**
  * @brief  Slave Rx Transfer completed callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_SlaveRxCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_SlaveRxCpltCallback() could be implemented in the user file
   */
}

/**
  * @brief  Slave Address Match callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  TransferDirection Master request Transfer Direction (Write/Read)
  * @param  AddrMatchCode Address Match Code
  * @retval None
  */
__weak void HAL_SMBUS_AddrCallback(SMBUS_HandleTypeDef *hsmbus, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);
  UNUSED(TransferDirection);
  UNUSED(AddrMatchCode);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_AddrCallback() could be implemented in the user file
   */
}

/**
  * @brief  Listen Complete callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_ListenCpltCallback(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_ListenCpltCallback() could be implemented in the user file
   */
}

/**
  * @brief  SMBUS error callback.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval None
  */
__weak void HAL_SMBUS_ErrorCallback(SMBUS_HandleTypeDef *hsmbus)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsmbus);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SMBUS_ErrorCallback() could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup SMBUS_Exported_Functions_Group3 Peripheral State and Errors functions
 *  @brief   Peripheral State and Errors functions
 *
@verbatim
 ===============================================================================
            ##### Peripheral State and Errors functions #####
 ===============================================================================
    [..]
    This subsection permits to get in run-time the status of the peripheral
    and the data flow.

@endverbatim
  * @{
  */

/**
  * @brief  Return the SMBUS handle state.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @retval HAL state
  */
uint32_t HAL_SMBUS_GetState(SMBUS_HandleTypeDef *hsmbus)
{
  /* Return SMBUS handle state */
  return hsmbus->State;
}

/**
* @brief  Return the SMBUS error code.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *              the configuration information for the specified SMBUS.
* @retval SMBUS Error Code
*/
uint32_t HAL_SMBUS_GetError(SMBUS_HandleTypeDef *hsmbus)
{
  return hsmbus->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup SMBUS_Private_Functions SMBUS Private Functions
 *  @brief   Data transfers Private functions
  * @{
  */

/**
  * @brief  Interrupt Sub-Routine which handle the Interrupt Flags Master Mode.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  StatusFlags Value of Interrupt Flags.
  * @retval HAL status
  */
static HAL_StatusTypeDef SMBUS_Master_ISR(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t StatusFlags)
{
  uint16_t DevAddress;

  /* Process Locked */
  __HAL_LOCK(hsmbus);

  if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_AF) != RESET)
  {
    /* Clear NACK Flag */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_AF);

    /* Set corresponding Error Code */
    /* No need to generate STOP, it is automatically done */
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_ACKF;

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    /* Call the Error callback to inform upper layer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
    hsmbus->ErrorCallback(hsmbus);
#else
    HAL_SMBUS_ErrorCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
  }
  else if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_STOPF) != RESET)
  {
    /* Check and treat errors if errors occurs during STOP process */
    SMBUS_ITErrorHandler(hsmbus);

    /* Call the corresponding callback to inform upper layer of End of Transfer */
    if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_TX)
    {
      /* Disable Interrupt */
      SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_TX);

      /* Clear STOP Flag */
      __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_STOPF);

      /* Clear Configuration Register 2 */
      SMBUS_RESET_CR2(hsmbus);

      /* Flush remaining data in Fifo register in case of error occurs before TXEmpty */
      /* Disable the selected SMBUS peripheral */
      __HAL_SMBUS_DISABLE(hsmbus);

      hsmbus->PreviousState = HAL_SMBUS_STATE_READY;
      hsmbus->State = HAL_SMBUS_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmbus);

      /* REenable the selected SMBUS peripheral */
      __HAL_SMBUS_ENABLE(hsmbus);

      /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
      hsmbus->MasterTxCpltCallback(hsmbus);
#else
      HAL_SMBUS_MasterTxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
    }
    else if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_RX)
    {
      /* Store Last receive data if any */
      if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_RXNE) != RESET)
      {
        /* Read data from RXDR */
        *hsmbus->pBuffPtr = (uint8_t)(hsmbus->Instance->RXDR);

        /* Increment Buffer pointer */
        hsmbus->pBuffPtr++;

        if ((hsmbus->XferSize > 0U))
        {
          hsmbus->XferSize--;
          hsmbus->XferCount--;
        }
      }

      /* Disable Interrupt */
      SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_RX);

      /* Clear STOP Flag */
      __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_STOPF);

      /* Clear Configuration Register 2 */
      SMBUS_RESET_CR2(hsmbus);

      hsmbus->PreviousState = HAL_SMBUS_STATE_READY;
      hsmbus->State = HAL_SMBUS_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmbus);

      /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
      hsmbus->MasterRxCpltCallback(hsmbus);
#else
      HAL_SMBUS_MasterRxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
    }
    else
    {
      /* Nothing to do */
    }
  }
  else if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_RXNE) != RESET)
  {
    /* Read data from RXDR */
    *hsmbus->pBuffPtr = (uint8_t)(hsmbus->Instance->RXDR);

    /* Increment Buffer pointer */
    hsmbus->pBuffPtr++;

    /* Increment Size counter */
    hsmbus->XferSize--;
    hsmbus->XferCount--;
  }
  else if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_TXIS) != RESET)
  {
    /* Write data to TXDR */
    hsmbus->Instance->TXDR = *hsmbus->pBuffPtr;

    /* Increment Buffer pointer */
    hsmbus->pBuffPtr++;

    /* Increment Size counter */
    hsmbus->XferSize--;
    hsmbus->XferCount--;
  }
  else if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_TCR) != RESET)
  {
    if ((hsmbus->XferCount != 0U) && (hsmbus->XferSize == 0U))
    {
      DevAddress = (uint16_t)(hsmbus->Instance->CR2 & I2C_CR2_SADD);

      if (hsmbus->XferCount > MAX_NBYTE_SIZE)
      {
        SMBUS_TransferConfig(hsmbus, DevAddress, MAX_NBYTE_SIZE, (SMBUS_RELOAD_MODE | (hsmbus->XferOptions & SMBUS_SENDPEC_MODE)), SMBUS_NO_STARTSTOP);
        hsmbus->XferSize = MAX_NBYTE_SIZE;
      }
      else
      {
        hsmbus->XferSize = hsmbus->XferCount;
        SMBUS_TransferConfig(hsmbus, DevAddress, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_NO_STARTSTOP);
        /* If PEC mode is enable, size to transmit should be Size-1 byte, corresponding to PEC byte */
        /* PEC byte is automatically sent by HW block, no need to manage it in Transmit process */
        if (SMBUS_GET_PEC_MODE(hsmbus) != 0UL)
        {
          hsmbus->XferSize--;
          hsmbus->XferCount--;
        }
      }
    }
    else if ((hsmbus->XferCount == 0U) && (hsmbus->XferSize == 0U))
    {
      /* Call TxCpltCallback() if no stop mode is set */
      if (SMBUS_GET_STOP_MODE(hsmbus) != SMBUS_AUTOEND_MODE)
      {
        /* Call the corresponding callback to inform upper layer of End of Transfer */
        if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_TX)
        {
          /* Disable Interrupt */
          SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_TX);
          hsmbus->PreviousState = hsmbus->State;
          hsmbus->State = HAL_SMBUS_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(hsmbus);

          /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
          hsmbus->MasterTxCpltCallback(hsmbus);
#else
          HAL_SMBUS_MasterTxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
        }
        else if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_RX)
        {
          SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_RX);
          hsmbus->PreviousState = hsmbus->State;
          hsmbus->State = HAL_SMBUS_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(hsmbus);

          /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
          hsmbus->MasterRxCpltCallback(hsmbus);
#else
          HAL_SMBUS_MasterRxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
        }
        else
        {
          /* Nothing to do */
        }
      }
    }
    else
    {
      /* Nothing to do */
    }
  }
  else if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_TC) != RESET)
  {
    if (hsmbus->XferCount == 0U)
    {
      /* Specific use case for Quick command */
      if (hsmbus->pBuffPtr == NULL)
      {
        /* Generate a Stop command */
        hsmbus->Instance->CR2 |= I2C_CR2_STOP;
      }
      /* Call TxCpltCallback() if no stop mode is set */
      else if (SMBUS_GET_STOP_MODE(hsmbus) != SMBUS_AUTOEND_MODE)
      {
        /* No Generate Stop, to permit restart mode */
        /* The stop will be done at the end of transfer, when SMBUS_AUTOEND_MODE enable */

        /* Call the corresponding callback to inform upper layer of End of Transfer */
        if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_TX)
        {
          /* Disable Interrupt */
          SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_TX);
          hsmbus->PreviousState = hsmbus->State;
          hsmbus->State = HAL_SMBUS_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(hsmbus);

          /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
          hsmbus->MasterTxCpltCallback(hsmbus);
#else
          HAL_SMBUS_MasterTxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
        }
        else if (hsmbus->State == HAL_SMBUS_STATE_MASTER_BUSY_RX)
        {
          SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_RX);
          hsmbus->PreviousState = hsmbus->State;
          hsmbus->State = HAL_SMBUS_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(hsmbus);

          /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
          hsmbus->MasterRxCpltCallback(hsmbus);
#else
          HAL_SMBUS_MasterRxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
        }
        else
        {
          /* Nothing to do */
        }
      }
      else
      {
        /* Nothing to do */
      }
    }
  }
  else
  {
    /* Nothing to do */
  }

  /* Process Unlocked */
  __HAL_UNLOCK(hsmbus);

  return HAL_OK;
}
/**
  * @brief  Interrupt Sub-Routine which handle the Interrupt Flags Slave Mode.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  StatusFlags Value of Interrupt Flags.
  * @retval HAL status
  */
static HAL_StatusTypeDef SMBUS_Slave_ISR(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t StatusFlags)
{
  uint8_t TransferDirection;
  uint16_t SlaveAddrCode;

  /* Process Locked */
  __HAL_LOCK(hsmbus);

  if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_AF) != RESET)
  {
    /* Check that SMBUS transfer finished */
    /* if yes, normal usecase, a NACK is sent by the HOST when Transfer is finished */
    /* Mean XferCount == 0*/
    /* So clear Flag NACKF only */
    if (hsmbus->XferCount == 0U)
    {
      /* Clear NACK Flag */
      __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_AF);

      /* Process Unlocked */
      __HAL_UNLOCK(hsmbus);
    }
    else
    {
      /* if no, error usecase, a Non-Acknowledge of last Data is generated by the HOST*/
      /* Clear NACK Flag */
      __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_AF);

      /* Set HAL State to "Idle" State, mean to LISTEN state */
      /* So reset Slave Busy state */
      hsmbus->PreviousState = hsmbus->State;
      hsmbus->State &= ~((uint32_t)HAL_SMBUS_STATE_SLAVE_BUSY_TX);
      hsmbus->State &= ~((uint32_t)HAL_SMBUS_STATE_SLAVE_BUSY_RX);

      /* Disable RX/TX Interrupts, keep only ADDR Interrupt */
      SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_RX | SMBUS_IT_TX);

      /* Set ErrorCode corresponding to a Non-Acknowledge */
      hsmbus->ErrorCode |= HAL_SMBUS_ERROR_ACKF;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmbus);

      /* Call the Error callback to inform upper layer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
      hsmbus->ErrorCallback(hsmbus);
#else
      HAL_SMBUS_ErrorCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
    }
  }
  else if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_ADDR) != RESET)
  {
    TransferDirection = (uint8_t)(SMBUS_GET_DIR(hsmbus));
    SlaveAddrCode = (uint16_t)(SMBUS_GET_ADDR_MATCH(hsmbus));

    /* Disable ADDR interrupt to prevent multiple ADDRInterrupt*/
    /* Other ADDRInterrupt will be treat in next Listen usecase */
    __HAL_SMBUS_DISABLE_IT(hsmbus, SMBUS_IT_ADDRI);

    /* Process Unlocked */
    __HAL_UNLOCK(hsmbus);

    /* Call Slave Addr callback */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
    hsmbus->AddrCallback(hsmbus, TransferDirection, SlaveAddrCode);
#else
    HAL_SMBUS_AddrCallback(hsmbus, TransferDirection, SlaveAddrCode);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
  }
  else if ((SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_RXNE) != RESET) || (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_TCR) != RESET))
  {
    if ((hsmbus->State & HAL_SMBUS_STATE_SLAVE_BUSY_RX) == HAL_SMBUS_STATE_SLAVE_BUSY_RX)
    {
      /* Read data from RXDR */
      *hsmbus->pBuffPtr = (uint8_t)(hsmbus->Instance->RXDR);

      /* Increment Buffer pointer */
      hsmbus->pBuffPtr++;

      hsmbus->XferSize--;
      hsmbus->XferCount--;

      if (hsmbus->XferCount == 1U)
      {
        /* Receive last Byte, can be PEC byte in case of PEC BYTE enabled */
        /* or only the last Byte of Transfer */
        /* So reset the RELOAD bit mode */
        hsmbus->XferOptions &= ~SMBUS_RELOAD_MODE;
        SMBUS_TransferConfig(hsmbus, 0, 1, hsmbus->XferOptions, SMBUS_NO_STARTSTOP);
      }
      else if (hsmbus->XferCount == 0U)
      {
        /* Last Byte is received, disable Interrupt */
        SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_RX);

        /* Remove HAL_SMBUS_STATE_SLAVE_BUSY_RX, keep only HAL_SMBUS_STATE_LISTEN */
        hsmbus->PreviousState = hsmbus->State;
        hsmbus->State &= ~((uint32_t)HAL_SMBUS_STATE_SLAVE_BUSY_RX);

        /* Process Unlocked */
        __HAL_UNLOCK(hsmbus);

        /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
        hsmbus->SlaveRxCpltCallback(hsmbus);
#else
        HAL_SMBUS_SlaveRxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
      }
      else
      {
        /* Set Reload for next Bytes */
        SMBUS_TransferConfig(hsmbus, 0, 1, SMBUS_RELOAD_MODE  | (hsmbus->XferOptions & SMBUS_SENDPEC_MODE), SMBUS_NO_STARTSTOP);

        /* Ack last Byte Read */
        hsmbus->Instance->CR2 &= ~I2C_CR2_NACK;
      }
    }
    else if ((hsmbus->State & HAL_SMBUS_STATE_SLAVE_BUSY_TX) == HAL_SMBUS_STATE_SLAVE_BUSY_TX)
    {
      if ((hsmbus->XferCount != 0U) && (hsmbus->XferSize == 0U))
      {
        if (hsmbus->XferCount > MAX_NBYTE_SIZE)
        {
          SMBUS_TransferConfig(hsmbus, 0, MAX_NBYTE_SIZE, (SMBUS_RELOAD_MODE | (hsmbus->XferOptions & SMBUS_SENDPEC_MODE)), SMBUS_NO_STARTSTOP);
          hsmbus->XferSize = MAX_NBYTE_SIZE;
        }
        else
        {
          hsmbus->XferSize = hsmbus->XferCount;
          SMBUS_TransferConfig(hsmbus, 0, (uint8_t)hsmbus->XferSize, hsmbus->XferOptions, SMBUS_NO_STARTSTOP);
          /* If PEC mode is enable, size to transmit should be Size-1 byte, corresponding to PEC byte */
          /* PEC byte is automatically sent by HW block, no need to manage it in Transmit process */
          if (SMBUS_GET_PEC_MODE(hsmbus) != 0UL)
          {
            hsmbus->XferSize--;
            hsmbus->XferCount--;
          }
        }
      }
    }
    else
    {
      /* Nothing to do */
    }
  }
  else if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_TXIS) != RESET)
  {
    /* Write data to TXDR only if XferCount not reach "0" */
    /* A TXIS flag can be set, during STOP treatment      */
    /* Check if all Data have already been sent */
    /* If it is the case, this last write in TXDR is not sent, correspond to a dummy TXIS event */
    if (hsmbus->XferCount > 0U)
    {
      /* Write data to TXDR */
      hsmbus->Instance->TXDR = *hsmbus->pBuffPtr;

      /* Increment Buffer pointer */
      hsmbus->pBuffPtr++;

      hsmbus->XferCount--;
      hsmbus->XferSize--;
    }

    if (hsmbus->XferCount == 0U)
    {
      /* Last Byte is Transmitted */
      /* Remove HAL_SMBUS_STATE_SLAVE_BUSY_TX, keep only HAL_SMBUS_STATE_LISTEN */
      SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_TX);
      hsmbus->PreviousState = hsmbus->State;
      hsmbus->State &= ~((uint32_t)HAL_SMBUS_STATE_SLAVE_BUSY_TX);

      /* Process Unlocked */
      __HAL_UNLOCK(hsmbus);

      /* Call the corresponding callback to inform upper layer of End of Transfer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
      hsmbus->SlaveTxCpltCallback(hsmbus);
#else
      HAL_SMBUS_SlaveTxCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
    }
  }
  else
  {
    /* Nothing to do */
  }

  /* Check if STOPF is set */
  if (SMBUS_CHECK_FLAG(StatusFlags, SMBUS_FLAG_STOPF) != RESET)
  {
    if ((hsmbus->State & HAL_SMBUS_STATE_LISTEN) == HAL_SMBUS_STATE_LISTEN)
    {
      /* Store Last receive data if any */
      if (__HAL_SMBUS_GET_FLAG(hsmbus, SMBUS_FLAG_RXNE) != RESET)
      {
        /* Read data from RXDR */
        *hsmbus->pBuffPtr = (uint8_t)(hsmbus->Instance->RXDR);

        /* Increment Buffer pointer */
        hsmbus->pBuffPtr++;

        if ((hsmbus->XferSize > 0U))
        {
          hsmbus->XferSize--;
          hsmbus->XferCount--;
        }
      }

      /* Disable RX and TX Interrupts */
      SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_RX | SMBUS_IT_TX);

      /* Disable ADDR Interrupt */
      SMBUS_Disable_IRQ(hsmbus, SMBUS_IT_ADDR);

      /* Disable Address Acknowledge */
      hsmbus->Instance->CR2 |= I2C_CR2_NACK;

      /* Clear Configuration Register 2 */
      SMBUS_RESET_CR2(hsmbus);

      /* Clear STOP Flag */
      __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_STOPF);

      /* Clear ADDR flag */
      __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_ADDR);

      hsmbus->XferOptions = 0;
      hsmbus->PreviousState = hsmbus->State;
      hsmbus->State = HAL_SMBUS_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hsmbus);

      /* Call the Listen Complete callback, to inform upper layer of the end of Listen usecase */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
      hsmbus->ListenCpltCallback(hsmbus);
#else
      HAL_SMBUS_ListenCpltCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
    }
  }

  /* Process Unlocked */
  __HAL_UNLOCK(hsmbus);

  return HAL_OK;
}
/**
  * @brief  Manage the enabling of Interrupts.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  InterruptRequest Value of @ref SMBUS_Interrupt_configuration_definition.
  * @retval HAL status
  */
static void SMBUS_Enable_IRQ(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t InterruptRequest)
{
  uint32_t tmpisr = 0UL;

  if ((InterruptRequest & SMBUS_IT_ALERT) == SMBUS_IT_ALERT)
  {
    /* Enable ERR interrupt */
    tmpisr |= SMBUS_IT_ERRI;
  }

  if ((InterruptRequest & SMBUS_IT_ADDR) == SMBUS_IT_ADDR)
  {
    /* Enable ADDR, STOP interrupt */
    tmpisr |= SMBUS_IT_ADDRI | SMBUS_IT_STOPI | SMBUS_IT_NACKI | SMBUS_IT_ERRI;
  }

  if ((InterruptRequest & SMBUS_IT_TX) == SMBUS_IT_TX)
  {
    /* Enable ERR, TC, STOP, NACK, RXI interrupt */
    tmpisr |= SMBUS_IT_ERRI | SMBUS_IT_TCI | SMBUS_IT_STOPI | SMBUS_IT_NACKI | SMBUS_IT_TXI;
  }

  if ((InterruptRequest & SMBUS_IT_RX) == SMBUS_IT_RX)
  {
    /* Enable ERR, TC, STOP, NACK, TXI interrupt */
    tmpisr |= SMBUS_IT_ERRI | SMBUS_IT_TCI | SMBUS_IT_STOPI | SMBUS_IT_NACKI | SMBUS_IT_RXI;
  }

  /* Enable interrupts only at the end */
  /* to avoid the risk of SMBUS interrupt handle execution before */
  /* all interrupts requested done */
  __HAL_SMBUS_ENABLE_IT(hsmbus, tmpisr);
}
/**
  * @brief  Manage the disabling of Interrupts.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  InterruptRequest Value of @ref SMBUS_Interrupt_configuration_definition.
  * @retval HAL status
  */
static void SMBUS_Disable_IRQ(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t InterruptRequest)
{
  uint32_t tmpisr = 0UL;
  uint32_t tmpstate = hsmbus->State;

  if ((tmpstate == HAL_SMBUS_STATE_READY) && ((InterruptRequest & SMBUS_IT_ALERT) == SMBUS_IT_ALERT))
  {
    /* Disable ERR interrupt */
    tmpisr |= SMBUS_IT_ERRI;
  }

  if ((InterruptRequest & SMBUS_IT_TX) == SMBUS_IT_TX)
  {
    /* Disable TC, STOP, NACK and TXI interrupt */
    tmpisr |= SMBUS_IT_TCI | SMBUS_IT_TXI;

    if ((SMBUS_GET_ALERT_ENABLED(hsmbus) == 0UL)
        && ((tmpstate & HAL_SMBUS_STATE_LISTEN) != HAL_SMBUS_STATE_LISTEN))
    {
      /* Disable ERR interrupt */
      tmpisr |= SMBUS_IT_ERRI;
    }

    if ((tmpstate & HAL_SMBUS_STATE_LISTEN) != HAL_SMBUS_STATE_LISTEN)
    {
      /* Disable STOP and NACK interrupt */
      tmpisr |= SMBUS_IT_STOPI | SMBUS_IT_NACKI;
    }
  }

  if ((InterruptRequest & SMBUS_IT_RX) == SMBUS_IT_RX)
  {
    /* Disable TC, STOP, NACK and RXI interrupt */
    tmpisr |= SMBUS_IT_TCI | SMBUS_IT_RXI;

    if ((SMBUS_GET_ALERT_ENABLED(hsmbus) == 0UL)
        && ((tmpstate & HAL_SMBUS_STATE_LISTEN) != HAL_SMBUS_STATE_LISTEN))
    {
      /* Disable ERR interrupt */
      tmpisr |= SMBUS_IT_ERRI;
    }

    if ((tmpstate & HAL_SMBUS_STATE_LISTEN) != HAL_SMBUS_STATE_LISTEN)
    {
      /* Disable STOP and NACK interrupt */
      tmpisr |= SMBUS_IT_STOPI | SMBUS_IT_NACKI;
    }
  }

  if ((InterruptRequest & SMBUS_IT_ADDR) == SMBUS_IT_ADDR)
  {
    /* Disable ADDR, STOP and NACK interrupt */
    tmpisr |= SMBUS_IT_ADDRI | SMBUS_IT_STOPI | SMBUS_IT_NACKI;

    if (SMBUS_GET_ALERT_ENABLED(hsmbus) == 0UL)
    {
      /* Disable ERR interrupt */
      tmpisr |= SMBUS_IT_ERRI;
    }
  }

  /* Disable interrupts only at the end */
  /* to avoid a breaking situation like at "t" time */
  /* all disable interrupts request are not done */
  __HAL_SMBUS_DISABLE_IT(hsmbus, tmpisr);
}

/**
  * @brief  SMBUS interrupts error handler.
  * @param  hsmbus SMBUS handle.
  * @retval None
  */
static void SMBUS_ITErrorHandler(struct __SMBUS_HandleTypeDef *hsmbus)
{
  uint32_t itflags   = READ_REG(hsmbus->Instance->ISR);
  uint32_t itsources = READ_REG(hsmbus->Instance->CR1);
  uint32_t tmpstate;
  uint32_t tmperror;

  /* SMBUS Bus error interrupt occurred ------------------------------------*/
  if (((itflags & SMBUS_FLAG_BERR) == SMBUS_FLAG_BERR) && ((itsources & SMBUS_IT_ERRI) == SMBUS_IT_ERRI))
  {
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_BERR;

    /* Clear BERR flag */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_BERR);
  }

  /* SMBUS Over-Run/Under-Run interrupt occurred ----------------------------------------*/
  if (((itflags & SMBUS_FLAG_OVR) == SMBUS_FLAG_OVR) && ((itsources & SMBUS_IT_ERRI) == SMBUS_IT_ERRI))
  {
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_OVR;

    /* Clear OVR flag */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_OVR);
  }

  /* SMBUS Arbitration Loss error interrupt occurred ------------------------------------*/
  if (((itflags & SMBUS_FLAG_ARLO) == SMBUS_FLAG_ARLO) && ((itsources & SMBUS_IT_ERRI) == SMBUS_IT_ERRI))
  {
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_ARLO;

    /* Clear ARLO flag */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_ARLO);
  }

  /* SMBUS Timeout error interrupt occurred ---------------------------------------------*/
  if (((itflags & SMBUS_FLAG_TIMEOUT) == SMBUS_FLAG_TIMEOUT) && ((itsources & SMBUS_IT_ERRI) == SMBUS_IT_ERRI))
  {
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_BUSTIMEOUT;

    /* Clear TIMEOUT flag */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_TIMEOUT);
  }

  /* SMBUS Alert error interrupt occurred -----------------------------------------------*/
  if (((itflags & SMBUS_FLAG_ALERT) == SMBUS_FLAG_ALERT) && ((itsources & SMBUS_IT_ERRI) == SMBUS_IT_ERRI))
  {
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_ALERT;

    /* Clear ALERT flag */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_ALERT);
  }

  /* SMBUS Packet Error Check error interrupt occurred ----------------------------------*/
  if (((itflags & SMBUS_FLAG_PECERR) == SMBUS_FLAG_PECERR) && ((itsources & SMBUS_IT_ERRI) == SMBUS_IT_ERRI))
  {
    hsmbus->ErrorCode |= HAL_SMBUS_ERROR_PECERR;

    /* Clear PEC error flag */
    __HAL_SMBUS_CLEAR_FLAG(hsmbus, SMBUS_FLAG_PECERR);
  }

  /* Store current volatile hsmbus->State, misra rule */
  tmperror = hsmbus->ErrorCode;

  /* Call the Error Callback in case of Error detected */
  if ((tmperror != HAL_SMBUS_ERROR_NONE) && (tmperror != HAL_SMBUS_ERROR_ACKF))
  {
    /* Do not Reset the HAL state in case of ALERT error */
    if ((tmperror & HAL_SMBUS_ERROR_ALERT) != HAL_SMBUS_ERROR_ALERT)
    {
      /* Store current volatile hsmbus->State, misra rule */
      tmpstate = hsmbus->State;

      if (((tmpstate & HAL_SMBUS_STATE_SLAVE_BUSY_TX) == HAL_SMBUS_STATE_SLAVE_BUSY_TX)
          || ((tmpstate & HAL_SMBUS_STATE_SLAVE_BUSY_RX) == HAL_SMBUS_STATE_SLAVE_BUSY_RX))
      {
        /* Reset only HAL_SMBUS_STATE_SLAVE_BUSY_XX */
        /* keep HAL_SMBUS_STATE_LISTEN if set */
        hsmbus->PreviousState = HAL_SMBUS_STATE_READY;
        hsmbus->State = HAL_SMBUS_STATE_LISTEN;
      }
    }

    /* Call the Error callback to inform upper layer */
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 1)
    hsmbus->ErrorCallback(hsmbus);
#else
    HAL_SMBUS_ErrorCallback(hsmbus);
#endif /* USE_HAL_SMBUS_REGISTER_CALLBACKS */
  }
}

/**
  * @brief  Handle SMBUS Communication Timeout.
  * @param  hsmbus Pointer to a SMBUS_HandleTypeDef structure that contains
  *                the configuration information for the specified SMBUS.
  * @param  Flag Specifies the SMBUS flag to check.
  * @param  Status The new Flag status (SET or RESET).
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
static HAL_StatusTypeDef SMBUS_WaitOnFlagUntilTimeout(struct __SMBUS_HandleTypeDef *hsmbus, uint32_t Flag, FlagStatus Status, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  /* Wait until flag is set */
  while ((FlagStatus)(__HAL_SMBUS_GET_FLAG(hsmbus, Flag)) == Status)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0UL))
      {
        hsmbus->PreviousState = hsmbus->State;
        hsmbus->State = HAL_SMBUS_STATE_READY;

        /* Update SMBUS error code */
        hsmbus->ErrorCode |= HAL_SMBUS_ERROR_HALTIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hsmbus);

        return HAL_ERROR;
      }
    }
  }

  return HAL_OK;
}

/**
  * @brief  Handle SMBUSx communication when starting transfer or during transfer (TC or TCR flag are set).
  * @param  hsmbus SMBUS handle.
  * @param  DevAddress specifies the slave address to be programmed.
  * @param  Size specifies the number of bytes to be programmed.
  *   This parameter must be a value between 0 and 255.
  * @param  Mode New state of the SMBUS START condition generation.
  *   This parameter can be one or a combination  of the following values:
  *     @arg @ref SMBUS_RELOAD_MODE Enable Reload mode.
  *     @arg @ref SMBUS_AUTOEND_MODE Enable Automatic end mode.
  *     @arg @ref SMBUS_SOFTEND_MODE Enable Software end mode and Reload mode.
  *     @arg @ref SMBUS_SENDPEC_MODE Enable Packet Error Calculation mode.
  * @param  Request New state of the SMBUS START condition generation.
  *   This parameter can be one of the following values:
  *     @arg @ref SMBUS_NO_STARTSTOP Don't Generate stop and start condition.
  *     @arg @ref SMBUS_GENERATE_STOP Generate stop condition (Size should be set to 0).
  *     @arg @ref SMBUS_GENERATE_START_READ Generate Restart for read request.
  *     @arg @ref SMBUS_GENERATE_START_WRITE Generate Restart for write request.
  * @retval None
  */
static void SMBUS_TransferConfig(struct __SMBUS_HandleTypeDef *hsmbus,  uint16_t DevAddress, uint8_t Size, uint32_t Mode, uint32_t Request)
{
  /* Check the parameters */
  assert_param(IS_SMBUS_ALL_INSTANCE(hsmbus->Instance));
  assert_param(IS_SMBUS_TRANSFER_MODE(Mode));
  assert_param(IS_SMBUS_TRANSFER_REQUEST(Request));

  /* update CR2 register */
  MODIFY_REG(hsmbus->Instance->CR2, ((I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | (I2C_CR2_RD_WRN & (uint32_t)(Request >> (31UL - I2C_CR2_RD_WRN_Pos))) | I2C_CR2_START | I2C_CR2_STOP  | I2C_CR2_PECBYTE)), \
             (uint32_t)(((uint32_t)DevAddress & I2C_CR2_SADD) | (((uint32_t)Size << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES) | (uint32_t)Mode | (uint32_t)Request));
}

/**
  * @brief  Convert SMBUSx OTHER_xxx XferOptions to functionnal XferOptions.
  * @param  hsmbus SMBUS handle.
  * @retval None
  */
static void SMBUS_ConvertOtherXferOptions(struct __SMBUS_HandleTypeDef *hsmbus)
{
  /* if user set XferOptions to SMBUS_OTHER_FRAME_NO_PEC   */
  /* it request implicitly to generate a restart condition */
  /* set XferOptions to SMBUS_FIRST_FRAME                  */
  if (hsmbus->XferOptions == SMBUS_OTHER_FRAME_NO_PEC)
  {
    hsmbus->XferOptions = SMBUS_FIRST_FRAME;
  }
  /* else if user set XferOptions to SMBUS_OTHER_FRAME_WITH_PEC */
  /* it request implicitly to generate a restart condition      */
  /* set XferOptions to SMBUS_FIRST_FRAME | SMBUS_SENDPEC_MODE  */
  else if (hsmbus->XferOptions == SMBUS_OTHER_FRAME_WITH_PEC)
  {
    hsmbus->XferOptions = SMBUS_FIRST_FRAME | SMBUS_SENDPEC_MODE;
  }
  /* else if user set XferOptions to SMBUS_OTHER_AND_LAST_FRAME_NO_PEC */
  /* it request implicitly to generate a restart condition             */
  /* then generate a stop condition at the end of transfer             */
  /* set XferOptions to SMBUS_FIRST_AND_LAST_FRAME_NO_PEC              */
  else if (hsmbus->XferOptions == SMBUS_OTHER_AND_LAST_FRAME_NO_PEC)
  {
    hsmbus->XferOptions = SMBUS_FIRST_AND_LAST_FRAME_NO_PEC;
  }
  /* else if user set XferOptions to SMBUS_OTHER_AND_LAST_FRAME_WITH_PEC */
  /* it request implicitly to generate a restart condition               */
  /* then generate a stop condition at the end of transfer               */
  /* set XferOptions to SMBUS_FIRST_AND_LAST_FRAME_WITH_PEC              */
  else if (hsmbus->XferOptions == SMBUS_OTHER_AND_LAST_FRAME_WITH_PEC)
  {
    hsmbus->XferOptions = SMBUS_FIRST_AND_LAST_FRAME_WITH_PEC;
  }
  else
  {
    /* Nothing to do */
  }
}
/**
  * @}
  */

#endif /* HAL_SMBUS_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
