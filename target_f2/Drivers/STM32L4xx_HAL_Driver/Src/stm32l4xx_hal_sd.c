/**
  ******************************************************************************
  * @file    stm32l4xx_hal_sd.c
  * @author  MCD Application Team
  * @brief   SD card HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Secure Digital (SD) peripheral:
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *           + Peripheral State functions
  *
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
  [..]
    This driver implements a high level communication layer for read and write from/to
    this memory. The needed STM32 hardware resources (SDMMC and GPIO) are performed by
    the user in HAL_SD_MspInit() function (MSP layer).
    Basically, the MSP layer configuration should be the same as we provide in the
    examples.
    You can easily tailor this configuration according to hardware resources.

  [..]
    This driver is a generic layered driver for SDMMC memories which uses the HAL
    SDMMC driver functions to interface with SD and uSD cards devices.
    It is used as follows:

    (#)Initialize the SDMMC low level resources by implementing the HAL_SD_MspInit() API:
        (##) Call the function HAL_RCCEx_PeriphCLKConfig with RCC_PERIPHCLK_SDMMC1 for
        PeriphClockSelection and select SDMMC1 clock source (MSI, main PLL or PLLSAI1)
        (##) Enable the SDMMC interface clock using __HAL_RCC_SDMMC1_CLK_ENABLE();
        (##) SDMMC pins configuration for SD card
            (+++) Enable the clock for the SDMMC GPIOs using the functions __HAL_RCC_GPIOx_CLK_ENABLE();
            (+++) Configure these SDMMC pins as alternate function pull-up using HAL_GPIO_Init()
                  and according to your pin assignment;
        (##) On STM32L4Rx/STM32L4Sxx devices, no DMA configuration is need, an internal DMA for SDMMC Peripheral is used.
        (##) On other devices, perform DMA configuration if you need to use DMA process (HAL_SD_ReadBlocks_DMA()
             and HAL_SD_WriteBlocks_DMA() APIs).
            (+++) Enable the DMAx interface clock using __HAL_RCC_DMAx_CLK_ENABLE();
            (+++) Configure the DMA using the function HAL_DMA_Init() with predeclared and filled.
        (##) NVIC configuration if you need to use interrupt process when using DMA transfer.
            (+++) Configure the SDMMC and DMA interrupt priorities using functions
                  HAL_NVIC_SetPriority(); DMA priority is superior to SDMMC's priority
            (+++) Enable the NVIC DMA and SDMMC IRQs using function HAL_NVIC_EnableIRQ()
            (+++) SDMMC interrupts are managed using the macros __HAL_SD_ENABLE_IT()
                  and __HAL_SD_DISABLE_IT() inside the communication process.
            (+++) SDMMC interrupts pending bits are managed using the macros __HAL_SD_GET_IT()
                  and __HAL_SD_CLEAR_IT()
        (##) NVIC configuration if you need to use interrupt process (HAL_SD_ReadBlocks_IT()
             and HAL_SD_WriteBlocks_IT() APIs).
            (+++) Configure the SDMMC interrupt priorities using function HAL_NVIC_SetPriority();
            (+++) Enable the NVIC SDMMC IRQs using function HAL_NVIC_EnableIRQ()
            (+++) SDMMC interrupts are managed using the macros __HAL_SD_ENABLE_IT()
                  and __HAL_SD_DISABLE_IT() inside the communication process.
            (+++) SDMMC interrupts pending bits are managed using the macros __HAL_SD_GET_IT()
                  and __HAL_SD_CLEAR_IT()
    (#) At this stage, you can perform SD read/write/erase operations after SD card initialization


  *** SD Card Initialization and configuration ***
  ================================================
  [..]
    To initialize the SD Card, use the HAL_SD_Init() function. It Initializes
    SDMMC Peripheral(STM32 side) and the SD Card, and put it into StandBy State (Ready for data transfer).
    This function provide the following operations:

    (#) Apply the SD Card initialization process at 400KHz and check the SD Card
        type (Standard Capacity or High Capacity). You can change or adapt this
        frequency by adjusting the "ClockDiv" field.
        The SD Card frequency (SDMMC_CK) is computed as follows:

           SDMMC_CK = SDMMCCLK / (2 * ClockDiv) on STM32L4Rx/STM32L4Sxx devices
           SDMMC_CK = SDMMCCLK / (ClockDiv + 2) on other devices

        In initialization mode and according to the SD Card standard,
        make sure that the SDMMC_CK frequency doesn't exceed 400KHz.

        This phase of initialization is done through SDMMC_Init() and
        SDMMC_PowerState_ON() SDMMC low level APIs.

    (#) Initialize the SD card. The API used is HAL_SD_InitCard().
        This phase allows the card initialization and identification
        and check the SD Card type (Standard Capacity or High Capacity)
        The initialization flow is compatible with SD standard.

        This API (HAL_SD_InitCard()) could be used also to reinitialize the card in case
        of plug-off plug-in.

    (#) Configure the SD Card Data transfer frequency. You can change or adapt this
        frequency by adjusting the "ClockDiv" field.
        In transfer mode and according to the SD Card standard, make sure that the
        SDMMC_CK frequency doesn't exceed 25MHz and 100MHz in High-speed mode switch.

    (#) Select the corresponding SD Card according to the address read with the step 2.

    (#) Configure the SD Card in wide bus mode: 4-bits data.

  *** SD Card Read operation ***
  ==============================
  [..]
    (+) You can read from SD card in polling mode by using function HAL_SD_ReadBlocks().
        This function support only 512-bytes block length (the block size should be
        chosen as 512 bytes).
        You can choose either one block read operation or multiple block read operation
        by adjusting the "NumberOfBlocks" parameter.
        After this, you have to ensure that the transfer is done correctly. The check is done
        through HAL_SD_GetCardState() function for SD card state.

    (+) You can read from SD card in DMA mode by using function HAL_SD_ReadBlocks_DMA().
        This function support only 512-bytes block length (the block size should be
        chosen as 512 bytes).
        You can choose either one block read operation or multiple block read operation
        by adjusting the "NumberOfBlocks" parameter.
        After this, you have to ensure that the transfer is done correctly. The check is done
        through HAL_SD_GetCardState() function for SD card state.
        You could also check the DMA transfer process through the SD Rx interrupt event.

    (+) You can read from SD card in Interrupt mode by using function HAL_SD_ReadBlocks_IT().
        This function support only 512-bytes block length (the block size should be
        chosen as 512 bytes).
        You can choose either one block read operation or multiple block read operation
        by adjusting the "NumberOfBlocks" parameter.
        After this, you have to ensure that the transfer is done correctly. The check is done
        through HAL_SD_GetCardState() function for SD card state.
        You could also check the IT transfer process through the SD Rx interrupt event.

  *** SD Card Write operation ***
  ===============================
  [..]
    (+) You can write to SD card in polling mode by using function HAL_SD_WriteBlocks().
        This function support only 512-bytes block length (the block size should be
        chosen as 512 bytes).
        You can choose either one block read operation or multiple block read operation
        by adjusting the "NumberOfBlocks" parameter.
        After this, you have to ensure that the transfer is done correctly. The check is done
        through HAL_SD_GetCardState() function for SD card state.

    (+) You can write to SD card in DMA mode by using function HAL_SD_WriteBlocks_DMA().
        This function support only 512-bytes block length (the block size should be
        chosen as 512 bytes).
        You can choose either one block read operation or multiple block read operation
        by adjusting the "NumberOfBlocks" parameter.
        After this, you have to ensure that the transfer is done correctly. The check is done
        through HAL_SD_GetCardState() function for SD card state.
        You could also check the DMA transfer process through the SD Tx interrupt event.

    (+) You can write to SD card in Interrupt mode by using function HAL_SD_WriteBlocks_IT().
        This function support only 512-bytes block length (the block size should be
        chosen as 512 bytes).
        You can choose either one block read operation or multiple block read operation
        by adjusting the "NumberOfBlocks" parameter.
        After this, you have to ensure that the transfer is done correctly. The check is done
        through HAL_SD_GetCardState() function for SD card state.
        You could also check the IT transfer process through the SD Tx interrupt event.

  *** SD card status ***
  ======================
  [..]
    (+) The SD Status contains status bits that are related to the SD Memory
        Card proprietary features. To get SD card status use the HAL_SD_GetCardStatus().

  *** SD card information ***
  ===========================
  [..]
    (+) To get SD card information, you can use the function HAL_SD_GetCardInfo().
        It returns useful information about the SD card such as block size, card type,
        block number ...

  *** SD card CSD register ***
  ============================
    (+) The HAL_SD_GetCardCSD() API allows to get the parameters of the CSD register.
        Some of the CSD parameters are useful for card initialization and identification.

  *** SD card CID register ***
  ============================
    (+) The HAL_SD_GetCardCID() API allows to get the parameters of the CID register.
        Some of the CSD parameters are useful for card initialization and identification.

  *** SD HAL driver macros list ***
  ==================================
  [..]
    Below the list of most used macros in SD HAL driver.

    (+) __HAL_SD_ENABLE : Enable the SD device
    (+) __HAL_SD_DISABLE : Disable the SD device
    (+) __HAL_SD_DMA_ENABLE: Enable the SDMMC DMA transfer
    (+) __HAL_SD_DMA_DISABLE: Disable the SDMMC DMA transfer
    (+) __HAL_SD_ENABLE_IT: Enable the SD device interrupt
    (+) __HAL_SD_DISABLE_IT: Disable the SD device interrupt
    (+) __HAL_SD_GET_FLAG:Check whether the specified SD flag is set or not
    (+) __HAL_SD_CLEAR_FLAG: Clear the SD's pending flags

    (@) You can refer to the SD HAL driver header file for more useful macros

  *** Callback registration ***
  =============================================
  [..]
    The compilation define USE_HAL_SD_REGISTER_CALLBACKS when set to 1
    allows the user to configure dynamically the driver callbacks.

    Use Functions @ref HAL_SD_RegisterCallback() to register a user callback,
    it allows to register following callbacks:
      (+) TxCpltCallback : callback when a transmission transfer is completed.
      (+) RxCpltCallback : callback when a reception transfer is completed.
      (+) ErrorCallback : callback when error occurs.
      (+) AbortCpltCallback : callback when abort is completed.
      (+) Read_DMADblBuf0CpltCallback : callback when the DMA reception of first buffer is completed.
      (+) Read_DMADblBuf1CpltCallback : callback when the DMA reception of second buffer is completed.
      (+) Write_DMADblBuf0CpltCallback : callback when the DMA transmission of first buffer is completed.
      (+) Write_DMADblBuf1CpltCallback : callback when the DMA transmission of second buffer is completed.
      (+) MspInitCallback    : SD MspInit.
      (+) MspDeInitCallback  : SD MspDeInit.
    This function takes as parameters the HAL peripheral handle, the Callback ID
    and a pointer to the user callback function.
    For specific callbacks TransceiverCallback use dedicated register callbacks:
    respectively @ref HAL_SD_RegisterTransceiverCallback().

    Use function @ref HAL_SD_UnRegisterCallback() to reset a callback to the default
    weak (surcharged) function. It allows to reset following callbacks:
      (+) TxCpltCallback : callback when a transmission transfer is completed.
      (+) RxCpltCallback : callback when a reception transfer is completed.
      (+) ErrorCallback : callback when error occurs.
      (+) AbortCpltCallback : callback when abort is completed.
      (+) Read_DMADblBuf0CpltCallback : callback when the DMA reception of first buffer is completed.
      (+) Read_DMADblBuf1CpltCallback : callback when the DMA reception of second buffer is completed.
      (+) Write_DMADblBuf0CpltCallback : callback when the DMA transmission of first buffer is completed.
      (+) Write_DMADblBuf1CpltCallback : callback when the DMA transmission of second buffer is completed.
      (+) MspInitCallback    : SD MspInit.
      (+) MspDeInitCallback  : SD MspDeInit.
    This function) takes as parameters the HAL peripheral handle and the Callback ID.
    For specific callbacks TransceiverCallback use dedicated unregister callbacks:
    respectively @ref HAL_SD_UnRegisterTransceiverCallback().

    By default, after the @ref HAL_SD_Init and if the state is HAL_SD_STATE_RESET
    all callbacks are reset to the corresponding legacy weak (surcharged) functions.
    Exception done for MspInit and MspDeInit callbacks that are respectively
    reset to the legacy weak (surcharged) functions in the @ref HAL_SD_Init
    and @ref  HAL_SD_DeInit only when these callbacks are null (not registered beforehand).
    If not, MspInit or MspDeInit are not null, the @ref HAL_SD_Init and @ref HAL_SD_DeInit
    keep and use the user MspInit/MspDeInit callbacks (registered beforehand)

    Callbacks can be registered/unregistered in READY state only.
    Exception done for MspInit/MspDeInit callbacks that can be registered/unregistered
    in READY or RESET state, thus registered (user) MspInit/DeInit callbacks can be used
    during the Init/DeInit.
    In that case first register the MspInit/MspDeInit user callbacks
    using @ref HAL_SD_RegisterCallback before calling @ref HAL_SD_DeInit
    or @ref HAL_SD_Init function.

    When The compilation define USE_HAL_SD_REGISTER_CALLBACKS is set to 0 or
    not defined, the callback registering feature is not available
    and weak (surcharged) callbacks are used.

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
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#if defined(SDMMC1)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup SD
  * @{
  */

#ifdef HAL_SD_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @addtogroup SD_Private_Defines
  * @{
  */

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @defgroup SD_Private_Functions SD Private Functions
  * @{
  */
static uint32_t SD_InitCard       (SD_HandleTypeDef *hsd);
static uint32_t SD_PowerON        (SD_HandleTypeDef *hsd);
static uint32_t SD_SendSDStatus   (SD_HandleTypeDef *hsd, uint32_t *pSDstatus);
static uint32_t SD_SendStatus     (SD_HandleTypeDef *hsd, uint32_t *pCardStatus);
static uint32_t SD_WideBus_Enable (SD_HandleTypeDef *hsd);
static uint32_t SD_WideBus_Disable(SD_HandleTypeDef *hsd);
static uint32_t SD_FindSCR        (SD_HandleTypeDef *hsd, uint32_t *pSCR);
static void     SD_PowerOFF       (SD_HandleTypeDef *hsd);
static void     SD_Write_IT       (SD_HandleTypeDef *hsd);
static void     SD_Read_IT        (SD_HandleTypeDef *hsd);
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
static void     SD_DMATransmitCplt(DMA_HandleTypeDef *hdma);
static void     SD_DMAReceiveCplt (DMA_HandleTypeDef *hdma);
static void     SD_DMAError       (DMA_HandleTypeDef *hdma);
static void     SD_DMATxAbort     (DMA_HandleTypeDef *hdma);
static void     SD_DMARxAbort     (DMA_HandleTypeDef *hdma);
#else
uint32_t        SD_HighSpeed      (SD_HandleTypeDef *hsd);
static uint32_t SD_UltraHighSpeed (SD_HandleTypeDef *hsd);
static uint32_t SD_DDR_Mode       (SD_HandleTypeDef *hsd);
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SD_Exported_Functions
  * @{
  */

/** @addtogroup SD_Exported_Functions_Group1
 *  @brief   Initialization and de-initialization functions
 *
@verbatim
  ==============================================================================
          ##### Initialization and de-initialization functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to initialize/de-initialize the SD
    card device to be ready for use.

@endverbatim
  * @{
  */

/**
  * @brief  Initializes the SD according to the specified parameters in the
            SD_HandleTypeDef and create the associated handle.
  * @param  hsd: Pointer to the SD handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_Init(SD_HandleTypeDef *hsd)
{
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  HAL_SD_CardStatusTypeDef CardStatus;
  uint32_t speedgrade, unitsize;
  uint32_t tickstart;
#endif

  /* Check the SD handle allocation */
  if(hsd == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_SDMMC_ALL_INSTANCE(hsd->Instance));
  assert_param(IS_SDMMC_CLOCK_EDGE(hsd->Init.ClockEdge));
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  assert_param(IS_SDMMC_CLOCK_BYPASS(hsd->Init.ClockBypass));
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */
  assert_param(IS_SDMMC_CLOCK_POWER_SAVE(hsd->Init.ClockPowerSave));
  assert_param(IS_SDMMC_BUS_WIDE(hsd->Init.BusWide));
  assert_param(IS_SDMMC_HARDWARE_FLOW_CONTROL(hsd->Init.HardwareFlowControl));
  assert_param(IS_SDMMC_CLKDIV(hsd->Init.ClockDiv));

  if(hsd->State == HAL_SD_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hsd->Lock = HAL_UNLOCKED;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
    /* Reset Callback pointers in HAL_SD_STATE_RESET only */
    hsd->TxCpltCallback    = HAL_SD_TxCpltCallback;
    hsd->RxCpltCallback    = HAL_SD_RxCpltCallback;
    hsd->ErrorCallback     = HAL_SD_ErrorCallback;
    hsd->AbortCpltCallback = HAL_SD_AbortCallback;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    hsd->Read_DMADblBuf0CpltCallback = HAL_SDEx_Read_DMADoubleBuffer0CpltCallback;
    hsd->Read_DMADblBuf1CpltCallback = HAL_SDEx_Read_DMADoubleBuffer1CpltCallback;
    hsd->Write_DMADblBuf0CpltCallback = HAL_SDEx_Write_DMADoubleBuffer0CpltCallback;
    hsd->Write_DMADblBuf1CpltCallback = HAL_SDEx_Write_DMADoubleBuffer1CpltCallback;
    hsd->DriveTransceiver_1_8V_Callback = HAL_SDEx_DriveTransceiver_1_8V_Callback;
#endif

    if(hsd->MspInitCallback == NULL)
    {
      hsd->MspInitCallback = HAL_SD_MspInit;
    }

    /* Init the low level hardware */
    hsd->MspInitCallback(hsd);
#else
    /* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
    HAL_SD_MspInit(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
  }

  hsd->State = HAL_SD_STATE_BUSY;

  /* Initialize the Card parameters */
  if (HAL_SD_InitCard(hsd) != HAL_OK)
  {
    return HAL_ERROR;
  }

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  if( HAL_SD_GetCardStatus(hsd, &CardStatus) != HAL_OK)
  {
    return HAL_ERROR;
  }
  /* Get Initial Card Speed from Card Status*/
  speedgrade = CardStatus.UhsSpeedGrade;
  unitsize = CardStatus.UhsAllocationUnitSize;
  if ((hsd->SdCard.CardType == CARD_SDHC_SDXC) && ((speedgrade != 0U) || (unitsize != 0U)))
  {
    hsd->SdCard.CardSpeed = CARD_ULTRA_HIGH_SPEED;
  }
  else
  {
    if (hsd->SdCard.CardType == CARD_SDHC_SDXC)
    {
      hsd->SdCard.CardSpeed  = CARD_HIGH_SPEED;
    }
    else
    {
      hsd->SdCard.CardSpeed  = CARD_NORMAL_SPEED;
    }

  }
  /* Configure the bus wide */
  if(HAL_SD_ConfigWideBusOperation(hsd, hsd->Init.BusWide) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Verify that SD card is ready to use after Initialization */
  tickstart = HAL_GetTick();
  while((HAL_SD_GetCardState(hsd) != HAL_SD_CARD_TRANSFER))
  {
    if((HAL_GetTick()-tickstart) >=  SDMMC_DATATIMEOUT)
    {
      hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
      hsd->State= HAL_SD_STATE_READY;
      return HAL_TIMEOUT;
    }
  }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

  /* Initialize the error code */
  hsd->ErrorCode = HAL_SD_ERROR_NONE;

  /* Initialize the SD operation */
  hsd->Context = SD_CONTEXT_NONE;

  /* Initialize the SD state */
  hsd->State = HAL_SD_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Initializes the SD Card.
  * @param  hsd: Pointer to SD handle
  * @note   This function initializes the SD card. It could be used when a card
            re-initialization is needed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_InitCard(SD_HandleTypeDef *hsd)
{
  uint32_t errorstate;
  HAL_StatusTypeDef status;
  SD_InitTypeDef Init;

  /* Default SDMMC peripheral configuration for SD card initialization */
  Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  Init.ClockBypass         = SDMMC_CLOCK_BYPASS_DISABLE;
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */
  Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  Init.BusWide             = SDMMC_BUS_WIDE_1B;
  Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  Init.ClockDiv            = SDMMC_INIT_CLK_DIV;

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  if(hsd->Init.Transceiver == SDMMC_TRANSCEIVER_ENABLE)
  {
    /* Set Transceiver polarity */
    hsd->Instance->POWER |= SDMMC_POWER_DIRPOL;
  }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

  /* Initialize SDMMC peripheral interface with default configuration */
  status = SDMMC_Init(hsd->Instance, Init);
  if(status != HAL_OK)
  {
    return HAL_ERROR;
  }

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  /* Disable SDMMC Clock */
  __HAL_SD_DISABLE(hsd);
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

  /* Set Power State to ON */
  status = SDMMC_PowerState_ON(hsd->Instance);
  if(status != HAL_OK)
  {
    return HAL_ERROR;
  }

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  /* Enable SDMMC Clock */
  __HAL_SD_ENABLE(hsd);
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

  /* Identify card operating voltage */
  errorstate = SD_PowerON(hsd);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    hsd->State = HAL_SD_STATE_READY;
    hsd->ErrorCode |= errorstate;
    return HAL_ERROR;
  }

  /* Card initialization */
  errorstate = SD_InitCard(hsd);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    hsd->State = HAL_SD_STATE_READY;
    hsd->ErrorCode |= errorstate;
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  De-Initializes the SD card.
  * @param  hsd: Pointer to SD handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_DeInit(SD_HandleTypeDef *hsd)
{
  /* Check the SD handle allocation */
  if(hsd == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_SDMMC_ALL_INSTANCE(hsd->Instance));

  hsd->State = HAL_SD_STATE_BUSY;

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  /* Desactivate the 1.8V Mode */
  if(hsd->Init.Transceiver == SDMMC_TRANSCEIVER_ENABLE)
  {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
    if(hsd->DriveTransceiver_1_8V_Callback == NULL)
    {
      hsd->DriveTransceiver_1_8V_Callback = HAL_SDEx_DriveTransceiver_1_8V_Callback;
    }
    hsd->DriveTransceiver_1_8V_Callback(RESET);
#else
    HAL_SDEx_DriveTransceiver_1_8V_Callback(RESET);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
  }
#endif

  /* Set SD power state to off */
  SD_PowerOFF(hsd);

#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
  if(hsd->MspDeInitCallback == NULL)
  {
    hsd->MspDeInitCallback = HAL_SD_MspDeInit;
  }

  /* DeInit the low level hardware */
  hsd->MspDeInitCallback(hsd);
#else
  /* De-Initialize the MSP layer */
  HAL_SD_MspDeInit(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */

  hsd->ErrorCode = HAL_SD_ERROR_NONE;
  hsd->State = HAL_SD_STATE_RESET;

  return HAL_OK;
}


/**
  * @brief  Initializes the SD MSP.
  * @param  hsd: Pointer to SD handle
  * @retval None
  */
__weak void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SD_MspInit could be implemented in the user file
   */
}

/**
  * @brief  De-Initialize SD MSP.
  * @param  hsd: Pointer to SD handle
  * @retval None
  */
__weak void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SD_MspDeInit could be implemented in the user file
   */
}

/**
  * @}
  */

/** @addtogroup SD_Exported_Functions_Group2
 *  @brief   Data transfer functions
 *
@verbatim
  ==============================================================================
                        ##### IO operation functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to manage the data
    transfer from/to SD card.

@endverbatim
  * @{
  */

/**
  * @brief  Reads block(s) from a specified address in a card. The Data transfer
  *         is managed by polling mode.
  * @note   This API should be followed by a check on the card state through
  *         HAL_SD_GetCardState().
  * @param  hsd: Pointer to SD handle
  * @param  pData: pointer to the buffer that will contain the received data
  * @param  BlockAdd: Block Address from where data is to be read
  * @param  NumberOfBlocks: Number of SD blocks to read
  * @param  Timeout: Specify timeout value
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_ReadBlocks(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t tickstart = HAL_GetTick();
  uint32_t count, data, dataremaining;
  uint32_t add = BlockAdd;
  uint8_t *tempbuff = pData;

  if(NULL == pData)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    return HAL_ERROR;
  }

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_BUSY;

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0U;

    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      add *= 512U;
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = NumberOfBlocks * BLOCKSIZE;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    config.DPSM          = SDMMC_DPSM_DISABLE;
#else
    config.DPSM          = SDMMC_DPSM_ENABLE;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
    (void)SDMMC_ConfigData(hsd->Instance, &config);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    __SDMMC_CMDTRANS_ENABLE( hsd->Instance);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    /* Read block(s) in polling mode */
    if(NumberOfBlocks > 1U)
    {
      hsd->Context = SD_CONTEXT_READ_MULTIPLE_BLOCK;

      /* Read Multi Block command */
      errorstate = SDMMC_CmdReadMultiBlock(hsd->Instance, add);
    }
    else
    {
      hsd->Context = SD_CONTEXT_READ_SINGLE_BLOCK;

      /* Read Single Block command */
      errorstate = SDMMC_CmdReadSingleBlock(hsd->Instance, add);
    }
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }

    /* Poll on SDMMC flags */
    dataremaining = config.DataLength;
    while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
    {
      if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF) && (dataremaining > 0U))
      {
        /* Read data from SDMMC Rx FIFO */
        for(count = 0U; count < 8U; count++)
        {
          data = SDMMC_ReadFIFO(hsd->Instance);
          *tempbuff = (uint8_t)(data & 0xFFU);
          tempbuff++;
          dataremaining--;
          *tempbuff = (uint8_t)((data >> 8U) & 0xFFU);
          tempbuff++;
          dataremaining--;
          *tempbuff = (uint8_t)((data >> 16U) & 0xFFU);
          tempbuff++;
          dataremaining--;
          *tempbuff = (uint8_t)((data >> 24U) & 0xFFU);
          tempbuff++;
          dataremaining--;
        }
      }

      if(((HAL_GetTick()-tickstart) >=  Timeout) || (Timeout == 0U))
      {
        /* Clear all the static flags */
        __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
        hsd->ErrorCode |= HAL_SD_ERROR_TIMEOUT;
        hsd->State= HAL_SD_STATE_READY;
        hsd->Context = SD_CONTEXT_NONE;
        return HAL_TIMEOUT;
      }
    }
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    __SDMMC_CMDTRANS_DISABLE( hsd->Instance);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    /* Send stop transmission command in case of multiblock read */
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
    {
      if(hsd->SdCard.CardType != CARD_SECURED)
      {
        /* Send stop transmission command */
        errorstate = SDMMC_CmdStopTransfer(hsd->Instance);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
          /* Clear all the static flags */
          __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
          hsd->ErrorCode |= errorstate;
          hsd->State = HAL_SD_STATE_READY;
          hsd->Context = SD_CONTEXT_NONE;
          return HAL_ERROR;
        }
      }
    }

    /* Get error state */
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_DATA_TIMEOUT;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }
    else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_DATA_CRC_FAIL;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }
    else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_RX_OVERRUN;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }
    else
    {
      /* Nothing to do */
    }

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
    /* Empty FIFO if there is still any data */
    while ((__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXDAVL)) && (dataremaining > 0U))
    {
      data = SDMMC_ReadFIFO(hsd->Instance);
      *tempbuff = (uint8_t)(data & 0xFFU);
      tempbuff++;
      dataremaining--;
      *tempbuff = (uint8_t)((data >> 8U) & 0xFFU);
      tempbuff++;
      dataremaining--;
      *tempbuff = (uint8_t)((data >> 16U) & 0xFFU);
      tempbuff++;
      dataremaining--;
      *tempbuff = (uint8_t)((data >> 24U) & 0xFFU);
      tempbuff++;
      dataremaining--;

      if(((HAL_GetTick()-tickstart) >=  Timeout) || (Timeout == 0U))
      {
        /* Clear all the static flags */
        __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
        hsd->ErrorCode |= HAL_SD_ERROR_TIMEOUT;
        hsd->State= HAL_SD_STATE_READY;
        hsd->Context = SD_CONTEXT_NONE;
        return HAL_ERROR;
      }
    }
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    hsd->State = HAL_SD_STATE_READY;

    return HAL_OK;
  }
  else
  {
    hsd->ErrorCode |= HAL_SD_ERROR_BUSY;
    return HAL_ERROR;
  }
}

/**
  * @brief  Allows to write block(s) to a specified address in a card. The Data
  *         transfer is managed by polling mode.
  * @note   This API should be followed by a check on the card state through
  *         HAL_SD_GetCardState().
  * @param  hsd: Pointer to SD handle
  * @param  pData: pointer to the buffer that will contain the data to transmit
  * @param  BlockAdd: Block Address where data will be written
  * @param  NumberOfBlocks: Number of SD blocks to write
  * @param  Timeout: Specify timeout value
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_WriteBlocks(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t tickstart = HAL_GetTick();
  uint32_t count, data, dataremaining;
  uint32_t add = BlockAdd;
  uint8_t *tempbuff = pData;

  if(NULL == pData)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    return HAL_ERROR;
  }

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_BUSY;

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0U;

    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      add *= 512U;
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = NumberOfBlocks * BLOCKSIZE;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    config.DPSM          = SDMMC_DPSM_DISABLE;
#else
    config.DPSM          = SDMMC_DPSM_ENABLE;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
    (void)SDMMC_ConfigData(hsd->Instance, &config);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    __SDMMC_CMDTRANS_ENABLE( hsd->Instance);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    /* Write Blocks in Polling mode */
    if(NumberOfBlocks > 1U)
    {
      hsd->Context = SD_CONTEXT_WRITE_MULTIPLE_BLOCK;

      /* Write Multi Block command */
      errorstate = SDMMC_CmdWriteMultiBlock(hsd->Instance, add);
    }
    else
    {
      hsd->Context = SD_CONTEXT_WRITE_SINGLE_BLOCK;

      /* Write Single Block command */
      errorstate = SDMMC_CmdWriteSingleBlock(hsd->Instance, add);
    }
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }

    /* Write block(s) in polling mode */
    dataremaining = config.DataLength;
    while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
    {
      if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_TXFIFOHE) && (dataremaining > 0U))
      {
        /* Write data to SDMMC Tx FIFO */
        for(count = 0U; count < 8U; count++)
        {
          data = (uint32_t)(*tempbuff);
          tempbuff++;
          dataremaining--;
          data |= ((uint32_t)(*tempbuff) << 8U);
          tempbuff++;
          dataremaining--;
          data |= ((uint32_t)(*tempbuff) << 16U);
          tempbuff++;
          dataremaining--;
          data |= ((uint32_t)(*tempbuff) << 24U);
          tempbuff++;
          dataremaining--;
          (void)SDMMC_WriteFIFO(hsd->Instance, &data);
        }
      }

      if(((HAL_GetTick()-tickstart) >=  Timeout) || (Timeout == 0U))
      {
        /* Clear all the static flags */
        __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
        hsd->ErrorCode |= errorstate;
        hsd->State = HAL_SD_STATE_READY;
        hsd->Context = SD_CONTEXT_NONE;
        return HAL_TIMEOUT;
      }
    }
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    __SDMMC_CMDTRANS_DISABLE( hsd->Instance);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    /* Send stop transmission command in case of multiblock write */
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
    {
      if(hsd->SdCard.CardType != CARD_SECURED)
      {
        /* Send stop transmission command */
        errorstate = SDMMC_CmdStopTransfer(hsd->Instance);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
          /* Clear all the static flags */
          __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
          hsd->ErrorCode |= errorstate;
          hsd->State = HAL_SD_STATE_READY;
          hsd->Context = SD_CONTEXT_NONE;
          return HAL_ERROR;
        }
      }
    }

    /* Get error state */
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_DATA_TIMEOUT;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }
    else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_DATA_CRC_FAIL;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }
    else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_TXUNDERR))
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_TX_UNDERRUN;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }
    else
    {
      /* Nothing to do */
    }

    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    hsd->State = HAL_SD_STATE_READY;

    return HAL_OK;
  }
  else
  {
    hsd->ErrorCode |= HAL_SD_ERROR_BUSY;
    return HAL_ERROR;
  }
}

/**
  * @brief  Reads block(s) from a specified address in a card. The Data transfer
  *         is managed in interrupt mode.
  * @note   This API should be followed by a check on the card state through
  *         HAL_SD_GetCardState().
  * @note   You could also check the IT transfer process through the SD Rx
  *         interrupt event.
  * @param  hsd: Pointer to SD handle
  * @param  pData: Pointer to the buffer that will contain the received data
  * @param  BlockAdd: Block Address from where data is to be read
  * @param  NumberOfBlocks: Number of blocks to read.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_ReadBlocks_IT(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t add = BlockAdd;

  if(NULL == pData)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    return HAL_ERROR;
  }

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_BUSY;

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0U;

    hsd->pRxBuffPtr = pData;
    hsd->RxXferSize = BLOCKSIZE * NumberOfBlocks;

    __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND | SDMMC_FLAG_RXFIFOHF));

    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      add *= 512U;
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    config.DPSM          = SDMMC_DPSM_DISABLE;
#else
    config.DPSM          = SDMMC_DPSM_ENABLE;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
    (void)SDMMC_ConfigData(hsd->Instance, &config);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    __SDMMC_CMDTRANS_ENABLE( hsd->Instance);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    /* Read Blocks in IT mode */
    if(NumberOfBlocks > 1U)
    {
      hsd->Context = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_IT);

      /* Read Multi Block command */
      errorstate = SDMMC_CmdReadMultiBlock(hsd->Instance, add);
    }
    else
    {
      hsd->Context = (SD_CONTEXT_READ_SINGLE_BLOCK | SD_CONTEXT_IT);

      /* Read Single Block command */
      errorstate = SDMMC_CmdReadSingleBlock(hsd->Instance, add);
    }
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Writes block(s) to a specified address in a card. The Data transfer
  *         is managed in interrupt mode.
  * @note   This API should be followed by a check on the card state through
  *         HAL_SD_GetCardState().
  * @note   You could also check the IT transfer process through the SD Tx
  *         interrupt event.
  * @param  hsd: Pointer to SD handle
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  BlockAdd: Block Address where data will be written
  * @param  NumberOfBlocks: Number of blocks to write
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_WriteBlocks_IT(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t add = BlockAdd;

  if(NULL == pData)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    return HAL_ERROR;
  }

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_BUSY;

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0U;

    hsd->pTxBuffPtr = pData;
    hsd->TxXferSize = BLOCKSIZE * NumberOfBlocks;

    /* Enable transfer interrupts */
    __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND | SDMMC_FLAG_TXFIFOHE));

    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      add *= 512U;
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_DISABLE;
    (void)SDMMC_ConfigData(hsd->Instance, &config);

    __SDMMC_CMDTRANS_ENABLE( hsd->Instance);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    /* Write Blocks in Polling mode */
    if(NumberOfBlocks > 1U)
    {
      hsd->Context = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK| SD_CONTEXT_IT);

      /* Write Multi Block command */
      errorstate = SDMMC_CmdWriteMultiBlock(hsd->Instance, add);
    }
    else
    {
      hsd->Context = (SD_CONTEXT_WRITE_SINGLE_BLOCK | SD_CONTEXT_IT);

      /* Write Single Block command */
      errorstate = SDMMC_CmdWriteSingleBlock(hsd->Instance, add);
    }
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_ENABLE;
    (void)SDMMC_ConfigData(hsd->Instance, &config);
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Reads block(s) from a specified address in a card. The Data transfer
  *         is managed by DMA mode.
  * @note   This API should be followed by a check on the card state through
  *         HAL_SD_GetCardState().
  * @note   You could also check the DMA transfer process through the SD Rx
  *         interrupt event.
  * @param  hsd: Pointer SD handle
  * @param  pData: Pointer to the buffer that will contain the received data
  * @param  BlockAdd: Block Address from where data is to be read
  * @param  NumberOfBlocks: Number of blocks to read.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_ReadBlocks_DMA(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t add = BlockAdd;

  if(NULL == pData)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    return HAL_ERROR;
  }

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_BUSY;

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0U;

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
    __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND));

    /* Set the DMA transfer complete callback */
    hsd->hdmarx->XferCpltCallback = SD_DMAReceiveCplt;

    /* Set the DMA error callback */
    hsd->hdmarx->XferErrorCallback = SD_DMAError;

    /* Set the DMA Abort callback */
    hsd->hdmarx->XferAbortCallback = NULL;

    /* Enable the DMA Channel */
    if(HAL_DMA_Start_IT(hsd->hdmarx, (uint32_t)&hsd->Instance->FIFO, (uint32_t)pData, (uint32_t)(BLOCKSIZE * NumberOfBlocks)/4U) != HAL_OK)
    {
      __HAL_SD_DISABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND));
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_DMA;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }
    else
    {
      /* Enable SD DMA transfer */
      __HAL_SD_DMA_ENABLE(hsd);
#else
      hsd->pRxBuffPtr = pData;
      hsd->RxXferSize = BLOCKSIZE * NumberOfBlocks;
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

      if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
      {
        add *= 512U;
      }

      /* Set Block Size for Card */
      errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
      if(errorstate != HAL_SD_ERROR_NONE)
      {
        /* Clear all the static flags */
        __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
        hsd->ErrorCode |= errorstate;
        hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
      }

      /* Configure the SD DPSM (Data Path State Machine) */
      config.DataTimeOut   = SDMMC_DATATIMEOUT;
      config.DataLength    = BLOCKSIZE * NumberOfBlocks;
      config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
      config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
      config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
      config.DPSM          = SDMMC_DPSM_DISABLE;
#else
      config.DPSM          = SDMMC_DPSM_ENABLE;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
      (void)SDMMC_ConfigData(hsd->Instance, &config);

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
      /* Enable transfer interrupts */
      __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND));

      __SDMMC_CMDTRANS_ENABLE( hsd->Instance);
      hsd->Instance->IDMABASE0 = (uint32_t) pData ;
      hsd->Instance->IDMACTRL  = SDMMC_ENABLE_IDMA_SINGLE_BUFF;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

      /* Read Blocks in DMA mode */
      if(NumberOfBlocks > 1U)
      {
        hsd->Context = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

        /* Read Multi Block command */
        errorstate = SDMMC_CmdReadMultiBlock(hsd->Instance, add);
      }
      else
      {
        hsd->Context = (SD_CONTEXT_READ_SINGLE_BLOCK | SD_CONTEXT_DMA);

        /* Read Single Block command */
        errorstate = SDMMC_CmdReadSingleBlock(hsd->Instance, add);
      }
      if(errorstate != HAL_SD_ERROR_NONE)
      {
        /* Clear all the static flags */
        __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
        __HAL_SD_DISABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND));
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
        hsd->ErrorCode |= errorstate;
        hsd->State = HAL_SD_STATE_READY;
        hsd->Context = SD_CONTEXT_NONE;
        return HAL_ERROR;
      }

      return HAL_OK;
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
    }
#endif
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Writes block(s) to a specified address in a card. The Data transfer
  *         is managed by DMA mode.
  * @note   This API should be followed by a check on the card state through
  *         HAL_SD_GetCardState().
  * @note   You could also check the DMA transfer process through the SD Tx
  *         interrupt event.
  * @param  hsd: Pointer to SD handle
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  BlockAdd: Block Address where data will be written
  * @param  NumberOfBlocks: Number of blocks to write
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_WriteBlocks_DMA(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t add = BlockAdd;

  if(NULL == pData)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    return HAL_ERROR;
  }

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    if((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_BUSY;

    /* Initialize data control register */
    hsd->Instance->DCTRL = 0U;

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    hsd->pTxBuffPtr = pData;
    hsd->TxXferSize = BLOCKSIZE * NumberOfBlocks;
#else
    /* Enable SD Error interrupts */
    __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR));

    /* Set the DMA transfer complete callback */
    hsd->hdmatx->XferCpltCallback = SD_DMATransmitCplt;

    /* Set the DMA error callback */
    hsd->hdmatx->XferErrorCallback = SD_DMAError;

    /* Set the DMA Abort callback */
    hsd->hdmatx->XferAbortCallback = NULL;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      add *= 512U;
    }

    /* Set Block Size for Card */
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    /* Configure the SD DPSM (Data Path State Machine) */
    config.DataTimeOut   = SDMMC_DATATIMEOUT;
    config.DataLength    = BLOCKSIZE * NumberOfBlocks;
    config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
    config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
    config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    config.DPSM          = SDMMC_DPSM_DISABLE;
    (void)SDMMC_ConfigData(hsd->Instance, &config);

    /* Enable transfer interrupts */
    __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND));

    __SDMMC_CMDTRANS_ENABLE( hsd->Instance);

    hsd->Instance->IDMABASE0 = (uint32_t) pData ;
    hsd->Instance->IDMACTRL  = SDMMC_ENABLE_IDMA_SINGLE_BUFF;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    /* Write Blocks in Polling mode */
    if(NumberOfBlocks > 1U)
    {
      hsd->Context = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

      /* Write Multi Block command */
      errorstate = SDMMC_CmdWriteMultiBlock(hsd->Instance, add);
    }
    else
    {
      hsd->Context = (SD_CONTEXT_WRITE_SINGLE_BLOCK | SD_CONTEXT_DMA);

      /* Write Single Block command */
      errorstate = SDMMC_CmdWriteSingleBlock(hsd->Instance, add);
    }
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
      __HAL_SD_DISABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND));
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
    /* Enable SDMMC DMA transfer */
    __HAL_SD_DMA_ENABLE(hsd);

    /* Enable the DMA Channel */
    if(HAL_DMA_Start_IT(hsd->hdmatx, (uint32_t)pData, (uint32_t)&hsd->Instance->FIFO, (uint32_t)(BLOCKSIZE * NumberOfBlocks)/4U) != HAL_OK)
    {
      __HAL_SD_DISABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR));   
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_DMA;
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      return HAL_ERROR;
    }
    else
    {
      /* Configure the SD DPSM (Data Path State Machine) */
      config.DataTimeOut   = SDMMC_DATATIMEOUT;
      config.DataLength    = BLOCKSIZE * NumberOfBlocks;
      config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
      config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
      config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
      config.DPSM          = SDMMC_DPSM_ENABLE;
      (void)SDMMC_ConfigData(hsd->Instance, &config);
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

      return HAL_OK;
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
    }
#endif
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @note   This API should be followed by a check on the card state through
  *         HAL_SD_GetCardState().
  * @param  hsd: Pointer to SD handle
  * @param  BlockStartAdd: Start Block address
  * @param  BlockEndAdd: End Block address
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_Erase(SD_HandleTypeDef *hsd, uint32_t BlockStartAdd, uint32_t BlockEndAdd)
{
  uint32_t errorstate;
  uint32_t start_add = BlockStartAdd;
  uint32_t end_add = BlockEndAdd;

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    if(end_add < start_add)
    {
      hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
      return HAL_ERROR;
    }

    if(end_add > (hsd->SdCard.LogBlockNbr))
    {
      hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_BUSY;

    /* Check if the card command class supports erase command */
    if(((hsd->SdCard.Class) & SDMMC_CCCC_ERASE) == 0U)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    if((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    /* Get start and end block for high capacity cards */
    if(hsd->SdCard.CardType != CARD_SDHC_SDXC)
    {
      start_add *= 512U;
      end_add   *= 512U;
    }

    /* According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
    if(hsd->SdCard.CardType != CARD_SECURED)
    {
      /* Send CMD32 SD_ERASE_GRP_START with argument as addr  */
      errorstate = SDMMC_CmdSDEraseStartAdd(hsd->Instance, start_add);
      if(errorstate != HAL_SD_ERROR_NONE)
      {
        /* Clear all the static flags */
        __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
        hsd->ErrorCode |= errorstate;
        hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
      }

      /* Send CMD33 SD_ERASE_GRP_END with argument as addr  */
      errorstate = SDMMC_CmdSDEraseEndAdd(hsd->Instance, end_add);
      if(errorstate != HAL_SD_ERROR_NONE)
      {
        /* Clear all the static flags */
        __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
        hsd->ErrorCode |= errorstate;
        hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
      }
    }

    /* Send CMD38 ERASE */
    errorstate = SDMMC_CmdErase(hsd->Instance);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
      hsd->ErrorCode |= errorstate;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_ERROR;
    }

    hsd->State = HAL_SD_STATE_READY;

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  This function handles SD card interrupt request.
  * @param  hsd: Pointer to SD handle
  * @retval None
  */
void HAL_SD_IRQHandler(SD_HandleTypeDef *hsd)
{
  uint32_t errorstate;
  uint32_t context = hsd->Context;

  /* Check for SDMMC interrupt flags */
  if((__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF) != RESET) && ((context & SD_CONTEXT_IT) != 0U))
  {
    SD_Read_IT(hsd);
  }

  else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DATAEND) != RESET)
  {
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DATAEND);

    __HAL_SD_DISABLE_IT(hsd, SDMMC_IT_DATAEND  | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT |\
                             SDMMC_IT_TXUNDERR | SDMMC_IT_RXOVERR  | SDMMC_IT_TXFIFOHE |\
                             SDMMC_IT_RXFIFOHF);

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    __HAL_SD_DISABLE_IT(hsd, SDMMC_IT_IDMABTC);
    __SDMMC_CMDTRANS_DISABLE( hsd->Instance);
#else
    hsd->Instance->DCTRL &= ~(SDMMC_DCTRL_DTEN);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    if((context & SD_CONTEXT_IT) != 0U)
    {
      if(((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) != 0U) || ((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U))
      {
        errorstate = SDMMC_CmdStopTransfer(hsd->Instance);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= errorstate;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
          hsd->ErrorCallback(hsd);
#else
          HAL_SD_ErrorCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
        }
      }

      /* Clear all the static flags */
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      if(((context & SD_CONTEXT_READ_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) != 0U))
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->RxCpltCallback(hsd);
#else
        HAL_SD_RxCpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
      else
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->TxCpltCallback(hsd);
#else
        HAL_SD_TxCpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
    }
    else if((context & SD_CONTEXT_DMA) != 0U)
    {
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
      hsd->Instance->DLEN = 0;
      hsd->Instance->DCTRL = 0;
      hsd->Instance->IDMACTRL = SDMMC_DISABLE_IDMA;

      /* Stop Transfer for Write Multi blocks or Read Multi blocks */
      if(((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) != 0U) || ((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U))
      {
        errorstate = SDMMC_CmdStopTransfer(hsd->Instance);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= errorstate;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
          hsd->ErrorCallback(hsd);
#else
          HAL_SD_ErrorCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
        }
      }

      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
      if(((context & SD_CONTEXT_WRITE_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U))
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->TxCpltCallback(hsd);
#else
        HAL_SD_TxCpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
      if(((context & SD_CONTEXT_READ_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) != 0U))
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->RxCpltCallback(hsd);
#else
        HAL_SD_RxCpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
#else
      if((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U)
      {
        errorstate = SDMMC_CmdStopTransfer(hsd->Instance);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= errorstate;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
          hsd->ErrorCallback(hsd);
#else
          HAL_SD_ErrorCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
        }
      }
      if(((context & SD_CONTEXT_READ_SINGLE_BLOCK) == 0U) && ((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) == 0U))
      {
        /* Disable the DMA transfer for transmit request by setting the DMAEN bit
        in the SD DCTRL register */
        hsd->Instance->DCTRL &= (uint32_t)~((uint32_t)SDMMC_DCTRL_DMAEN);

        hsd->State = HAL_SD_STATE_READY;

#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->TxCpltCallback(hsd);
#else
        HAL_SD_TxCpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
    }
    else
    {
      /* Nothing to do */
    }
  }

  else if((__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_TXFIFOHE) != RESET) && ((context & SD_CONTEXT_IT) != 0U))
  {
    SD_Write_IT(hsd);
  }

  else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_RXOVERR | SDMMC_FLAG_TXUNDERR) != RESET)
  {
    /* Set Error code */
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_IT_DCRCFAIL) != RESET)
    {
      hsd->ErrorCode |= HAL_SD_ERROR_DATA_CRC_FAIL;
    }
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_IT_DTIMEOUT) != RESET)
    {
      hsd->ErrorCode |= HAL_SD_ERROR_DATA_TIMEOUT;
    }
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_IT_RXOVERR) != RESET)
    {
      hsd->ErrorCode |= HAL_SD_ERROR_RX_OVERRUN;
    }
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_IT_TXUNDERR) != RESET)
    {
      hsd->ErrorCode |= HAL_SD_ERROR_TX_UNDERRUN;
    }

    /* Clear All flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    /* Disable all interrupts */
    __HAL_SD_DISABLE_IT(hsd, SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT|\
                             SDMMC_IT_TXUNDERR| SDMMC_IT_RXOVERR);

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    __SDMMC_CMDTRANS_DISABLE( hsd->Instance);
    hsd->Instance->DCTRL |= SDMMC_DCTRL_FIFORST;
    hsd->Instance->CMD |= SDMMC_CMD_CMDSTOP;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
    hsd->ErrorCode |= SDMMC_CmdStopTransfer(hsd->Instance);
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    hsd->Instance->CMD &= ~(SDMMC_CMD_CMDSTOP);
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DABORT);
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    if((context & SD_CONTEXT_IT) != 0U)
    {
      /* Set the SD state to ready to be able to start again the process */
      hsd->State = HAL_SD_STATE_READY;
      hsd->Context = SD_CONTEXT_NONE;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
      hsd->ErrorCallback(hsd);
#else
      HAL_SD_ErrorCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
    }
    else if((context & SD_CONTEXT_DMA) != 0U)
    {
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
      if(hsd->ErrorCode != HAL_SD_ERROR_NONE)
      {
        /* Disable Internal DMA */
        __HAL_SD_DISABLE_IT(hsd, SDMMC_IT_IDMABTC);
        hsd->Instance->IDMACTRL = SDMMC_DISABLE_IDMA;

        /* Set the SD state to ready to be able to start again the process */
        hsd->State = HAL_SD_STATE_READY;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->ErrorCallback(hsd);
#else
        HAL_SD_ErrorCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
#else
      /* Abort the SD DMA channel */
      if(((context & SD_CONTEXT_WRITE_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U))
      {
        /* Set the DMA Tx abort callback */
        hsd->hdmatx->XferAbortCallback = SD_DMATxAbort;
        /* Abort DMA in IT mode */
        if(HAL_DMA_Abort_IT(hsd->hdmatx) != HAL_OK)
        {
          SD_DMATxAbort(hsd->hdmatx);
        }
      }
      else if(((context & SD_CONTEXT_READ_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) != 0U))
      {
        /* Set the DMA Rx abort callback */
        hsd->hdmarx->XferAbortCallback = SD_DMARxAbort;
        /* Abort DMA in IT mode */
        if(HAL_DMA_Abort_IT(hsd->hdmarx) != HAL_OK)
        {
          SD_DMARxAbort(hsd->hdmarx);
        }
      }
      else
      {
        hsd->ErrorCode = HAL_SD_ERROR_NONE;
        hsd->State = HAL_SD_STATE_READY;
        hsd->Context = SD_CONTEXT_NONE;
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->AbortCpltCallback(hsd);
#else
        HAL_SD_AbortCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
    }
    else
    {
      /* Nothing to do */
    }
  }

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_IDMABTC) != RESET)
  {
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_IDMABTC);
    if(READ_BIT(hsd->Instance->IDMACTRL, SDMMC_IDMA_IDMABACT) == 0U)
    {
      /* Current buffer is buffer0, Transfer complete for buffer1 */
      if((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U)
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->Write_DMADblBuf1CpltCallback(hsd);
#else
        HAL_SDEx_Write_DMADoubleBuffer1CpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
      else /* SD_CONTEXT_READ_MULTIPLE_BLOCK */
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->Read_DMADblBuf1CpltCallback(hsd);
#else
        HAL_SDEx_Read_DMADoubleBuffer1CpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
    }
    else /* SD_DMA_BUFFER1 */
    {
      /* Current buffer is buffer1, Transfer complete for buffer0 */
      if((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U)
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->Write_DMADblBuf0CpltCallback(hsd);
#else
        HAL_SDEx_Write_DMADoubleBuffer0CpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
      else /* SD_CONTEXT_READ_MULTIPLE_BLOCK */
      {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
        hsd->Read_DMADblBuf0CpltCallback(hsd);
#else
        HAL_SDEx_Read_DMADoubleBuffer0CpltCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
      }
    }
  }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
  else
  {
    /* Nothing to do */
  }
}

/**
  * @brief return the SD state
  * @param hsd: Pointer to sd handle
  * @retval HAL state
  */
HAL_SD_StateTypeDef HAL_SD_GetState(SD_HandleTypeDef *hsd)
{
  return hsd->State;
}

/**
* @brief  Return the SD error code
* @param  hsd : Pointer to a SD_HandleTypeDef structure that contains
  *              the configuration information.
* @retval SD Error Code
*/
uint32_t HAL_SD_GetError(SD_HandleTypeDef *hsd)
{
  return hsd->ErrorCode;
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: Pointer to SD handle
  * @retval None
  */
__weak void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SD_TxCpltCallback can be implemented in the user file
   */
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: Pointer SD handle
  * @retval None
  */
__weak void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SD_RxCpltCallback can be implemented in the user file
   */
}

/**
  * @brief SD error callbacks
  * @param hsd: Pointer SD handle
  * @retval None
  */
__weak void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SD_ErrorCallback can be implemented in the user file
   */
}

/**
  * @brief SD Abort callbacks
  * @param hsd: Pointer SD handle
  * @retval None
  */
__weak void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsd);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SD_AbortCallback can be implemented in the user file
   */
}

#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
/**
  * @brief  Register a User SD Callback
  *         To be used instead of the weak (surcharged) predefined callback
  * @param hsd : SD handle
  * @param CallbackID : ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_SD_TX_CPLT_CB_ID                 SD Tx Complete Callback ID
  *          @arg @ref HAL_SD_RX_CPLT_CB_ID                 SD Rx Complete Callback ID
  *          @arg @ref HAL_SD_ERROR_CB_ID                   SD Error Callback ID
  *          @arg @ref HAL_SD_ABORT_CB_ID                   SD Abort Callback ID
  *          @arg @ref HAL_SD_READ_DMA_DBL_BUF0_CPLT_CB_ID  SD DMA Rx Double buffer 0 Callback ID
  *          @arg @ref HAL_SD_READ_DMA_DBL_BUF1_CPLT_CB_ID  SD DMA Rx Double buffer 1 Callback ID
  *          @arg @ref HAL_SD_WRITE_DMA_DBL_BUF0_CPLT_CB_ID SD DMA Tx Double buffer 0 Callback ID
  *          @arg @ref HAL_SD_WRITE_DMA_DBL_BUF1_CPLT_CB_ID SD DMA Tx Double buffer 1 Callback ID
  *          @arg @ref HAL_SD_MSP_INIT_CB_ID                SD MspInit Callback ID
  *          @arg @ref HAL_SD_MSP_DEINIT_CB_ID              SD MspDeInit Callback ID
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_SD_RegisterCallback(SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackID, pSD_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    /* Update the error code */
    hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hsd);

  if(hsd->State == HAL_SD_STATE_READY)
  {
    switch (CallbackID)
    {
    case HAL_SD_TX_CPLT_CB_ID :
      hsd->TxCpltCallback = pCallback;
      break;
    case HAL_SD_RX_CPLT_CB_ID :
      hsd->RxCpltCallback = pCallback;
      break;
    case HAL_SD_ERROR_CB_ID :
      hsd->ErrorCallback = pCallback;
      break;
    case HAL_SD_ABORT_CB_ID :
      hsd->AbortCpltCallback = pCallback;
      break;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    case HAL_SD_READ_DMA_DBL_BUF0_CPLT_CB_ID :
      hsd->Read_DMADblBuf0CpltCallback = pCallback;
      break;
    case HAL_SD_READ_DMA_DBL_BUF1_CPLT_CB_ID :
      hsd->Read_DMADblBuf1CpltCallback = pCallback;
      break;
    case HAL_SD_WRITE_DMA_DBL_BUF0_CPLT_CB_ID :
      hsd->Write_DMADblBuf0CpltCallback = pCallback;
      break;
    case HAL_SD_WRITE_DMA_DBL_BUF1_CPLT_CB_ID :
      hsd->Write_DMADblBuf1CpltCallback = pCallback;
      break;
#endif
    case HAL_SD_MSP_INIT_CB_ID :
      hsd->MspInitCallback = pCallback;
      break;
    case HAL_SD_MSP_DEINIT_CB_ID :
      hsd->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hsd->State == HAL_SD_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_SD_MSP_INIT_CB_ID :
      hsd->MspInitCallback = pCallback;
      break;
    case HAL_SD_MSP_DEINIT_CB_ID :
      hsd->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsd);
  return status;
}

/**
  * @brief  Unregister a User SD Callback
  *         SD Callback is redirected to the weak (surcharged) predefined callback
  * @param hsd : SD handle
  * @param CallbackID : ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_SD_TX_CPLT_CB_ID                 SD Tx Complete Callback ID
  *          @arg @ref HAL_SD_RX_CPLT_CB_ID                 SD Rx Complete Callback ID
  *          @arg @ref HAL_SD_ERROR_CB_ID                   SD Error Callback ID
  *          @arg @ref HAL_SD_ABORT_CB_ID                   SD Abort Callback ID
  *          @arg @ref HAL_SD_READ_DMA_DBL_BUF0_CPLT_CB_ID  SD DMA Rx Double buffer 0 Callback ID
  *          @arg @ref HAL_SD_READ_DMA_DBL_BUF1_CPLT_CB_ID  SD DMA Rx Double buffer 1 Callback ID
  *          @arg @ref HAL_SD_WRITE_DMA_DBL_BUF0_CPLT_CB_ID SD DMA Tx Double buffer 0 Callback ID
  *          @arg @ref HAL_SD_WRITE_DMA_DBL_BUF1_CPLT_CB_ID SD DMA Tx Double buffer 1 Callback ID
  *          @arg @ref HAL_SD_MSP_INIT_CB_ID                SD MspInit Callback ID
  *          @arg @ref HAL_SD_MSP_DEINIT_CB_ID              SD MspDeInit Callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_SD_UnRegisterCallback(SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hsd);

  if(hsd->State == HAL_SD_STATE_READY)
  {
    switch (CallbackID)
    {
    case HAL_SD_TX_CPLT_CB_ID :
      hsd->TxCpltCallback = HAL_SD_TxCpltCallback;
      break;
    case HAL_SD_RX_CPLT_CB_ID :
      hsd->RxCpltCallback = HAL_SD_RxCpltCallback;
      break;
    case HAL_SD_ERROR_CB_ID :
      hsd->ErrorCallback = HAL_SD_ErrorCallback;
      break;
    case HAL_SD_ABORT_CB_ID :
      hsd->AbortCpltCallback = HAL_SD_AbortCallback;
      break;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    case HAL_SD_READ_DMA_DBL_BUF0_CPLT_CB_ID :
      hsd->Read_DMADblBuf0CpltCallback = HAL_SDEx_Read_DMADoubleBuffer0CpltCallback;
      break;
    case HAL_SD_READ_DMA_DBL_BUF1_CPLT_CB_ID :
      hsd->Read_DMADblBuf1CpltCallback = HAL_SDEx_Read_DMADoubleBuffer1CpltCallback;
      break;
    case HAL_SD_WRITE_DMA_DBL_BUF0_CPLT_CB_ID :
      hsd->Write_DMADblBuf0CpltCallback = HAL_SDEx_Write_DMADoubleBuffer0CpltCallback;
      break;
    case HAL_SD_WRITE_DMA_DBL_BUF1_CPLT_CB_ID :
      hsd->Write_DMADblBuf1CpltCallback = HAL_SDEx_Write_DMADoubleBuffer1CpltCallback;
      break;
#endif
    case HAL_SD_MSP_INIT_CB_ID :
      hsd->MspInitCallback = HAL_SD_MspInit;
      break;
    case HAL_SD_MSP_DEINIT_CB_ID :
      hsd->MspDeInitCallback = HAL_SD_MspDeInit;
      break;
    default :
      /* Update the error code */
      hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hsd->State == HAL_SD_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_SD_MSP_INIT_CB_ID :
      hsd->MspInitCallback = HAL_SD_MspInit;
      break;
    case HAL_SD_MSP_DEINIT_CB_ID :
      hsd->MspDeInitCallback = HAL_SD_MspDeInit;
      break;
    default :
      /* Update the error code */
      hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsd);
  return status;
}

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/**
  * @brief  Register a User SD Transceiver Callback
  *         To be used instead of the weak (surcharged) predefined callback
  * @param hsd : SD handle
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_SD_RegisterTransceiverCallback(SD_HandleTypeDef *hsd, pSD_TransceiverCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    /* Update the error code */
    hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hsd);

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->DriveTransceiver_1_8V_Callback = pCallback;
  }
  else
  {
    /* Update the error code */
    hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsd);
  return status;
}

/**
  * @brief  Unregister a User SD Transceiver Callback
  *         SD Callback is redirected to the weak (surcharged) predefined callback
  * @param hsd : SD handle
  * @retval status
  */
HAL_StatusTypeDef HAL_SD_UnRegisterTransceiverCallback(SD_HandleTypeDef *hsd)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hsd);

  if(hsd->State == HAL_SD_STATE_READY)
  {
    hsd->DriveTransceiver_1_8V_Callback = HAL_SDEx_DriveTransceiver_1_8V_Callback;
  }
  else
  {
    /* Update the error code */
    hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hsd);
  return status;
}
#endif
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @addtogroup SD_Exported_Functions_Group3
 *  @brief   management functions
 *
@verbatim
  ==============================================================================
                      ##### Peripheral Control functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to control the SD card
    operations and get the related information

@endverbatim
  * @{
  */

/**
  * @brief  Returns information the information of the card which are stored on
  *         the CID register.
  * @param  hsd: Pointer to SD handle
  * @param  pCID: Pointer to a HAL_SD_CardCIDTypeDef structure that  
  *         contains all CID register parameters
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_GetCardCID(SD_HandleTypeDef *hsd, HAL_SD_CardCIDTypeDef *pCID)
{
  pCID->ManufacturerID = (uint8_t)((hsd->CID[0] & 0xFF000000U) >> 24U);

  pCID->OEM_AppliID = (uint16_t)((hsd->CID[0] & 0x00FFFF00U) >> 8U);

  pCID->ProdName1 = (((hsd->CID[0] & 0x000000FFU) << 24U) | ((hsd->CID[1] & 0xFFFFFF00U) >> 8U));

  pCID->ProdName2 = (uint8_t)(hsd->CID[1] & 0x000000FFU);

  pCID->ProdRev = (uint8_t)((hsd->CID[2] & 0xFF000000U) >> 24U);

  pCID->ProdSN = (((hsd->CID[2] & 0x00FFFFFFU) << 8U) | ((hsd->CID[3] & 0xFF000000U) >> 24U));

  pCID->Reserved1 = (uint8_t)((hsd->CID[3] & 0x00F00000U) >> 20U);

  pCID->ManufactDate = (uint16_t)((hsd->CID[3] & 0x000FFF00U) >> 8U);

  pCID->CID_CRC = (uint8_t)((hsd->CID[3] & 0x000000FEU) >> 1U);

  pCID->Reserved2 = 1U;

  return HAL_OK;
}

/**
  * @brief  Returns information the information of the card which are stored on
  *         the CSD register.
  * @param  hsd: Pointer to SD handle
  * @param  pCSD: Pointer to a HAL_SD_CardCSDTypeDef structure that  
  *         contains all CSD register parameters
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_GetCardCSD(SD_HandleTypeDef *hsd, HAL_SD_CardCSDTypeDef *pCSD)
{
  pCSD->CSDStruct = (uint8_t)((hsd->CSD[0] & 0xC0000000U) >> 30U);

  pCSD->SysSpecVersion = (uint8_t)((hsd->CSD[0] & 0x3C000000U) >> 26U);

  pCSD->Reserved1 = (uint8_t)((hsd->CSD[0] & 0x03000000U) >> 24U);

  pCSD->TAAC = (uint8_t)((hsd->CSD[0] & 0x00FF0000U) >> 16U);

  pCSD->NSAC = (uint8_t)((hsd->CSD[0] & 0x0000FF00U) >> 8U);

  pCSD->MaxBusClkFrec = (uint8_t)(hsd->CSD[0] & 0x000000FFU);

  pCSD->CardComdClasses = (uint16_t)((hsd->CSD[1] & 0xFFF00000U) >> 20U);

  pCSD->RdBlockLen = (uint8_t)((hsd->CSD[1] & 0x000F0000U) >> 16U);

  pCSD->PartBlockRead   = (uint8_t)((hsd->CSD[1] & 0x00008000U) >> 15U);

  pCSD->WrBlockMisalign = (uint8_t)((hsd->CSD[1] & 0x00004000U) >> 14U);

  pCSD->RdBlockMisalign = (uint8_t)((hsd->CSD[1] & 0x00002000U) >> 13U);

  pCSD->DSRImpl = (uint8_t)((hsd->CSD[1] & 0x00001000U) >> 12U);

  pCSD->Reserved2 = 0U; /*!< Reserved */

  if(hsd->SdCard.CardType == CARD_SDSC)
  {
    pCSD->DeviceSize = (((hsd->CSD[1] & 0x000003FFU) << 2U) | ((hsd->CSD[2] & 0xC0000000U) >> 30U));

    pCSD->MaxRdCurrentVDDMin = (uint8_t)((hsd->CSD[2] & 0x38000000U) >> 27U);

    pCSD->MaxRdCurrentVDDMax = (uint8_t)((hsd->CSD[2] & 0x07000000U) >> 24U);

    pCSD->MaxWrCurrentVDDMin = (uint8_t)((hsd->CSD[2] & 0x00E00000U) >> 21U);

    pCSD->MaxWrCurrentVDDMax = (uint8_t)((hsd->CSD[2] & 0x001C0000U) >> 18U);

    pCSD->DeviceSizeMul = (uint8_t)((hsd->CSD[2] & 0x00038000U) >> 15U);

    hsd->SdCard.BlockNbr  = (pCSD->DeviceSize + 1U) ;
    hsd->SdCard.BlockNbr *= (1UL << ((pCSD->DeviceSizeMul & 0x07U) + 2U));
    hsd->SdCard.BlockSize = (1UL << (pCSD->RdBlockLen & 0x0FU));

    hsd->SdCard.LogBlockNbr =  (hsd->SdCard.BlockNbr) * ((hsd->SdCard.BlockSize) / 512U);
    hsd->SdCard.LogBlockSize = 512U;
  }
  else if(hsd->SdCard.CardType == CARD_SDHC_SDXC)
  {
    /* Byte 7 */
    pCSD->DeviceSize = (((hsd->CSD[1] & 0x0000003FU) << 16U) | ((hsd->CSD[2] & 0xFFFF0000U) >> 16U));

    hsd->SdCard.BlockNbr = ((pCSD->DeviceSize + 1U) * 1024U);
    hsd->SdCard.LogBlockNbr = hsd->SdCard.BlockNbr;
    hsd->SdCard.BlockSize = 512U;
    hsd->SdCard.LogBlockSize = hsd->SdCard.BlockSize;
  }
  else
  {
    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
    hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    hsd->State = HAL_SD_STATE_READY;
    return HAL_ERROR;
  }

  pCSD->EraseGrSize = (uint8_t)((hsd->CSD[2] & 0x00004000U) >> 14U);

  pCSD->EraseGrMul = (uint8_t)((hsd->CSD[2] & 0x00003F80U) >> 7U);

  pCSD->WrProtectGrSize = (uint8_t)(hsd->CSD[2] & 0x0000007FU);

  pCSD->WrProtectGrEnable = (uint8_t)((hsd->CSD[3] & 0x80000000U) >> 31U);

  pCSD->ManDeflECC = (uint8_t)((hsd->CSD[3] & 0x60000000U) >> 29U);

  pCSD->WrSpeedFact = (uint8_t)((hsd->CSD[3] & 0x1C000000U) >> 26U);

  pCSD->MaxWrBlockLen= (uint8_t)((hsd->CSD[3] & 0x03C00000U) >> 22U);

  pCSD->WriteBlockPaPartial = (uint8_t)((hsd->CSD[3] & 0x00200000U) >> 21U);

  pCSD->Reserved3 = 0;

  pCSD->ContentProtectAppli = (uint8_t)((hsd->CSD[3] & 0x00010000U) >> 16U);

  pCSD->FileFormatGroup = (uint8_t)((hsd->CSD[3] & 0x00008000U) >> 15U);

  pCSD->CopyFlag = (uint8_t)((hsd->CSD[3] & 0x00004000U) >> 14U);

  pCSD->PermWrProtect = (uint8_t)((hsd->CSD[3] & 0x00002000U) >> 13U);

  pCSD->TempWrProtect = (uint8_t)((hsd->CSD[3] & 0x00001000U) >> 12U);

  pCSD->FileFormat = (uint8_t)((hsd->CSD[3] & 0x00000C00U) >> 10U);

  pCSD->ECC= (uint8_t)((hsd->CSD[3] & 0x00000300U) >> 8U);

  pCSD->CSD_CRC = (uint8_t)((hsd->CSD[3] & 0x000000FEU) >> 1U);

  pCSD->Reserved4 = 1;

  return HAL_OK;
}

/**
  * @brief  Gets the SD status info.
  * @param  hsd: Pointer to SD handle
  * @param  pStatus: Pointer to the HAL_SD_CardStatusTypeDef structure that 
  *         will contain the SD card status information
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_GetCardStatus(SD_HandleTypeDef *hsd, HAL_SD_CardStatusTypeDef *pStatus)
{
  uint32_t sd_status[16];
  uint32_t errorstate;

  errorstate = SD_SendSDStatus(hsd, sd_status);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
    hsd->ErrorCode |= errorstate;
    hsd->State = HAL_SD_STATE_READY;
    return HAL_ERROR;
  }
  else
  {
    pStatus->DataBusWidth = (uint8_t)((sd_status[0] & 0xC0U) >> 6U);

    pStatus->SecuredMode = (uint8_t)((sd_status[0] & 0x20U) >> 5U);

    pStatus->CardType = (uint16_t)(((sd_status[0] & 0x00FF0000U) >> 8U) | ((sd_status[0] & 0xFF000000U) >> 24U));

    pStatus->ProtectedAreaSize = (((sd_status[1] & 0xFFU) << 24U)    | ((sd_status[1] & 0xFF00U) << 8U) |
                                  ((sd_status[1] & 0xFF0000U) >> 8U) | ((sd_status[1] & 0xFF000000U) >> 24U));

    pStatus->SpeedClass = (uint8_t)(sd_status[2] & 0xFFU);

    pStatus->PerformanceMove = (uint8_t)((sd_status[2] & 0xFF00U) >> 8U);

    pStatus->AllocationUnitSize = (uint8_t)((sd_status[2] & 0xF00000U) >> 20U);

    pStatus->EraseSize = (uint16_t)(((sd_status[2] & 0xFF000000U) >> 16U) | (sd_status[3] & 0xFFU));

    pStatus->EraseTimeout = (uint8_t)((sd_status[3] & 0xFC00U) >> 10U);

    pStatus->EraseOffset = (uint8_t)((sd_status[3] & 0x0300U) >> 8U);

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    pStatus->UhsSpeedGrade = (uint8_t)((sd_status[3] & 0x00F0U) >> 4U);
    pStatus->UhsAllocationUnitSize = (uint8_t)(sd_status[3] & 0x000FU) ;
    pStatus->VideoSpeedClass = (uint8_t)((sd_status[4] & 0xFF000000U) >> 24U);
#endif
  }

  return HAL_OK;
}

/**
  * @brief  Gets the SD card info.
  * @param  hsd: Pointer to SD handle
  * @param  pCardInfo: Pointer to the HAL_SD_CardInfoTypeDef structure that
  *         will contain the SD card status information
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_GetCardInfo(SD_HandleTypeDef *hsd, HAL_SD_CardInfoTypeDef *pCardInfo)
{
  pCardInfo->CardType     = (uint32_t)(hsd->SdCard.CardType);
  pCardInfo->CardVersion  = (uint32_t)(hsd->SdCard.CardVersion);
  pCardInfo->Class        = (uint32_t)(hsd->SdCard.Class);
  pCardInfo->RelCardAdd   = (uint32_t)(hsd->SdCard.RelCardAdd);
  pCardInfo->BlockNbr     = (uint32_t)(hsd->SdCard.BlockNbr);
  pCardInfo->BlockSize    = (uint32_t)(hsd->SdCard.BlockSize);
  pCardInfo->LogBlockNbr  = (uint32_t)(hsd->SdCard.LogBlockNbr);
  pCardInfo->LogBlockSize = (uint32_t)(hsd->SdCard.LogBlockSize);

  return HAL_OK;
}

/**
  * @brief  Enables wide bus operation for the requested card if supported by
  *         card.
  * @param  hsd: Pointer to SD handle
  * @param  WideMode: Specifies the SD card wide bus mode
  *          This parameter can be one of the following values:
  *            @arg SDMMC_BUS_WIDE_8B: 8-bit data transfer
  *            @arg SDMMC_BUS_WIDE_4B: 4-bit data transfer
  *            @arg SDMMC_BUS_WIDE_1B: 1-bit data transfer
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_ConfigWideBusOperation(SD_HandleTypeDef *hsd, uint32_t WideMode)
{
  SDMMC_InitTypeDef Init;
  uint32_t errorstate;

  /* Check the parameters */
  assert_param(IS_SDMMC_BUS_WIDE(WideMode));

  /* Change State */
  hsd->State = HAL_SD_STATE_BUSY;

  if(hsd->SdCard.CardType != CARD_SECURED)
  {
    if(WideMode == SDMMC_BUS_WIDE_8B)
    {
      hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    }
    else if(WideMode == SDMMC_BUS_WIDE_4B)
    {
      errorstate = SD_WideBus_Enable(hsd);

      hsd->ErrorCode |= errorstate;
    }
    else if(WideMode == SDMMC_BUS_WIDE_1B)
    {
      errorstate = SD_WideBus_Disable(hsd);

      hsd->ErrorCode |= errorstate;
    }
    else
    {
      /* WideMode is not a valid argument*/
      hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    }
  }
  else
  {
    /* MMC Card does not support this feature */
    hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
  }

  if(hsd->ErrorCode != HAL_SD_ERROR_NONE)
  {
    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);
    hsd->State = HAL_SD_STATE_READY;
    return HAL_ERROR;
  }
  else
  {
    /* Configure the SDMMC peripheral */
    Init.ClockEdge           = hsd->Init.ClockEdge;
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
    Init.ClockBypass         = hsd->Init.ClockBypass;
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */
    Init.ClockPowerSave      = hsd->Init.ClockPowerSave;
    Init.BusWide             = WideMode;
    Init.HardwareFlowControl = hsd->Init.HardwareFlowControl;

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    /* Check if user Clock div < Normal speed 25Mhz, no change in Clockdiv */
    if(hsd->Init.ClockDiv >= SDMMC_NSpeed_CLK_DIV)
    {
      Init.ClockDiv = hsd->Init.ClockDiv;
    }
    else if (hsd->SdCard.CardSpeed == CARD_ULTRA_HIGH_SPEED)
    {
      /* UltraHigh speed SD card,user Clock div */
      Init.ClockDiv = hsd->Init.ClockDiv;
    }
    else if (hsd->SdCard.CardSpeed == CARD_HIGH_SPEED)
    {
      /* High speed SD card, Max Frequency = 50Mhz */
      Init.ClockDiv = SDMMC_HSpeed_CLK_DIV;
    }
    else
    {
      /* No High speed SD card, Max Frequency = 25Mhz */
      Init.ClockDiv = SDMMC_NSpeed_CLK_DIV;
    }
#else
    Init.ClockDiv            = hsd->Init.ClockDiv;
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

    (void)SDMMC_Init(hsd->Instance, Init);
  }

  /* Change State */
  hsd->State = HAL_SD_STATE_READY;

  return HAL_OK;
}

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/**
  * @brief  Configure the speed bus mode
  * @param  hsd: Pointer to the SD handle
  * @param  SpeedMode: Specifies the SD card speed bus mode
  *          This parameter can be one of the following values:
  *            @arg SDMMC_SPEED_MODE_AUTO: Max speed mode supported by the card
  *            @arg SDMMC_SPEED_MODE_DEFAULT: Default Speed/SDR12 mode
  *            @arg SDMMC_SPEED_MODE_HIGH: High Speed/SDR25 mode
  *            @arg SDMMC_SPEED_MODE_ULTRA: Ultra high speed mode
  * @retval HAL status
  */

HAL_StatusTypeDef HAL_SD_ConfigSpeedBusOperation(SD_HandleTypeDef *hsd, uint32_t SpeedMode)
{
  uint32_t tickstart;
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the parameters */
  assert_param(IS_SDMMC_SPEED_MODE(SpeedMode));
  /* Change State */
  hsd->State = HAL_SD_STATE_BUSY;

  if(hsd->Init.Transceiver == SDMMC_TRANSCEIVER_ENABLE)
  {
  switch (SpeedMode)
  {
    case SDMMC_SPEED_MODE_AUTO:
    {
      if ((hsd->SdCard.CardSpeed  == CARD_ULTRA_HIGH_SPEED) ||
          (hsd->SdCard.CardType == CARD_SDHC_SDXC))
      {
        hsd->Instance->CLKCR |= 0x00100000U;
        /* Enable Ultra High Speed */
        if (SD_UltraHighSpeed(hsd) != HAL_SD_ERROR_NONE)
        {
          if (SD_HighSpeed(hsd) != HAL_SD_ERROR_NONE)
          {
            hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
            status = HAL_ERROR;
          }
        }
      }
      else if (hsd->SdCard.CardSpeed  == CARD_HIGH_SPEED)
      {
        /* Enable High Speed */
        if (SD_HighSpeed(hsd) != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
          status = HAL_ERROR;
        }
      }
      else
      {
        /*Nothing to do, Use defaultSpeed */
      }
      break;
    }
    case SDMMC_SPEED_MODE_ULTRA:
    {
      if ((hsd->SdCard.CardSpeed  == CARD_ULTRA_HIGH_SPEED) ||
          (hsd->SdCard.CardType == CARD_SDHC_SDXC))
      {
        hsd->Instance->CLKCR |= 0x00100000U;
        /* Enable UltraHigh Speed */
        if (SD_UltraHighSpeed(hsd) != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
          status = HAL_ERROR;
        }
      }
      else
      {
        hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
        status = HAL_ERROR;
      }
      break;
    }
    case SDMMC_SPEED_MODE_DDR:
    {
      if ((hsd->SdCard.CardSpeed  == CARD_ULTRA_HIGH_SPEED) ||
          (hsd->SdCard.CardType == CARD_SDHC_SDXC))
      {
        hsd->Instance->CLKCR |= 0x00100000U;
        /* Enable DDR Mode*/
        if (SD_DDR_Mode(hsd) != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
          status = HAL_ERROR;
        }
      }
      else
      {
        hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
        status = HAL_ERROR;
      }
      break;
    }
  case SDMMC_SPEED_MODE_HIGH:
    {
      if ((hsd->SdCard.CardSpeed  == CARD_ULTRA_HIGH_SPEED) ||
          (hsd->SdCard.CardSpeed  == CARD_HIGH_SPEED) ||
          (hsd->SdCard.CardType == CARD_SDHC_SDXC))
      {
        /* Enable High Speed */
        if (SD_HighSpeed(hsd) != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
          status = HAL_ERROR;
        }
      }
      else
      {
        hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
        status = HAL_ERROR;
      }
      break;
    }
    case SDMMC_SPEED_MODE_DEFAULT:
      break;
    default:
      hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
      status = HAL_ERROR;
      break;
  }
  }
  else
  {
  switch (SpeedMode)
  {
  case SDMMC_SPEED_MODE_AUTO:
    {
      if ((hsd->SdCard.CardSpeed  == CARD_ULTRA_HIGH_SPEED) ||
          (hsd->SdCard.CardSpeed  == CARD_HIGH_SPEED) ||
            (hsd->SdCard.CardType == CARD_SDHC_SDXC))
      {
        /* Enable High Speed */
        if (SD_HighSpeed(hsd) != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
          status = HAL_ERROR;
        }
      }
      else
      {
        /*Nothing to do, Use defaultSpeed */
      }
      break;
    }
  case SDMMC_SPEED_MODE_HIGH:
    {
      if ((hsd->SdCard.CardSpeed  == CARD_ULTRA_HIGH_SPEED) ||
          (hsd->SdCard.CardSpeed  == CARD_HIGH_SPEED) ||
            (hsd->SdCard.CardType == CARD_SDHC_SDXC))
      {
        /* Enable High Speed */
        if (SD_HighSpeed(hsd) != HAL_SD_ERROR_NONE)
        {
          hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
          status = HAL_ERROR;
        }
      }
      else
      {
        hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
        status = HAL_ERROR;
      }
      break;
    }
  case SDMMC_SPEED_MODE_DEFAULT:
    break;
  case SDMMC_SPEED_MODE_ULTRA: /*not valid without transceiver*/
  default:
    hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
    status = HAL_ERROR;
    break;
  }
  }


  /* Verify that SD card is ready to use after Speed mode switch*/
  tickstart = HAL_GetTick();
  while ((HAL_SD_GetCardState(hsd) != HAL_SD_CARD_TRANSFER))
  {
    if ((HAL_GetTick() - tickstart) >=  SDMMC_DATATIMEOUT)
    {
      hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
      hsd->State = HAL_SD_STATE_READY;
      return HAL_TIMEOUT;
    }
  }

  /* Change State */
  hsd->State = HAL_SD_STATE_READY;
  return status;
}
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

/**
  * @brief  Gets the current sd card data state.
  * @param  hsd: pointer to SD handle
  * @retval Card state
  */
HAL_SD_CardStateTypeDef HAL_SD_GetCardState(SD_HandleTypeDef *hsd)
{
  uint32_t cardstate;
  uint32_t errorstate;
  uint32_t resp1 = 0;

  errorstate = SD_SendStatus(hsd, &resp1);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    hsd->ErrorCode |= errorstate;
  }

  cardstate = ((resp1 >> 9U) & 0x0FU);

  return (HAL_SD_CardStateTypeDef)cardstate;
}

/**
  * @brief  Abort the current transfer and disable the SD.
  * @param  hsd: pointer to a SD_HandleTypeDef structure that contains
  *                the configuration information for SD module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_Abort(SD_HandleTypeDef *hsd)
{
  HAL_SD_CardStateTypeDef CardState;
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  uint32_t context = hsd->Context;
#endif

  /* DIsable All interrupts */
  __HAL_SD_DISABLE_IT(hsd, SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT|\
                           SDMMC_IT_TXUNDERR| SDMMC_IT_RXOVERR);

  /* Clear All flags */
  __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  /* If IDMA Context, disable Internal DMA */
  hsd->Instance->IDMACTRL = SDMMC_DISABLE_IDMA;
#else
  CLEAR_BIT(hsd->Instance->DCTRL, SDMMC_DCTRL_DTEN);

  if ((context & SD_CONTEXT_DMA) != 0U)
  {
    /* Disable the SD DMA request */
    hsd->Instance->DCTRL &= (uint32_t)~((uint32_t)SDMMC_DCTRL_DMAEN);

    /* Abort the SD DMA Tx channel */
    if (((context & SD_CONTEXT_WRITE_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U))
    {
      if(HAL_DMA_Abort(hsd->hdmatx) != HAL_OK)
      {
        hsd->ErrorCode |= HAL_SD_ERROR_DMA;
      }
    }
    /* Abort the SD DMA Rx channel */
    else if (((context & SD_CONTEXT_READ_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) != 0U))
    {
      if(HAL_DMA_Abort(hsd->hdmarx) != HAL_OK)
      {
        hsd->ErrorCode |= HAL_SD_ERROR_DMA;
      }
    }
    else
    {
      /* Nothing to do */
    }
  }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

  hsd->State = HAL_SD_STATE_READY;

  /* Initialize the SD operation */
  hsd->Context = SD_CONTEXT_NONE;

  CardState = HAL_SD_GetCardState(hsd);
  if((CardState == HAL_SD_CARD_RECEIVING) || (CardState == HAL_SD_CARD_SENDING))
  {
    hsd->ErrorCode = SDMMC_CmdStopTransfer(hsd->Instance);
  }
  if(hsd->ErrorCode != HAL_SD_ERROR_NONE)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}

/**
  * @brief  Abort the current transfer and disable the SD (IT mode).
  * @param  hsd: pointer to a SD_HandleTypeDef structure that contains
  *                the configuration information for SD module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SD_Abort_IT(SD_HandleTypeDef *hsd)
{
  HAL_SD_CardStateTypeDef CardState;
#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  uint32_t context = hsd->Context;
#endif

  /* Disable All interrupts */
  __HAL_SD_DISABLE_IT(hsd, SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT|\
                           SDMMC_IT_TXUNDERR| SDMMC_IT_RXOVERR);

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  /* If IDMA Context, disable Internal DMA */
  hsd->Instance->IDMACTRL = SDMMC_DISABLE_IDMA;

  /* Clear All flags */
  __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

  CardState = HAL_SD_GetCardState(hsd);
  hsd->State = HAL_SD_STATE_READY;

  if((CardState == HAL_SD_CARD_RECEIVING) || (CardState == HAL_SD_CARD_SENDING))
  {
    hsd->ErrorCode = SDMMC_CmdStopTransfer(hsd->Instance);
  }

  if(hsd->ErrorCode != HAL_SD_ERROR_NONE)
  {
    return HAL_ERROR;
  }
  else
  {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
    hsd->AbortCpltCallback(hsd);
#else
    HAL_SD_AbortCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
  }
#else
  CLEAR_BIT(hsd->Instance->DCTRL, SDMMC_DCTRL_DTEN);

  if ((context & SD_CONTEXT_DMA) != 0U)
  {
    /* Disable the SD DMA request */
    hsd->Instance->DCTRL &= (uint32_t)~((uint32_t)SDMMC_DCTRL_DMAEN);

    /* Abort the SD DMA Tx channel */
    if (((context & SD_CONTEXT_WRITE_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != 0U))
    {
      hsd->hdmatx->XferAbortCallback = SD_DMATxAbort;
      if(HAL_DMA_Abort_IT(hsd->hdmatx) != HAL_OK)
      {
        hsd->hdmatx = NULL;
      }
    }
    /* Abort the SD DMA Rx channel */
    else if (((context & SD_CONTEXT_READ_SINGLE_BLOCK) != 0U) || ((context & SD_CONTEXT_READ_MULTIPLE_BLOCK) != 0U))
    {
      hsd->hdmarx->XferAbortCallback = SD_DMARxAbort;
      if(HAL_DMA_Abort_IT(hsd->hdmarx) != HAL_OK)
      {
        hsd->hdmarx = NULL;
      }
    }
    else
    {
      /* Nothing to do */
    }
  }
  /* No transfer ongoing on both DMA channels*/
  else
  {
    /* Clear All flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    CardState = HAL_SD_GetCardState(hsd);
    hsd->State = HAL_SD_STATE_READY;
    hsd->Context = SD_CONTEXT_NONE;
    if((CardState == HAL_SD_CARD_RECEIVING) || (CardState == HAL_SD_CARD_SENDING))
    {
      hsd->ErrorCode = SDMMC_CmdStopTransfer(hsd->Instance);
    }
    if(hsd->ErrorCode != HAL_SD_ERROR_NONE)
    {
      return HAL_ERROR;
    }
    else
    {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
      hsd->AbortCpltCallback(hsd);
#else
      HAL_SD_AbortCallback(hsd);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
    }
  }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

  return HAL_OK;
}

/**
  * @}
  */

/**
  * @}
  */

/* Private function ----------------------------------------------------------*/
/** @addtogroup SD_Private_Functions
  * @{
  */

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
/**
  * @brief  DMA SD transmit process complete callback
  * @param  hdma: DMA handle
  * @retval None
  */
static void SD_DMATransmitCplt(DMA_HandleTypeDef *hdma)
{
  SD_HandleTypeDef* hsd = (SD_HandleTypeDef* )(hdma->Parent);

  /* Enable DATAEND Interrupt */
  __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DATAEND));
}

/**
  * @brief  DMA SD receive process complete callback
  * @param  hdma: DMA handle
  * @retval None
  */
static void SD_DMAReceiveCplt(DMA_HandleTypeDef *hdma)
{
  SD_HandleTypeDef* hsd = (SD_HandleTypeDef* )(hdma->Parent);
  uint32_t errorstate;

  /* Send stop command in multiblock write */
  if(hsd->Context == (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_DMA))
  {
    errorstate = SDMMC_CmdStopTransfer(hsd->Instance);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      hsd->ErrorCode |= errorstate;
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
      hsd->ErrorCallback(hsd);
#else
      HAL_SD_ErrorCallback(hsd);
#endif
    }
  }

  /* Disable the DMA transfer for transmit request by setting the DMAEN bit
  in the SD DCTRL register */
  hsd->Instance->DCTRL &= (uint32_t)~((uint32_t)SDMMC_DCTRL_DMAEN);

  /* Clear all the static flags */
  __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

  hsd->State = HAL_SD_STATE_READY;
  hsd->Context = SD_CONTEXT_NONE;

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
  hsd->RxCpltCallback(hsd);
#else
  HAL_SD_RxCpltCallback(hsd);
#endif
}

/**
  * @brief  DMA SD communication error callback
  * @param  hdma: DMA handle
  * @retval None
  */
static void SD_DMAError(DMA_HandleTypeDef *hdma)
{
  SD_HandleTypeDef* hsd = (SD_HandleTypeDef* )(hdma->Parent);
  HAL_SD_CardStateTypeDef CardState;
  uint32_t RxErrorCode, TxErrorCode;

  RxErrorCode = hsd->hdmarx->ErrorCode;
  TxErrorCode = hsd->hdmatx->ErrorCode;
  if((RxErrorCode == HAL_DMA_ERROR_TE) || (TxErrorCode == HAL_DMA_ERROR_TE))
  {
    /* Clear All flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

    /* Disable All interrupts */
    __HAL_SD_DISABLE_IT(hsd, SDMMC_IT_DATAEND | SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT|\
      SDMMC_IT_TXUNDERR| SDMMC_IT_RXOVERR);

    hsd->ErrorCode |= HAL_SD_ERROR_DMA;
    CardState = HAL_SD_GetCardState(hsd);
    if((CardState == HAL_SD_CARD_RECEIVING) || (CardState == HAL_SD_CARD_SENDING))
    {
      hsd->ErrorCode |= SDMMC_CmdStopTransfer(hsd->Instance);
    }

    hsd->State= HAL_SD_STATE_READY;
    hsd->Context = SD_CONTEXT_NONE;
  }

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
  hsd->ErrorCallback(hsd);
#else
  HAL_SD_ErrorCallback(hsd);
#endif
}

/**
  * @brief  DMA SD Tx Abort callback
  * @param  hdma: DMA handle
  * @retval None
  */
static void SD_DMATxAbort(DMA_HandleTypeDef *hdma)
{
  SD_HandleTypeDef* hsd = (SD_HandleTypeDef* )(hdma->Parent);
  HAL_SD_CardStateTypeDef CardState;

  /* Clear All flags */
  __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

  CardState = HAL_SD_GetCardState(hsd);
  hsd->State = HAL_SD_STATE_READY;
  hsd->Context = SD_CONTEXT_NONE;
  if((CardState == HAL_SD_CARD_RECEIVING) || (CardState == HAL_SD_CARD_SENDING))
  {
    hsd->ErrorCode |= SDMMC_CmdStopTransfer(hsd->Instance);
  }

  if(hsd->ErrorCode == HAL_SD_ERROR_NONE)
  {
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
    hsd->AbortCpltCallback(hsd);
#else
    HAL_SD_AbortCallback(hsd);
#endif
  }
  else
  {
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
    hsd->ErrorCallback(hsd);
#else
    HAL_SD_ErrorCallback(hsd);
#endif
  }
}

/**
  * @brief  DMA SD Rx Abort callback
  * @param  hdma: DMA handle
  * @retval None
  */
static void SD_DMARxAbort(DMA_HandleTypeDef *hdma)
{
  SD_HandleTypeDef* hsd = (SD_HandleTypeDef* )(hdma->Parent);
  HAL_SD_CardStateTypeDef CardState;

  /* Clear All flags */
  __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

  CardState = HAL_SD_GetCardState(hsd);
  hsd->State = HAL_SD_STATE_READY;
  hsd->Context = SD_CONTEXT_NONE;
  if((CardState == HAL_SD_CARD_RECEIVING) || (CardState == HAL_SD_CARD_SENDING))
  {
    hsd->ErrorCode |= SDMMC_CmdStopTransfer(hsd->Instance);
  }

  if(hsd->ErrorCode == HAL_SD_ERROR_NONE)
  {
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
    hsd->AbortCpltCallback(hsd);
#else
    HAL_SD_AbortCallback(hsd);
#endif
  }
  else
  {
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
    hsd->ErrorCallback(hsd);
#else
    HAL_SD_ErrorCallback(hsd);
#endif
  }
}
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

/**
  * @brief  Initializes the sd card.
  * @param  hsd: Pointer to SD handle
  * @retval SD Card error state
  */
static uint32_t SD_InitCard(SD_HandleTypeDef *hsd)
{
  HAL_SD_CardCSDTypeDef CSD;
  uint32_t errorstate;
  uint16_t sd_rca = 1U;

  /* Check the power State */
  if(SDMMC_GetPowerState(hsd->Instance) == 0U)
  {
    /* Power off */
    return HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
  }

  if(hsd->SdCard.CardType != CARD_SECURED)
  {
    /* Send CMD2 ALL_SEND_CID */
    errorstate = SDMMC_CmdSendCID(hsd->Instance);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }
    else
    {
      /* Get Card identification number data */
      hsd->CID[0U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);
      hsd->CID[1U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP2);
      hsd->CID[2U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP3);
      hsd->CID[3U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP4);
    }
  }

  if(hsd->SdCard.CardType != CARD_SECURED)
  {
    /* Send CMD3 SET_REL_ADDR with argument 0 */
    /* SD Card publishes its RCA. */
    errorstate = SDMMC_CmdSetRelAdd(hsd->Instance, &sd_rca);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }
  }
  if(hsd->SdCard.CardType != CARD_SECURED)
  {
    /* Get the SD card RCA */
    hsd->SdCard.RelCardAdd = sd_rca;

    /* Send CMD9 SEND_CSD with argument as card's RCA */
    errorstate = SDMMC_CmdSendCSD(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }
    else
    {
      /* Get Card Specific Data */
      hsd->CSD[0U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);
      hsd->CSD[1U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP2);
      hsd->CSD[2U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP3);
      hsd->CSD[3U] = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP4);
    }
  }

  /* Get the Card Class */
  hsd->SdCard.Class = (SDMMC_GetResponse(hsd->Instance, SDMMC_RESP2) >> 20U);

  /* Get CSD parameters */
  if (HAL_SD_GetCardCSD(hsd, &CSD) != HAL_OK)
  {
    return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
  }

  /* Select the Card */
  errorstate = SDMMC_CmdSelDesel(hsd->Instance, (uint32_t)(((uint32_t)hsd->SdCard.RelCardAdd) << 16U));
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

#if !defined(STM32L4R5xx) && !defined(STM32L4R7xx) && !defined(STM32L4R9xx) && !defined(STM32L4S5xx) && !defined(STM32L4S7xx) && !defined(STM32L4S9xx)
  /* Configure SDMMC peripheral interface */
  (void)SDMMC_Init(hsd->Instance, hsd->Init);
#endif /* !STM32L4R5xx && !STM32L4R7xx && !STM32L4R9xx && !STM32L4S5xx && !STM32L4S7xx && !STM32L4S9xx */

  /* All cards are initialized */
  return HAL_SD_ERROR_NONE;
}

/**
  * @brief  Enquires cards about their operating voltage and configures clock
  *         controls and stores SD information that will be needed in future
  *         in the SD handle.
  * @param  hsd: Pointer to SD handle
  * @retval error state
  */
static uint32_t SD_PowerON(SD_HandleTypeDef *hsd)
{
  __IO uint32_t count = 0U;
  uint32_t response = 0U, validvoltage = 0U;
  uint32_t errorstate;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  uint32_t tickstart = HAL_GetTick();
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

  /* CMD0: GO_IDLE_STATE */
  errorstate = SDMMC_CmdGoIdleState(hsd->Instance);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

  /* CMD8: SEND_IF_COND: Command available only on V2.0 cards */
  errorstate = SDMMC_CmdOperCond(hsd->Instance);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    hsd->SdCard.CardVersion = CARD_V1_X;
    /* CMD0: GO_IDLE_STATE */
    errorstate = SDMMC_CmdGoIdleState(hsd->Instance);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

  }
  else
  {
    hsd->SdCard.CardVersion = CARD_V2_X;
  }

  if( hsd->SdCard.CardVersion == CARD_V2_X)
  {
    /* SEND CMD55 APP_CMD with RCA as 0 */
    errorstate = SDMMC_CmdAppCommand(hsd->Instance, 0);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    }
  }
  /* SD CARD */
  /* Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
  while((count < SDMMC_MAX_VOLT_TRIAL) && (validvoltage == 0U))
  {
    /* SEND CMD55 APP_CMD with RCA as 0 */
    errorstate = SDMMC_CmdAppCommand(hsd->Instance, 0);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    /* Send CMD41 */
    errorstate = SDMMC_CmdAppOperCommand(hsd->Instance, SDMMC_VOLTAGE_WINDOW_SD | SDMMC_HIGH_CAPACITY | SD_SWITCH_1_8V_CAPACITY);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    }

    /* Get command response */
    response = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);

    /* Get operating voltage*/
    validvoltage = (((response >> 31U) == 1U) ? 1U : 0U);

    count++;
  }

  if(count >= SDMMC_MAX_VOLT_TRIAL)
  {
    return HAL_SD_ERROR_INVALID_VOLTRANGE;
  }

  if((response & SDMMC_HIGH_CAPACITY) == SDMMC_HIGH_CAPACITY) /* (response &= SD_HIGH_CAPACITY) */
  {
    hsd->SdCard.CardType = CARD_SDHC_SDXC;
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
    if(hsd->Init.Transceiver == SDMMC_TRANSCEIVER_ENABLE)
    {
      if((response & SD_SWITCH_1_8V_CAPACITY) == SD_SWITCH_1_8V_CAPACITY)
      {
        hsd->SdCard.CardSpeed = CARD_ULTRA_HIGH_SPEED;

        /* Start switching procedue */
        hsd->Instance->POWER |= SDMMC_POWER_VSWITCHEN;

        /* Send CMD11 to switch 1.8V mode */
        errorstate = SDMMC_CmdVoltageSwitch(hsd->Instance);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
          return errorstate;
        }

        /* Check to CKSTOP */
        while(( hsd->Instance->STA & SDMMC_FLAG_CKSTOP) != SDMMC_FLAG_CKSTOP)
        {
          if((HAL_GetTick() - tickstart) >=  SDMMC_DATATIMEOUT)
          {
            return HAL_SD_ERROR_TIMEOUT;
          }
        }

        /* Clear CKSTOP Flag */
        hsd->Instance->ICR = SDMMC_FLAG_CKSTOP;

        /* Check to BusyD0 */
        if(( hsd->Instance->STA & SDMMC_FLAG_BUSYD0) != SDMMC_FLAG_BUSYD0)
        {
          /* Error when activate Voltage Switch in SDMMC Peripheral */
          return SDMMC_ERROR_UNSUPPORTED_FEATURE;
        }
        else
        {
          /* Enable Transceiver Switch PIN */
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
          hsd->DriveTransceiver_1_8V_Callback(SET);
#else
          HAL_SDEx_DriveTransceiver_1_8V_Callback(SET);
#endif

          /* Switch ready */
          hsd->Instance->POWER |= SDMMC_POWER_VSWITCH;

          /* Check VSWEND Flag */
          while(( hsd->Instance->STA & SDMMC_FLAG_VSWEND) != SDMMC_FLAG_VSWEND)
          {
            if((HAL_GetTick() - tickstart) >=  SDMMC_DATATIMEOUT)
            {
              return HAL_SD_ERROR_TIMEOUT;
            }
          }

          /* Clear VSWEND Flag */
          hsd->Instance->ICR = SDMMC_FLAG_VSWEND;

          /* Check BusyD0 status */
          if(( hsd->Instance->STA & SDMMC_FLAG_BUSYD0) == SDMMC_FLAG_BUSYD0)
          {
            /* Error when enabling 1.8V mode */
            return HAL_SD_ERROR_INVALID_VOLTRANGE;
          }
          /* Switch to 1.8V OK */

          /* Disable VSWITCH FLAG from SDMMC Peripheral */
          hsd->Instance->POWER = 0x13U;

          /* Clean Status flags */
          hsd->Instance->ICR = 0xFFFFFFFFU;
        }

        hsd->SdCard.CardSpeed = CARD_ULTRA_HIGH_SPEED;
      }
    }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
  }
  else
  {
    hsd->SdCard.CardType = CARD_SDSC;
  }


  return HAL_SD_ERROR_NONE;
}

/**
  * @brief  Turns the SDMMC output signals off.
  * @param  hsd: Pointer to SD handle
  * @retval None
  */
static void SD_PowerOFF(SD_HandleTypeDef *hsd)
{
  /* Set Power State to OFF */
  (void)SDMMC_PowerState_OFF(hsd->Instance);
}

/**
  * @brief  Send Status info command.
  * @param  hsd: pointer to SD handle
  * @param  pSDstatus: Pointer to the buffer that will contain the SD card status
  *         SD Status register)
  * @retval error state
  */
static uint32_t SD_SendSDStatus(SD_HandleTypeDef *hsd, uint32_t *pSDstatus)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t tickstart = HAL_GetTick();
  uint32_t count;
  uint32_t *pData = pSDstatus;

  /* Check SD response */
  if((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
  {
    return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
  }

  /* Set block size for card if it is not equal to current block size for card */
  errorstate = SDMMC_CmdBlockLength(hsd->Instance, 64U);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_NONE;
    return errorstate;
  }

  /* Send CMD55 */
  errorstate = SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_NONE;
    return errorstate;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  config.DataTimeOut   = SDMMC_DATATIMEOUT;
  config.DataLength    = 64U;
  config.DataBlockSize = SDMMC_DATABLOCK_SIZE_64B;
  config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
  config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
  config.DPSM          = SDMMC_DPSM_ENABLE;
  (void)SDMMC_ConfigData(hsd->Instance, &config);

  /* Send ACMD13 (SD_APP_STAUS)  with argument as card's RCA */
  errorstate = SDMMC_CmdStatusRegister(hsd->Instance);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    hsd->ErrorCode |= HAL_SD_ERROR_NONE;
    return errorstate;
  }

  /* Get status data */
#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
#else
  while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
  {
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
    {
      for(count = 0U; count < 8U; count++)
      {
        *pData = SDMMC_ReadFIFO(hsd->Instance);
        pData++;
      }
    }

    if((HAL_GetTick() - tickstart) >=  SDMMC_DATATIMEOUT)
    {
      return HAL_SD_ERROR_TIMEOUT;
    }
  }

  if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
  {
    return HAL_SD_ERROR_DATA_TIMEOUT;
  }
  else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
  {
    return HAL_SD_ERROR_DATA_CRC_FAIL;
  }
  else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
  {
    return HAL_SD_ERROR_RX_OVERRUN;
  }
  else
  {
    /* Nothing to do */
  }

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  while ((__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DPSMACT)))
#else
  while ((__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXDAVL)))
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
  {
    *pData = SDMMC_ReadFIFO(hsd->Instance);
    pData++;

    if((HAL_GetTick() - tickstart) >=  SDMMC_DATATIMEOUT)
    {
      return HAL_SD_ERROR_TIMEOUT;
    }
  }

  /* Clear all the static status flags*/
  __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

  return HAL_SD_ERROR_NONE;
}

/**
  * @brief  Returns the current card's status.
  * @param  hsd: Pointer to SD handle
  * @param  pCardStatus: pointer to the buffer that will contain the SD card
  *         status (Card Status register)
  * @retval error state
  */
static uint32_t SD_SendStatus(SD_HandleTypeDef *hsd, uint32_t *pCardStatus)
{
  uint32_t errorstate;

  if(pCardStatus == NULL)
  {
    return HAL_SD_ERROR_PARAM;
  }

  /* Send Status command */
  errorstate = SDMMC_CmdSendStatus(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

  /* Get SD card status */
  *pCardStatus = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);

  return HAL_SD_ERROR_NONE;
}

/**
  * @brief  Enables the SDMMC wide bus mode.
  * @param  hsd: pointer to SD handle
  * @retval error state
  */
static uint32_t SD_WideBus_Enable(SD_HandleTypeDef *hsd)
{
  uint32_t scr[2U] = {0U, 0U};
  uint32_t errorstate;

  if((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
  {
    return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
  }

  /* Get SCR Register */
  errorstate = SD_FindSCR(hsd, scr);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

  /* If requested card supports wide bus operation */
  if((scr[1U] & SDMMC_WIDE_BUS_SUPPORT) != SDMMC_ALLZERO)
  {
    /* Send CMD55 APP_CMD with argument as card's RCA.*/
    errorstate = SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    /* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
    errorstate = SDMMC_CmdBusWidth(hsd->Instance, 2U);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    return HAL_SD_ERROR_NONE;
  }
  else
  {
    return HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
  }
}

/**
  * @brief  Disables the SDMMC wide bus mode.
  * @param  hsd: Pointer to SD handle
  * @retval error state
  */
static uint32_t SD_WideBus_Disable(SD_HandleTypeDef *hsd)
{
  uint32_t scr[2U] = {0U, 0U};
  uint32_t errorstate;

  if((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
  {
    return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
  }

  /* Get SCR Register */
  errorstate = SD_FindSCR(hsd, scr);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

  /* If requested card supports 1 bit mode operation */
  if((scr[1U] & SDMMC_SINGLE_BUS_SUPPORT) != SDMMC_ALLZERO)
  {
    /* Send CMD55 APP_CMD with argument as card's RCA */
    errorstate = SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    /* Send ACMD6 APP_CMD with argument as 0 for single bus mode */
    errorstate = SDMMC_CmdBusWidth(hsd->Instance, 0U);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    return HAL_SD_ERROR_NONE;
  }
  else
  {
    return HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
  }
}


/**
  * @brief  Finds the SD card SCR register value.
  * @param  hsd: Pointer to SD handle
  * @param  pSCR: pointer to the buffer that will contain the SCR value
  * @retval error state
  */
static uint32_t SD_FindSCR(SD_HandleTypeDef *hsd, uint32_t *pSCR)
{
  SDMMC_DataInitTypeDef config;
  uint32_t errorstate;
  uint32_t tickstart = HAL_GetTick();
  uint32_t index = 0U;
  uint32_t tempscr[2U] = {0U, 0U};
  uint32_t *scr = pSCR;

  /* Set Block Size To 8 Bytes */
  errorstate = SDMMC_CmdBlockLength(hsd->Instance, 8U);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

  /* Send CMD55 APP_CMD with argument as card's RCA */
  errorstate = SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)((hsd->SdCard.RelCardAdd) << 16U));
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

  config.DataTimeOut   = SDMMC_DATATIMEOUT;
  config.DataLength    = 8U;
  config.DataBlockSize = SDMMC_DATABLOCK_SIZE_8B;
  config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
  config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
  config.DPSM          = SDMMC_DPSM_ENABLE;
  (void)SDMMC_ConfigData(hsd->Instance, &config);

  /* Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
  errorstate = SDMMC_CmdSendSCR(hsd->Instance);
  if(errorstate != HAL_SD_ERROR_NONE)
  {
    return errorstate;
  }

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
  while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND | SDMMC_FLAG_DATAEND))
  {
    if((!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOE)) && (index == 0U))
    {
      tempscr[0] = SDMMC_ReadFIFO(hsd->Instance);
      tempscr[1] = SDMMC_ReadFIFO(hsd->Instance);
      index++;
    }


    if((HAL_GetTick() - tickstart) >=  SDMMC_DATATIMEOUT)
    {
      return HAL_SD_ERROR_TIMEOUT;
    }
  }
#else
  while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
  {
    if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXDAVL))
    {
      *(tempscr + index) = SDMMC_ReadFIFO(hsd->Instance);
      index++;
    }

    if((HAL_GetTick() - tickstart) >=  SDMMC_DATATIMEOUT)
    {
      return HAL_SD_ERROR_TIMEOUT;
    }
  }
#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */

  if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
  {
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

    return HAL_SD_ERROR_DATA_TIMEOUT;
  }
  else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
  {
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

    return HAL_SD_ERROR_DATA_CRC_FAIL;
  }
  else if(__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
  {
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

    return HAL_SD_ERROR_RX_OVERRUN;
  }
  else
  {
    /* No error flag set */
    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    *scr = (((tempscr[1] & SDMMC_0TO7BITS) << 24)  | ((tempscr[1] & SDMMC_8TO15BITS) << 8) |\
            ((tempscr[1] & SDMMC_16TO23BITS) >> 8) | ((tempscr[1] & SDMMC_24TO31BITS) >> 24));
    scr++;
    *scr = (((tempscr[0] & SDMMC_0TO7BITS) << 24)  | ((tempscr[0] & SDMMC_8TO15BITS) << 8) |\
            ((tempscr[0] & SDMMC_16TO23BITS) >> 8) | ((tempscr[0] & SDMMC_24TO31BITS) >> 24));

  }

  return HAL_SD_ERROR_NONE;
}

/**
  * @brief  Wrap up reading in non-blocking mode.
  * @param  hsd: pointer to a SD_HandleTypeDef structure that contains
  *              the configuration information.
  * @retval None
  */
static void SD_Read_IT(SD_HandleTypeDef *hsd)
{
  uint32_t count, data, dataremaining;
  uint8_t* tmp;

  tmp = hsd->pRxBuffPtr;
  dataremaining = hsd->RxXferSize;

  if (dataremaining > 0U)
  {
    /* Read data from SDMMC Rx FIFO */
    for(count = 0U; count < 8U; count++)
    {
      data = SDMMC_ReadFIFO(hsd->Instance);
      *tmp = (uint8_t)(data & 0xFFU);
      tmp++;
      dataremaining--;
      *tmp = (uint8_t)((data >> 8U) & 0xFFU);
      tmp++;
      dataremaining--;
      *tmp = (uint8_t)((data >> 16U) & 0xFFU);
      tmp++;
      dataremaining--;
      *tmp = (uint8_t)((data >> 24U) & 0xFFU);
      tmp++;
      dataremaining--;
    }

    hsd->pRxBuffPtr = tmp;
    hsd->RxXferSize = dataremaining;
  }
}

/**
  * @brief  Wrap up writing in non-blocking mode.
  * @param  hsd: pointer to a SD_HandleTypeDef structure that contains
  *              the configuration information.
  * @retval None
  */
static void SD_Write_IT(SD_HandleTypeDef *hsd)
{
  uint32_t count, data, dataremaining;
  uint8_t* tmp;

  tmp = hsd->pTxBuffPtr;
  dataremaining = hsd->TxXferSize;

  if (dataremaining > 0U)
  {
    /* Write data to SDMMC Tx FIFO */
    for(count = 0U; count < 8U; count++)
    {
      data = (uint32_t)(*tmp);
      tmp++;
      dataremaining--;
      data |= ((uint32_t)(*tmp) << 8U);
      tmp++;
      dataremaining--;
      data |= ((uint32_t)(*tmp) << 16U);
      tmp++;
      dataremaining--;
      data |= ((uint32_t)(*tmp) << 24U);
      tmp++;
      dataremaining--;
      (void)SDMMC_WriteFIFO(hsd->Instance, &data);
    }

    hsd->pTxBuffPtr = tmp;
    hsd->TxXferSize = dataremaining;
  }
}

#if defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
/**
  * @brief  Switches the SD card to High Speed mode.
  *         This API must be used after "Transfer State"
  * @note   This operation should be followed by the configuration
  *         of PLL to have SDMMCCK clock between 50 and 120 MHz
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
uint32_t SD_HighSpeed(SD_HandleTypeDef *hsd)
{
  uint32_t errorstate = HAL_SD_ERROR_NONE;
  SDMMC_DataInitTypeDef sdmmc_datainitstructure;
  uint32_t SD_hs[16]  = {0};
  uint32_t count, loop = 0 ;
  uint32_t Timeout = HAL_GetTick();

  if(hsd->SdCard.CardSpeed == CARD_NORMAL_SPEED)
  {
     /* Standard Speed Card <= 12.5Mhz  */
     return HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
  }

  if(hsd->SdCard.CardSpeed == CARD_HIGH_SPEED)
  {
    /* Initialize the Data control register */
    hsd->Instance->DCTRL = 0;
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, 64);

    if (errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    sdmmc_datainitstructure.DataTimeOut   = SDMMC_DATATIMEOUT;
    sdmmc_datainitstructure.DataLength    = 64;
    sdmmc_datainitstructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_64B ;
    sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
    sdmmc_datainitstructure.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    sdmmc_datainitstructure.DPSM          = SDMMC_DPSM_ENABLE;

    if ( SDMMC_ConfigData(hsd->Instance, &sdmmc_datainitstructure) != HAL_OK)
    {
      return (HAL_SD_ERROR_GENERAL_UNKNOWN_ERR);
    }


    errorstate = SDMMC_CmdSwitch(hsd->Instance,SDMMC_SDR25_SWITCH_PATTERN);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND| SDMMC_FLAG_DATAEND ))
    {
      if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
      {
        for (count = 0U; count < 8U; count++)
        {
          SD_hs[(8U*loop)+count]  = SDMMC_ReadFIFO(hsd->Instance);
        }
        loop ++;
      }

      if((HAL_GetTick()-Timeout) >=  SDMMC_DATATIMEOUT)
      {
        hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
        hsd->State= HAL_SD_STATE_READY;
        return HAL_SD_ERROR_TIMEOUT;
      }
    }

    if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

      errorstate = 0;

      return errorstate;
    }
    else if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

      errorstate = SDMMC_ERROR_DATA_CRC_FAIL;

      return errorstate;
    }
    else if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

      errorstate = SDMMC_ERROR_RX_OVERRUN;

      return errorstate;
    }
    else
    {
      /* No error flag set */
    }

    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    /* Test if the switch mode HS is ok */
    if ((((uint8_t*)SD_hs)[13] & 2U) != 2U)
    {
      errorstate = SDMMC_ERROR_UNSUPPORTED_FEATURE;
    }

  }

  return errorstate;
}

/**
  * @brief  Switches the SD card to Ultra High Speed mode.
  *         This API must be used after "Transfer State"
  * @note   This operation should be followed by the configuration
  *         of PLL to have SDMMCCK clock between 50 and 120 MHz
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static uint32_t SD_UltraHighSpeed(SD_HandleTypeDef *hsd)
{
  uint32_t errorstate = HAL_SD_ERROR_NONE;
  SDMMC_DataInitTypeDef sdmmc_datainitstructure;
  uint32_t SD_hs[16]  = {0};
  uint32_t count, loop = 0 ;
  uint32_t Timeout = HAL_GetTick();

  if(hsd->SdCard.CardSpeed == CARD_NORMAL_SPEED)
  {
     /* Standard Speed Card <= 12.5Mhz  */
     return HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
  }

  if((hsd->SdCard.CardSpeed == CARD_ULTRA_HIGH_SPEED) &&
     (hsd->Init.Transceiver == SDMMC_TRANSCEIVER_ENABLE))
  {
    /* Initialize the Data control register */
    hsd->Instance->DCTRL = 0;
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, 64);

    if (errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    sdmmc_datainitstructure.DataTimeOut   = SDMMC_DATATIMEOUT;
    sdmmc_datainitstructure.DataLength    = 64;
    sdmmc_datainitstructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_64B ;
    sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
    sdmmc_datainitstructure.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    sdmmc_datainitstructure.DPSM          = SDMMC_DPSM_ENABLE;

    if ( SDMMC_ConfigData(hsd->Instance, &sdmmc_datainitstructure) != HAL_OK)
    {
      return (HAL_SD_ERROR_GENERAL_UNKNOWN_ERR);
    }

    errorstate = SDMMC_CmdSwitch(hsd->Instance, SDMMC_SDR104_SWITCH_PATTERN);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND| SDMMC_FLAG_DATAEND ))
    {
      if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
      {
        for (count = 0U; count < 8U; count++)
        {
          SD_hs[(8U*loop)+count]  = SDMMC_ReadFIFO(hsd->Instance);
        }
        loop ++;
      }

      if((HAL_GetTick()-Timeout) >=  SDMMC_DATATIMEOUT)
      {
        hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
        hsd->State= HAL_SD_STATE_READY;
        return HAL_SD_ERROR_TIMEOUT;
      }
    }

    if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

      errorstate = 0;

      return errorstate;
    }
    else if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

      errorstate = SDMMC_ERROR_DATA_CRC_FAIL;

      return errorstate;
    }
    else if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

      errorstate = SDMMC_ERROR_RX_OVERRUN;

      return errorstate;
    }
    else
    {
      /* No error flag set */
    }

    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    /* Test if the switch mode HS is ok */
    if ((((uint8_t*)SD_hs)[13] & 2U) != 2U)
    {
      errorstate = SDMMC_ERROR_UNSUPPORTED_FEATURE;
    }
    else
    {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
      hsd->DriveTransceiver_1_8V_Callback(SET);
#else
      HAL_SDEx_DriveTransceiver_1_8V_Callback(SET);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
#if defined (DLYB_SDMMC1) || defined (DLYB_SDMMC2)
      /* Enable DelayBlock Peripheral */
      /* SDMMC_FB_CLK tuned feedback clock selected as receive clock, for SDR104 */
      MODIFY_REG(hsd->Instance->CLKCR, SDMMC_CLKCR_SELCLKRX,SDMMC_CLKCR_SELCLKRX_1);
      if (DelayBlock_Enable(SD_GET_DLYB_INSTANCE(hsd->Instance)) != HAL_OK)
      {
        return (HAL_SD_ERROR_GENERAL_UNKNOWN_ERR);
      }
#endif /* (DLYB_SDMMC1) || (DLYB_SDMMC2) */
    }
  }

  return errorstate;
}

/**
  * @brief  Switches the SD card to Double Data Rate (DDR) mode.
  *         This API must be used after "Transfer State"
  * @note   This operation should be followed by the configuration
  *         of PLL to have SDMMCCK clock less than 50MHz
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static uint32_t SD_DDR_Mode(SD_HandleTypeDef *hsd)
{
  uint32_t errorstate = HAL_SD_ERROR_NONE;
  SDMMC_DataInitTypeDef sdmmc_datainitstructure;
  uint32_t SD_hs[16]  = {0};
  uint32_t count, loop = 0 ;
  uint32_t Timeout = HAL_GetTick();

  if(hsd->SdCard.CardSpeed == CARD_NORMAL_SPEED)
  {
     /* Standard Speed Card <= 12.5Mhz  */
     return HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
  }

  if((hsd->SdCard.CardSpeed == CARD_ULTRA_HIGH_SPEED) &&
     (hsd->Init.Transceiver == SDMMC_TRANSCEIVER_ENABLE))
  {
    /* Initialize the Data control register */
    hsd->Instance->DCTRL = 0;
    errorstate = SDMMC_CmdBlockLength(hsd->Instance, 64);

    if (errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    /* Configure the SD DPSM (Data Path State Machine) */
    sdmmc_datainitstructure.DataTimeOut   = SDMMC_DATATIMEOUT;
    sdmmc_datainitstructure.DataLength    = 64;
    sdmmc_datainitstructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_64B ;
    sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
    sdmmc_datainitstructure.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
    sdmmc_datainitstructure.DPSM          = SDMMC_DPSM_ENABLE;

    if ( SDMMC_ConfigData(hsd->Instance, &sdmmc_datainitstructure) != HAL_OK)
    {
      return (HAL_SD_ERROR_GENERAL_UNKNOWN_ERR);
    }

    errorstate = SDMMC_CmdSwitch(hsd->Instance, SDMMC_DDR50_SWITCH_PATTERN);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
      return errorstate;
    }

    while(!__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND| SDMMC_FLAG_DATAEND ))
    {
      if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
      {
        for (count = 0U; count < 8U; count++)
        {
          SD_hs[(8U*loop)+count]  = SDMMC_ReadFIFO(hsd->Instance);
        }
        loop ++;
      }

      if((HAL_GetTick()-Timeout) >=  SDMMC_DATATIMEOUT)
      {
        hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
        hsd->State= HAL_SD_STATE_READY;
        return HAL_SD_ERROR_TIMEOUT;
      }
    }

    if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

      errorstate = 0;

      return errorstate;
    }
    else if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

      errorstate = SDMMC_ERROR_DATA_CRC_FAIL;

      return errorstate;
    }
    else if (__HAL_SD_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
    {
      __HAL_SD_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

      errorstate = SDMMC_ERROR_RX_OVERRUN;

      return errorstate;
    }
    else
    {
      /* No error flag set */
    }

    /* Clear all the static flags */
    __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

    /* Test if the switch mode  is ok */
    if ((((uint8_t*)SD_hs)[13] & 2U) != 2U)
    {
      errorstate = SDMMC_ERROR_UNSUPPORTED_FEATURE;
    }
    else
    {
#if defined (USE_HAL_SD_REGISTER_CALLBACKS) && (USE_HAL_SD_REGISTER_CALLBACKS == 1U)
      hsd->DriveTransceiver_1_8V_Callback(SET);
#else
      HAL_SDEx_DriveTransceiver_1_8V_Callback(SET);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
#if defined (DLYB_SDMMC1) || defined (DLYB_SDMMC2)
      /* Enable DelayBlock Peripheral */
      /* SDMMC_FB_CLK tuned feedback clock selected as receive clock, for SDR104 */
      MODIFY_REG(hsd->Instance->CLKCR, SDMMC_CLKCR_SELCLKRX,SDMMC_CLKCR_SELCLKRX_1);
      if (DelayBlock_Enable(SD_GET_DLYB_INSTANCE(hsd->Instance)) != HAL_OK)
      {
        return (HAL_SD_ERROR_GENERAL_UNKNOWN_ERR);
      }
#endif /* (DLYB_SDMMC1) || (DLYB_SDMMC2) */
    }
  }

  return errorstate;
}

#endif /* STM32L4R5xx || STM32L4R7xx || STM32L4R9xx || STM32L4S5xx || STM32L4S7xx || STM32L4S9xx */
/**
  * @}
  */

#endif /* HAL_SD_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

#endif /* SDMMC1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
