/**
  ******************************************************************************
  * @file    stm32l4xx_hal_ospi.c
  * @author  MCD Application Team
  * @brief   OSPI HAL module driver.
             This file provides firmware functions to manage the following
             functionalities of the OctoSPI interface (OSPI).
              + Initialization and de-initialization functions
              + Hyperbus configuration
              + Indirect functional mode management
              + Memory-mapped functional mode management
              + Auto-polling functional mode management
              + Interrupts and flags management
              + DMA channel configuration for indirect functional mode
              + Errors management and abort functionality
              + IO manager configuration

  @verbatim
 ===============================================================================
                        ##### How to use this driver #####
 ===============================================================================
  [..]
    *** Initialization ***
    ======================
    [..]
      (#) As prerequisite, fill in the HAL_OSPI_MspInit() :
        (++) Enable OctoSPI and OctoSPIM clocks interface with __HAL_RCC_OSPIx_CLK_ENABLE().
        (++) Reset OctoSPI Peripheral with __HAL_RCC_OSPIx_FORCE_RESET() and __HAL_RCC_OSPIx_RELEASE_RESET().
        (++) Enable the clocks for the OctoSPI GPIOS with __HAL_RCC_GPIOx_CLK_ENABLE().
        (++) Configure these OctoSPI pins in alternate mode using HAL_GPIO_Init().
        (++) If interrupt or DMA mode is used, enable and configure OctoSPI global
            interrupt with HAL_NVIC_SetPriority() and HAL_NVIC_EnableIRQ().
        (++) If DMA mode is used, enable the clocks for the OctoSPI DMA channel
            with __HAL_RCC_DMAx_CLK_ENABLE(), configure DMA with HAL_DMA_Init(),
            link it with OctoSPI handle using __HAL_LINKDMA(), enable and configure
            DMA channel global interrupt with HAL_NVIC_SetPriority() and HAL_NVIC_EnableIRQ().
      (#) Configure the fifo threshold, the dual-quad mode, the memory type, the
          device size, the CS high time, the free running clock, the clock mode,
          the wrap size, the clock prescaler, the sample shifting, the hold delay
          and the CS boundary using the HAL_OSPI_Init() function.
      (#) When using Hyperbus, configure the RW recovery time, the access time,
          the write latency and the latency mode unsing the HAL_OSPI_HyperbusCfg()
          function.

    *** Indirect functional mode ***
    ================================
    [..]
      (#) In regular mode, configure the command sequence using the HAL_OSPI_Command()
          or HAL_OSPI_Command_IT() functions :
         (++) Instruction phase : the mode used and if present the size, the instruction
              opcode and the DTR mode.
         (++) Address phase : the mode used and if present the size, the address
              value and the DTR mode.
         (++) Alternate-bytes phase : the mode used and if present the size, the
              alternate bytes values and the DTR mode.
         (++) Dummy-cycles phase : the number of dummy cycles (mode used is same as data phase).
         (++) Data phase : the mode used and if present the number of bytes and the DTR mode.
         (++) Data strobe (DQS) mode : the activation (or not) of this mode
         (++) Sending Instruction Only Once (SIOO) mode : the activation (or not) of this mode.
         (++) Flash identifier : in dual-quad mode, indicates which flash is concerned
         (++) Operation type : always common configuration
      (#) In Hyperbus mode, configure the command sequence using the HAL_OSPI_HyperbusCmd()
          function :
         (++) Address space : indicate if the access will be done in register or memory
         (++) Address size
         (++) Number of data
         (++) Data strobe (DQS) mode : the activation (or not) of this mode
      (#) If no data is required for the command (only for regular mode, not for
          Hyperbus mode), it is sent directly to the memory :
         (++) In polling mode, the output of the function is done when the transfer is complete.
         (++) In interrupt mode, HAL_OSPI_CmdCpltCallback() will be called when the transfer is complete.
      (#) For the indirect write mode, use HAL_OSPI_Transmit(), HAL_OSPI_Transmit_DMA() or
          HAL_OSPI_Transmit_IT() after the command configuration :
         (++) In polling mode, the output of the function is done when the transfer is complete.
         (++) In interrupt mode, HAL_OSPI_FifoThresholdCallback() will be called when the fifo threshold
             is reached and HAL_OSPI_TxCpltCallback() will be called when the transfer is complete.
         (++) In DMA mode, HAL_OSPI_TxHalfCpltCallback() will be called at the half transfer and
             HAL_OSPI_TxCpltCallback() will be called when the transfer is complete.
      (#) For the indirect read mode, use HAL_OSPI_Receive(), HAL_OSPI_Receive_DMA() or
          HAL_OSPI_Receive_IT() after the command configuration :
         (++) In polling mode, the output of the function is done when the transfer is complete.
         (++) In interrupt mode, HAL_OSPI_FifoThresholdCallback() will be called when the fifo threshold
             is reached and HAL_OSPI_RxCpltCallback() will be called when the transfer is complete.
         (++) In DMA mode, HAL_OSPI_RxHalfCpltCallback() will be called at the half transfer and
             HAL_OSPI_RxCpltCallback() will be called when the transfer is complete.

    *** Auto-polling functional mode ***
    ====================================
    [..]
      (#) Configure the command sequence by the same way than the indirect mode
      (#) Configure the auto-polling functional mode using the HAL_OSPI_AutoPolling()
          or HAL_OSPI_AutoPolling_IT() functions :
         (++) The size of the status bytes, the match value, the mask used, the match mode (OR/AND),
             the polling interval and the automatic stop activation.
      (#) After the configuration :
         (++) In polling mode, the output of the function is done when the status match is reached. The
             automatic stop is activated to avoid an infinite loop.
         (++) In interrupt mode, HAL_OSPI_StatusMatchCallback() will be called each time the status match is reached.

    *** Memory-mapped functional mode ***
    =====================================
    [..]
      (#) Configure the command sequence by the same way than the indirect mode except
          for the operation type in regular mode :
         (++) Operation type equals to read configuration : the command configuration
              applies to read access in memory-mapped mode
         (++) Operation type equals to write configuration : the command configuration
              applies to write access in memory-mapped mode
         (++) Both read and write configuration should be performed before activating
              memory-mapped mode
      (#) Configure the memory-mapped functional mode using the HAL_OSPI_MemoryMapped()
          functions :
         (++) The timeout activation and the timeout period.
      (#) After the configuration, the OctoSPI will be used as soon as an access on the AHB is done on
          the address range. HAL_OSPI_TimeOutCallback() will be called when the timeout expires.

    *** Errors management and abort functionality ***
    =================================================
    [..]
      (#) HAL_OSPI_GetError() function gives the error raised during the last operation.
      (#) HAL_OSPI_Abort() and HAL_OSPI_AbortIT() functions aborts any on-going operation and
          flushes the fifo :
         (++) In polling mode, the output of the function is done when the transfer
              complete bit is set and the busy bit cleared.
         (++) In interrupt mode, HAL_OSPI_AbortCpltCallback() will be called when
              the transfer complete bit is set.

    *** Control functions ***
    =========================
    [..]
      (#) HAL_OSPI_GetState() function gives the current state of the HAL OctoSPI driver.
      (#) HAL_OSPI_SetTimeout() function configures the timeout value used in the driver.
      (#) HAL_OSPI_SetFifoThreshold() function configures the threshold on the Fifo of the OSPI Peripheral.
      (#) HAL_OSPI_GetFifoThreshold() function gives the current of the Fifo's threshold

    *** IO manager configuration functions ***
    ==========================================
    [..]
      (#) HAL_OSPIM_Config() function configures the IO manager for the OctoSPI instance.

    *** Callback registration ***
    =============================================
    [..]
      The compilation define  USE_HAL_OSPI_REGISTER_CALLBACKS when set to 1
      allows the user to configure dynamically the driver callbacks.

      Use Functions @ref HAL_OSPI_RegisterCallback() to register a user callback,
      it allows to register following callbacks:
        (+) ErrorCallback : callback when error occurs.
        (+) AbortCpltCallback : callback when abort is completed.
        (+) FifoThresholdCallback : callback when the fifo threshold is reached.
        (+) CmdCpltCallback : callback when a command without data is completed.
        (+) RxCpltCallback : callback when a reception transfer is completed.
        (+) TxCpltCallback : callback when a transmission transfer is completed.
        (+) RxHalfCpltCallback : callback when half of the reception transfer is completed.
        (+) TxHalfCpltCallback : callback when half of the transmission transfer is completed.
        (+) StatusMatchCallback : callback when a status match occurs.
        (+) TimeOutCallback : callback when the timeout perioed expires.
        (+) MspInitCallback    : OSPI MspInit.
        (+) MspDeInitCallback  : OSPI MspDeInit.
      This function takes as parameters the HAL peripheral handle, the Callback ID
      and a pointer to the user callback function.

      Use function @ref HAL_OSPI_UnRegisterCallback() to reset a callback to the default
      weak (surcharged) function. It allows to reset following callbacks:
        (+) ErrorCallback : callback when error occurs.
        (+) AbortCpltCallback : callback when abort is completed.
        (+) FifoThresholdCallback : callback when the fifo threshold is reached.
        (+) CmdCpltCallback : callback when a command without data is completed.
        (+) RxCpltCallback : callback when a reception transfer is completed.
        (+) TxCpltCallback : callback when a transmission transfer is completed.
        (+) RxHalfCpltCallback : callback when half of the reception transfer is completed.
        (+) TxHalfCpltCallback : callback when half of the transmission transfer is completed.
        (+) StatusMatchCallback : callback when a status match occurs.
        (+) TimeOutCallback : callback when the timeout perioed expires.
        (+) MspInitCallback    : OSPI MspInit.
        (+) MspDeInitCallback  : OSPI MspDeInit.
      This function) takes as parameters the HAL peripheral handle and the Callback ID.

      By default, after the @ref HAL_OSPI_Init and if the state is HAL_OSPI_STATE_RESET
      all callbacks are reset to the corresponding legacy weak (surcharged) functions.
      Exception done for MspInit and MspDeInit callbacks that are respectively
      reset to the legacy weak (surcharged) functions in the @ref HAL_OSPI_Init
      and @ref  HAL_OSPI_DeInit only when these callbacks are null (not registered beforehand).
      If not, MspInit or MspDeInit are not null, the @ref HAL_OSPI_Init and @ref HAL_OSPI_DeInit
      keep and use the user MspInit/MspDeInit callbacks (registered beforehand)

      Callbacks can be registered/unregistered in READY state only.
      Exception done for MspInit/MspDeInit callbacks that can be registered/unregistered
      in READY or RESET state, thus registered (user) MspInit/DeInit callbacks can be used
      during the Init/DeInit.
      In that case first register the MspInit/MspDeInit user callbacks
      using @ref HAL_OSPI_RegisterCallback before calling @ref HAL_OSPI_DeInit
      or @ref HAL_OSPI_Init function.

      When The compilation define USE_HAL_OSPI_REGISTER_CALLBACKS is set to 0 or
      not defined, the callback registering feature is not available
      and weak (surcharged) callbacks are used.

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
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

#if defined(OCTOSPI) || defined(OCTOSPI1) || defined(OCTOSPI2)

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup OSPI OSPI
  * @brief OSPI HAL module driver
  * @{
  */

#ifdef HAL_OSPI_MODULE_ENABLED

/**
  @cond 0
  */
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE ((uint32_t)0x00000000)         /*!< Indirect write mode    */
#define OSPI_FUNCTIONAL_MODE_INDIRECT_READ  ((uint32_t)OCTOSPI_CR_FMODE_0) /*!< Indirect read mode     */
#define OSPI_FUNCTIONAL_MODE_AUTO_POLLING   ((uint32_t)OCTOSPI_CR_FMODE_1) /*!< Automatic polling mode */
#define OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  ((uint32_t)OCTOSPI_CR_FMODE)   /*!< Memory-mapped mode     */

#define OSPI_CFG_STATE_MASK  0x00000004U
#define OSPI_BUSY_STATE_MASK 0x00000008U

#define OSPI_NB_INSTANCE   2U
#define OSPI_IOM_NB_PORTS  2U
#define OSPI_IOM_PORT_MASK 0x1U

/* Private macro -------------------------------------------------------------*/
#define IS_OSPI_FUNCTIONAL_MODE(MODE) (((MODE) == OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE) || \
                                       ((MODE) == OSPI_FUNCTIONAL_MODE_INDIRECT_READ)  || \
                                       ((MODE) == OSPI_FUNCTIONAL_MODE_AUTO_POLLING)   || \
                                       ((MODE) == OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED))

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void              OSPI_DMACplt                  (DMA_HandleTypeDef *hdma);
static void              OSPI_DMAHalfCplt              (DMA_HandleTypeDef *hdma);
static void              OSPI_DMAError                 (DMA_HandleTypeDef *hdma);
static void              OSPI_DMAAbortCplt             (DMA_HandleTypeDef *hdma);
static HAL_StatusTypeDef OSPI_WaitFlagStateUntilTimeout(OSPI_HandleTypeDef *hospi, uint32_t Flag, FlagStatus State, uint32_t Tickstart, uint32_t Timeout);
static HAL_StatusTypeDef OSPI_ConfigCmd                (OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd);
static HAL_StatusTypeDef OSPIM_GetConfig               (uint8_t instance_nb, OSPIM_CfgTypeDef *cfg);
/**
  @endcond
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup OSPI_Exported_Functions OSPI Exported Functions
  * @{
  */

/** @defgroup OSPI_Exported_Functions_Group1 Initialization/de-initialization functions
  * @brief    Initialization and Configuration functions
  *
@verbatim
===============================================================================
            ##### Initialization and Configuration functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to :
      (+) Initialize the OctoSPI.
      (+) De-initialize the OctoSPI.

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the OSPI mode according to the specified parameters
  *         in the OSPI_InitTypeDef and initialize the associated handle.
  * @param  hospi : OSPI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Init (OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();

  /* Check the OSPI handle allocation */
  if (hospi == NULL)
  {
    status = HAL_ERROR;
    /* No error code can be set set as the handler is null */
  }
  else
  {
    /* Check the parameters of the initialization structure */
    assert_param(IS_OSPI_FIFO_THRESHOLD (hospi->Init.FifoThreshold));
    assert_param(IS_OSPI_DUALQUAD_MODE  (hospi->Init.DualQuad));
    assert_param(IS_OSPI_MEMORY_TYPE    (hospi->Init.MemoryType));
    assert_param(IS_OSPI_DEVICE_SIZE    (hospi->Init.DeviceSize));
    assert_param(IS_OSPI_CS_HIGH_TIME   (hospi->Init.ChipSelectHighTime));
    assert_param(IS_OSPI_FREE_RUN_CLK   (hospi->Init.FreeRunningClock));
    assert_param(IS_OSPI_CLOCK_MODE     (hospi->Init.ClockMode));
    assert_param(IS_OSPI_WRAP_SIZE      (hospi->Init.WrapSize));
    assert_param(IS_OSPI_CLK_PRESCALER  (hospi->Init.ClockPrescaler));
    assert_param(IS_OSPI_SAMPLE_SHIFTING(hospi->Init.SampleShifting));
    assert_param(IS_OSPI_DHQC           (hospi->Init.DelayHoldQuarterCycle));
    assert_param(IS_OSPI_CS_BOUNDARY    (hospi->Init.ChipSelectBoundary));

    /* Initialize error code */
    hospi->ErrorCode = HAL_OSPI_ERROR_NONE;

    /* Check if the state is the reset state */
    if (hospi->State == HAL_OSPI_STATE_RESET)
    {
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
      /* Reset Callback pointers in HAL_OSPI_STATE_RESET only */
      hospi->ErrorCallback         = HAL_OSPI_ErrorCallback;
      hospi->AbortCpltCallback     = HAL_OSPI_AbortCpltCallback;
      hospi->FifoThresholdCallback = HAL_OSPI_FifoThresholdCallback;
      hospi->CmdCpltCallback       = HAL_OSPI_CmdCpltCallback;
      hospi->RxCpltCallback        = HAL_OSPI_RxCpltCallback;
      hospi->TxCpltCallback        = HAL_OSPI_TxCpltCallback;
      hospi->RxHalfCpltCallback    = HAL_OSPI_RxHalfCpltCallback;
      hospi->TxHalfCpltCallback    = HAL_OSPI_TxHalfCpltCallback;
      hospi->StatusMatchCallback   = HAL_OSPI_StatusMatchCallback;
      hospi->TimeOutCallback       = HAL_OSPI_TimeOutCallback;

      if(hospi->MspInitCallback == NULL)
      {
        hospi->MspInitCallback = HAL_OSPI_MspInit;
      }

      /* Init the low level hardware */
      hospi->MspInitCallback(hospi);
#else
      /* Initialization of the low level hardware */
      HAL_OSPI_MspInit(hospi);
#endif

      /* Configure the default timeout for the OSPI memory access */
      status = HAL_OSPI_SetTimeout(hospi, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    }

    if (status == HAL_OK)
    {
     /* Configure memory type, device size, chip select high time, free running clock, clock mode */
      MODIFY_REG(hospi->Instance->DCR1, (OCTOSPI_DCR1_MTYP | OCTOSPI_DCR1_DEVSIZE | OCTOSPI_DCR1_CSHT | OCTOSPI_DCR1_FRCK | OCTOSPI_DCR1_CKMODE),
                 (hospi->Init.MemoryType | ((hospi->Init.DeviceSize - 1U) << OCTOSPI_DCR1_DEVSIZE_Pos) |
                  ((hospi->Init.ChipSelectHighTime - 1U) << OCTOSPI_DCR1_CSHT_Pos) | hospi->Init.ClockMode));

      /* Configure wrap size */
      MODIFY_REG(hospi->Instance->DCR2, OCTOSPI_DCR2_WRAPSIZE, hospi->Init.WrapSize);

      /* Configure chip select boundary */
      hospi->Instance->DCR3 = (hospi->Init.ChipSelectBoundary << OCTOSPI_DCR3_CSBOUND_Pos);


      /* Configure FIFO threshold */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FTHRES, ((hospi->Init.FifoThreshold - 1U) << OCTOSPI_CR_FTHRES_Pos));

      /* Wait till busy flag is reset */
      status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

      if (status == HAL_OK)
      {
         /* Configure clock prescaler */
         MODIFY_REG(hospi->Instance->DCR2, OCTOSPI_DCR2_PRESCALER, ((hospi->Init.ClockPrescaler - 1U) << OCTOSPI_DCR2_PRESCALER_Pos));

         /* Configure Dual Quad mode */
         MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_DQM, hospi->Init.DualQuad);

         /* Configure sample shifting and delay hold quarter cycle */
         MODIFY_REG(hospi->Instance->TCR, (OCTOSPI_TCR_SSHIFT | OCTOSPI_TCR_DHQC), (hospi->Init.SampleShifting | hospi->Init.DelayHoldQuarterCycle));

         /* Enable OctoSPI */
         __HAL_OSPI_ENABLE(hospi);
         
         /* Enable free running clock if needed : must be done after OSPI enable */
         if (hospi->Init.FreeRunningClock == HAL_OSPI_FREERUNCLK_ENABLE)
         {
           SET_BIT(hospi->Instance->DCR1, OCTOSPI_DCR1_FRCK);
         }

         /* Initialize the OSPI state */
         if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
         {
            hospi->State = HAL_OSPI_STATE_HYPERBUS_INIT;
         }
         else
         {
            hospi->State = HAL_OSPI_STATE_READY;
         }
      }
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Initialize the OSPI MSP.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_MspInit(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_MspInit can be implemented in the user file
   */
}

/**
  * @brief  De-Initialize the OSPI peripheral.
  * @param  hospi : OSPI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_DeInit(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the OSPI handle allocation */
  if (hospi == NULL)
  {
    status = HAL_ERROR;
    /* No error code can be set set as the handler is null */
  }
  else
  {
     /* Disable OctoSPI */
     __HAL_OSPI_DISABLE(hospi);

     /* Disable free running clock if needed : must be done after OSPI disable */
     CLEAR_BIT(hospi->Instance->DCR1, OCTOSPI_DCR1_FRCK);

#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
     if(hospi->MspDeInitCallback == NULL)
     {
       hospi->MspDeInitCallback = HAL_OSPI_MspDeInit;
     }

     /* DeInit the low level hardware */
     hospi->MspDeInitCallback(hospi);
#else
     /* De-initialize the low-level hardware */
     HAL_OSPI_MspDeInit(hospi);
#endif

     /* Reset the driver state */
     hospi->State = HAL_OSPI_STATE_RESET;
  }

  return status;
}

/**
  * @brief  DeInitialize the OSPI MSP.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_MspDeInit(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_MspDeInit can be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup OSPI_Exported_Functions_Group2 Input and Output operation functions
  *  @brief OSPI Transmit/Receive functions
  *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to :
      (+) Handle the interrupts.
      (+) Handle the command sequence (regular and Hyperbus).
      (+) Handle the Hyperbus configuration.
      (+) Transmit data in blocking, interrupt or DMA mode.
      (+) Receive data in blocking, interrupt or DMA mode.
      (+) Manage the auto-polling functional mode.
      (+) Manage the memory-mapped functional mode.

@endverbatim
  * @{
  */

/**
  * @brief  Handle OSPI interrupt request.
  * @param  hospi : OSPI handle
  * @retval None
  */
void HAL_OSPI_IRQHandler(OSPI_HandleTypeDef *hospi)
{
  __IO uint32_t *data_reg = &hospi->Instance->DR;
  uint32_t flag           = hospi->Instance->SR;
  uint32_t itsource       = hospi->Instance->CR;
  uint32_t currentstate   = hospi->State;

  /* OctoSPI fifo threshold interrupt occurred -------------------------------*/
  if (((flag & HAL_OSPI_FLAG_FT) != 0U) && ((itsource & HAL_OSPI_IT_FT) != 0U))
  {
    if (currentstate == HAL_OSPI_STATE_BUSY_TX)
    {
      /* Write a data in the fifo */
      *((__IO uint8_t *)data_reg) = *hospi->pBuffPtr;
      hospi->pBuffPtr++;
      hospi->XferCount--;
    }
    else if (currentstate == HAL_OSPI_STATE_BUSY_RX)
    {
      /* Read a data from the fifo */
      *hospi->pBuffPtr = *((__IO uint8_t *)data_reg);
      hospi->pBuffPtr++;
      hospi->XferCount--;
    }
    else
    {
      /* Nothing to do */
    }

    if (hospi->XferCount == 0U)
    {
      /* All data have been received or transmitted for the transfer */
      /* Disable fifo threshold interrupt */
      __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_FT);
    }

    /* Fifo threshold callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->FifoThresholdCallback(hospi);
#else
    HAL_OSPI_FifoThresholdCallback(hospi);
#endif
  }
  /* OctoSPI transfer complete interrupt occurred ----------------------------*/
  else if (((flag & HAL_OSPI_FLAG_TC) != 0U) && ((itsource & HAL_OSPI_IT_TC) != 0U))
  {
    if (currentstate == HAL_OSPI_STATE_BUSY_RX)
    {
      if ((hospi->XferCount > 0U) && ((flag & OCTOSPI_SR_FLEVEL) != 0U))
      {
        /* Read the last data received in the fifo */
        *hospi->pBuffPtr = *((__IO uint8_t *)data_reg);
        hospi->pBuffPtr++;
        hospi->XferCount--;
      }
      else if(hospi->XferCount == 0U)
      {
        /* Clear flag */
        hospi->Instance->FCR = HAL_OSPI_FLAG_TC;

        /* Disable the interrupts */
        __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* RX complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->RxCpltCallback(hospi);
#else
        HAL_OSPI_RxCpltCallback(hospi);
#endif
      }
      else
      {
        /* Nothing to do */
      }
    }
    else
    {
      /* Clear flag */
      hospi->Instance->FCR = HAL_OSPI_FLAG_TC;

      /* Disable the interrupts */
      __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;

      if (currentstate == HAL_OSPI_STATE_BUSY_TX)
      {
        /* TX complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->TxCpltCallback(hospi);
#else
        HAL_OSPI_TxCpltCallback(hospi);
#endif
      }
      else if (currentstate == HAL_OSPI_STATE_BUSY_CMD)
      {
        /* Command complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->CmdCpltCallback(hospi);
#else
        HAL_OSPI_CmdCpltCallback(hospi);
#endif
      }
      else if (currentstate == HAL_OSPI_STATE_ABORT)
      {
        if (hospi->ErrorCode == HAL_OSPI_ERROR_NONE)
        {
          /* Abort called by the user */
          /* Abort complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
          hospi->AbortCpltCallback(hospi);
#else
          HAL_OSPI_AbortCpltCallback(hospi);
#endif
        }
        else
        {
          /* Abort due to an error (eg : DMA error) */
          /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
          hospi->ErrorCallback(hospi);
#else
          HAL_OSPI_ErrorCallback(hospi);
#endif
        }
      }
      else
      {
        /* Nothing to do */
      }
    }
  }
  /* OctoSPI status match interrupt occurred ---------------------------------*/
  else if (((flag & HAL_OSPI_FLAG_SM) != 0U) && ((itsource & HAL_OSPI_IT_SM) != 0U))
  {
    /* Clear flag */
    hospi->Instance->FCR = HAL_OSPI_FLAG_SM;

    /* Check if automatic poll mode stop is activated */
    if ((hospi->Instance->CR & OCTOSPI_CR_APMS) != 0U)
    {
      /* Disable the interrupts */
      __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_SM | HAL_OSPI_IT_TE);

      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;
    }

    /* Status match callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->StatusMatchCallback(hospi);
#else
    HAL_OSPI_StatusMatchCallback(hospi);
#endif
  }
  /* OctoSPI transfer error interrupt occurred -------------------------------*/
  else if (((flag & HAL_OSPI_FLAG_TE) != 0U) && ((itsource & HAL_OSPI_IT_TE) != 0U))
  {
    /* Clear flag */
    hospi->Instance->FCR = HAL_OSPI_FLAG_TE;

    /* Disable all interrupts */
    __HAL_OSPI_DISABLE_IT(hospi, (HAL_OSPI_IT_TO | HAL_OSPI_IT_SM | HAL_OSPI_IT_FT | HAL_OSPI_IT_TC | HAL_OSPI_IT_TE));

    /* Set error code */
    hospi->ErrorCode = HAL_OSPI_ERROR_TRANSFER;

    /* Check if the DMA is enabled */
    if ((hospi->Instance->CR & OCTOSPI_CR_DMAEN) != 0U)
    {
      /* Disable the DMA transfer on the OctoSPI side */
      CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

      /* Disable the DMA transfer on the DMA side */
      hospi->hdma->XferAbortCallback = OSPI_DMAAbortCplt;
      if (HAL_DMA_Abort_IT(hospi->hdma) != HAL_OK)
      {
        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->ErrorCallback(hospi);
#else
        HAL_OSPI_ErrorCallback(hospi);
#endif
      }
    }
    else
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;

      /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
      hospi->ErrorCallback(hospi);
#else
      HAL_OSPI_ErrorCallback(hospi);
#endif
    }
  }
  /* OctoSPI timeout interrupt occurred --------------------------------------*/
  else if (((flag & HAL_OSPI_FLAG_TO) != 0U) && ((itsource & HAL_OSPI_IT_TO) != 0U))
  {
    /* Clear flag */
    hospi->Instance->FCR = HAL_OSPI_FLAG_TO;

    /* Timeout callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->TimeOutCallback(hospi);
#else
    HAL_OSPI_TimeOutCallback(hospi);
#endif
  }
  else
  {
    /* Nothing to do */
  }
}

/**
  * @brief  Set the command configuration.
  * @param  hospi   : OSPI handle
  * @param  cmd     : structure that contains the command configuration information
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Command(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t state;
  uint32_t tickstart = HAL_GetTick();

  /* Check the parameters of the command structure */
  assert_param(IS_OSPI_OPERATION_TYPE(cmd->OperationType));

  if (hospi->Init.DualQuad == HAL_OSPI_DUALQUAD_DISABLE)
  {
    assert_param(IS_OSPI_FLASH_ID(cmd->FlashId));
  }

  assert_param(IS_OSPI_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != HAL_OSPI_INSTRUCTION_NONE)
  {
    assert_param(IS_OSPI_INSTRUCTION_SIZE    (cmd->InstructionSize));
    assert_param(IS_OSPI_INSTRUCTION_DTR_MODE(cmd->InstructionDtrMode));
  }

  assert_param(IS_OSPI_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != HAL_OSPI_ADDRESS_NONE)
  {
    assert_param(IS_OSPI_ADDRESS_SIZE    (cmd->AddressSize));
    assert_param(IS_OSPI_ADDRESS_DTR_MODE(cmd->AddressDtrMode));
  }

  assert_param(IS_OSPI_ALT_BYTES_MODE(cmd->AlternateBytesMode));
  if (cmd->AlternateBytesMode != HAL_OSPI_ALTERNATE_BYTES_NONE)
  {
    assert_param(IS_OSPI_ALT_BYTES_SIZE    (cmd->AlternateBytesSize));
    assert_param(IS_OSPI_ALT_BYTES_DTR_MODE(cmd->AlternateBytesDtrMode));
  }

  assert_param(IS_OSPI_DATA_MODE(cmd->DataMode));
  if (cmd->DataMode != HAL_OSPI_DATA_NONE)
  {
    if (cmd->OperationType == HAL_OSPI_OPTYPE_COMMON_CFG)
    {
      assert_param(IS_OSPI_NUMBER_DATA  (cmd->NbData));
    }
    assert_param(IS_OSPI_DATA_DTR_MODE(cmd->DataDtrMode));
    assert_param(IS_OSPI_DUMMY_CYCLES (cmd->DummyCycles));
  }

  assert_param(IS_OSPI_DQS_MODE (cmd->DQSMode));
  assert_param(IS_OSPI_SIOO_MODE(cmd->SIOOMode));

  /* Check the state of the driver */
  state = hospi->State;
  if (((state == HAL_OSPI_STATE_READY)         && (hospi->Init.MemoryType != HAL_OSPI_MEMTYPE_HYPERBUS)) ||
      ((state == HAL_OSPI_STATE_READ_CMD_CFG)  && (cmd->OperationType == HAL_OSPI_OPTYPE_WRITE_CFG))     ||
      ((state == HAL_OSPI_STATE_WRITE_CMD_CFG) && (cmd->OperationType == HAL_OSPI_OPTYPE_READ_CFG)))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Initialize error code */
      hospi->ErrorCode = HAL_OSPI_ERROR_NONE;

      /* Configure the registers */
      status = OSPI_ConfigCmd(hospi, cmd);

      if (status == HAL_OK)
      {
        if (cmd->DataMode == HAL_OSPI_DATA_NONE)
        {
          /* When there is no data phase, the transfer start as soon as the configuration is done
             so wait until TC flag is set to go back in idle state */
          status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, Timeout);

          __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);
        }
        else
        {
          /* Update the state */
          if (cmd->OperationType == HAL_OSPI_OPTYPE_COMMON_CFG)
          {
            hospi->State = HAL_OSPI_STATE_CMD_CFG;
          }
          else if (cmd->OperationType == HAL_OSPI_OPTYPE_READ_CFG)
          {
            if (hospi->State == HAL_OSPI_STATE_WRITE_CMD_CFG)
            {
              hospi->State = HAL_OSPI_STATE_CMD_CFG;
            }
            else
            {
              hospi->State = HAL_OSPI_STATE_READ_CMD_CFG;
            }
          }
          else
          {
            if (hospi->State == HAL_OSPI_STATE_READ_CMD_CFG)
            {
              hospi->State = HAL_OSPI_STATE_CMD_CFG;
            }
            else
            {
              hospi->State = HAL_OSPI_STATE_WRITE_CMD_CFG;
            }
          }
        }
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Set the command configuration in interrupt mode.
  * @param  hospi : OSPI handle
  * @param  cmd   : structure that contains the command configuration information
  * @note   This function is used only in Indirect Read or Write Modes
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Command_IT(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the parameters of the command structure */
  assert_param(IS_OSPI_OPERATION_TYPE(cmd->OperationType));

  if (hospi->Init.DualQuad == HAL_OSPI_DUALQUAD_DISABLE)
  {
    assert_param(IS_OSPI_FLASH_ID(cmd->FlashId));
  }

  assert_param(IS_OSPI_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != HAL_OSPI_INSTRUCTION_NONE)
  {
    assert_param(IS_OSPI_INSTRUCTION_SIZE    (cmd->InstructionSize));
    assert_param(IS_OSPI_INSTRUCTION_DTR_MODE(cmd->InstructionDtrMode));
  }

  assert_param(IS_OSPI_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != HAL_OSPI_ADDRESS_NONE)
  {
    assert_param(IS_OSPI_ADDRESS_SIZE    (cmd->AddressSize));
    assert_param(IS_OSPI_ADDRESS_DTR_MODE(cmd->AddressDtrMode));
  }

  assert_param(IS_OSPI_ALT_BYTES_MODE(cmd->AlternateBytesMode));
  if (cmd->AlternateBytesMode != HAL_OSPI_ALTERNATE_BYTES_NONE)
  {
    assert_param(IS_OSPI_ALT_BYTES_SIZE    (cmd->AlternateBytesSize));
    assert_param(IS_OSPI_ALT_BYTES_DTR_MODE(cmd->AlternateBytesDtrMode));
  }

  assert_param(IS_OSPI_DATA_MODE(cmd->DataMode));
  if (cmd->DataMode != HAL_OSPI_DATA_NONE)
  {
    assert_param(IS_OSPI_NUMBER_DATA  (cmd->NbData));
    assert_param(IS_OSPI_DATA_DTR_MODE(cmd->DataDtrMode));
    assert_param(IS_OSPI_DUMMY_CYCLES (cmd->DummyCycles));
  }

  assert_param(IS_OSPI_DQS_MODE (cmd->DQSMode));
  assert_param(IS_OSPI_SIOO_MODE(cmd->SIOOMode));

  /* Check the state of the driver */
  if ((hospi->State  == HAL_OSPI_STATE_READY) && (cmd->OperationType     == HAL_OSPI_OPTYPE_COMMON_CFG) &&
      (cmd->DataMode == HAL_OSPI_DATA_NONE)   && (hospi->Init.MemoryType != HAL_OSPI_MEMTYPE_HYPERBUS))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

    if (status == HAL_OK)
    {
      /* Initialize error code */
      hospi->ErrorCode = HAL_OSPI_ERROR_NONE;

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

      /* Configure the registers */
      status = OSPI_ConfigCmd(hospi, cmd);

      if (status == HAL_OK)
      {
        /* Update the state */
          hospi->State = HAL_OSPI_STATE_BUSY_CMD;

        /* Enable the transfer complete and transfer error interrupts */
        __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_TE);
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the Hyperbus parameters.
  * @param  hospi   : OSPI handle
  * @param  cfg     : Structure containing the Hyperbus configuration
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_HyperbusCfg(OSPI_HandleTypeDef *hospi, OSPI_HyperbusCfgTypeDef *cfg, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t state;
  uint32_t tickstart = HAL_GetTick();

  /* Check the parameters of the hyperbus configuration structure */
  assert_param(IS_OSPI_RW_RECOVERY_TIME  (cfg->RWRecoveryTime));
  assert_param(IS_OSPI_ACCESS_TIME       (cfg->AccessTime));
  assert_param(IS_OSPI_WRITE_ZERO_LATENCY(cfg->WriteZeroLatency));
  assert_param(IS_OSPI_LATENCY_MODE      (cfg->LatencyMode));

  /* Check the state of the driver */
  state = hospi->State;
  if ((state == HAL_OSPI_STATE_HYPERBUS_INIT) || (state == HAL_OSPI_STATE_READY))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Configure Hyperbus configuration Latency register */
      WRITE_REG(hospi->Instance->HLCR, ((cfg->RWRecoveryTime << OCTOSPI_HLCR_TRWR_Pos) |
                                        (cfg->AccessTime << OCTOSPI_HLCR_TACC_Pos)     |
                                        cfg->WriteZeroLatency | cfg->LatencyMode));

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_READY;
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Set the Hyperbus command configuration.
  * @param  hospi   : OSPI handle
  * @param  cmd     : Structure containing the Hyperbus command
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_HyperbusCmd(OSPI_HandleTypeDef *hospi, OSPI_HyperbusCmdTypeDef *cmd, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the parameters of the hyperbus command structure */
  assert_param(IS_OSPI_ADDRESS_SPACE(cmd->AddressSpace));
  assert_param(IS_OSPI_ADDRESS_SIZE (cmd->AddressSize));
  assert_param(IS_OSPI_NUMBER_DATA  (cmd->NbData));
  assert_param(IS_OSPI_DQS_MODE     (cmd->DQSMode));

  /* Check the state of the driver */
  if ((hospi->State == HAL_OSPI_STATE_READY) && (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Re-initialize the value of the functional mode */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, 0U);

      /* Configure the address space in the DCR1 register */
      MODIFY_REG(hospi->Instance->DCR1, OCTOSPI_DCR1_MTYP_0, cmd->AddressSpace);

      /* Configure the CCR and WCCR registers with the address size and the following configuration :
         - DQS signal enabled (used as RWDS)
         - DTR mode enabled on address and data
         - address and data on 8 lines */
      WRITE_REG(hospi->Instance->CCR, (cmd->DQSMode | OCTOSPI_CCR_DDTR | OCTOSPI_CCR_DMODE_2 |
                                       cmd->AddressSize | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADMODE_2));
      WRITE_REG(hospi->Instance->WCCR, (cmd->DQSMode | OCTOSPI_WCCR_DDTR | OCTOSPI_WCCR_DMODE_2 |
                                        cmd->AddressSize | OCTOSPI_WCCR_ADDTR | OCTOSPI_WCCR_ADMODE_2));

      /* Configure the DLR register with the number of data */
      WRITE_REG(hospi->Instance->DLR, (cmd->NbData - 1U));

      /* Configure the AR register with the address value */
      WRITE_REG(hospi->Instance->AR, cmd->Address);

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_CMD_CFG;
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Transmit an amount of data in blocking mode.
  * @param  hospi   : OSPI handle
  * @param  pData   : pointer to data buffer
  * @param  Timeout : Timeout duration
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Transmit(OSPI_HandleTypeDef *hospi, uint8_t *pData, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  __IO uint32_t *data_reg = &hospi->Instance->DR;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect write */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

      do
      {
        /* Wait till fifo threshold flag is set to send data */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_FT, SET, tickstart, Timeout);

        if (status != HAL_OK)
        {
          break;
        }

        *((__IO uint8_t *)data_reg) = *hospi->pBuffPtr;
        hospi->pBuffPtr++;
        hospi->XferCount--;
      } while (hospi->XferCount > 0U);

      if (status == HAL_OK)
      {
        /* Wait till transfer complete flag is set to go back in idle state */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, Timeout);

        if (status == HAL_OK)
        {
          /* Clear transfer complete flag */
          __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

          /* Update state */
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Receive an amount of data in blocking mode.
  * @param  hospi   : OSPI handle
  * @param  pData   : pointer to data buffer
  * @param  Timeout : Timeout duration
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Receive(OSPI_HandleTypeDef *hospi, uint8_t *pData, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  __IO uint32_t *data_reg = &hospi->Instance->DR;
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect read */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_READ);

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }

      do
      {
        /* Wait till fifo threshold or transfer complete flags are set to read received data */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, (HAL_OSPI_FLAG_FT | HAL_OSPI_FLAG_TC), SET, tickstart, Timeout);

        if (status != HAL_OK)
        {
          break;
        }

        *hospi->pBuffPtr = *((__IO uint8_t *)data_reg);
        hospi->pBuffPtr++;
        hospi->XferCount--;
      } while(hospi->XferCount > 0U);

      if (status == HAL_OK)
      {
        /* Wait till transfer complete flag is set to go back in idle state */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, Timeout);

        if (status == HAL_OK)
        {
          /* Clear transfer complete flag */
          __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

          /* Update state */
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Send an amount of data in non-blocking mode with interrupt.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Transmit_IT(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect write */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_BUSY_TX;

      /* Enable the transfer complete, fifo threshold and transfer error interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Receive an amount of data in non-blocking mode with interrupt.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Receive_IT(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect read */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_READ);

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_BUSY_RX;

      /* Enable the transfer complete, fifo threshold and transfer error interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Send an amount of data in non-blocking mode with DMA.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @note   If DMA peripheral access is configured as halfword, the number
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Transmit_DMA(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t data_size = hospi->Instance->DLR + 1U;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      if (hospi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_BYTE)
      {
        hospi->XferCount = data_size;
      }
      else if (hospi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_HALFWORD)
      {
        if (((data_size % 2U) != 0U) || ((hospi->Init.FifoThreshold % 2U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on halfword
          => no transfer possible with DMA peripheral access configured as halfword */
          hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;
        }
        else
        {
          hospi->XferCount = (data_size >> 1);
        }
      }
      else if (hospi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_WORD)
      {
        if (((data_size % 4U) != 0U) || ((hospi->Init.FifoThreshold % 4U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on word
          => no transfer possible with DMA peripheral access configured as word */
          hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;
        }
        else
        {
          hospi->XferCount = (data_size >> 2);
        }
      }
      else
      {
        /* Nothing to do */
      }

      if (status == HAL_OK)
      {
        hospi->XferSize = hospi->XferCount;
        hospi->pBuffPtr = pData;

        /* Configure CR register with functional mode as indirect write */
        MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

        /* Clear flags related to interrupt */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

        /* Update the state */
        hospi->State = HAL_OSPI_STATE_BUSY_TX;

        /* Set the DMA transfer complete callback */
        hospi->hdma->XferCpltCallback = OSPI_DMACplt;

        /* Set the DMA Half transfer complete callback */
        hospi->hdma->XferHalfCpltCallback = OSPI_DMAHalfCplt;

        /* Set the DMA error callback */
        hospi->hdma->XferErrorCallback = OSPI_DMAError;

        /* Clear the DMA abort callback */
        hospi->hdma->XferAbortCallback = NULL;

        /* Configure the direction of the DMA */
        hospi->hdma->Init.Direction = DMA_MEMORY_TO_PERIPH;
        MODIFY_REG(hospi->hdma->Instance->CCR, DMA_CCR_DIR, hospi->hdma->Init.Direction);

        /* Enable the transmit DMA Channel */
        if (HAL_DMA_Start_IT(hospi->hdma, (uint32_t)pData, (uint32_t)&hospi->Instance->DR, hospi->XferSize) == HAL_OK)
        {
          /* Enable the transfer error interrupt */
          __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TE);

          /* Enable the DMA transfer by setting the DMAEN bit  */
          SET_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);
        }
        else
        {
          status = HAL_ERROR;
          hospi->ErrorCode = HAL_OSPI_ERROR_DMA;
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Receive an amount of data in non-blocking mode with DMA.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer.
  * @note   This function is used only in Indirect Read Mode
  * @note   If DMA peripheral access is configured as halfword, the number
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Receive_DMA(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t data_size = hospi->Instance->DLR + 1U;
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      if (hospi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_BYTE)
      {
        hospi->XferCount = data_size;
      }
      else if (hospi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_HALFWORD)
      {
        if (((data_size % 2U) != 0U) || ((hospi->Init.FifoThreshold % 2U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on halfword
          => no transfer possible with DMA peripheral access configured as halfword */
          hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;
        }
        else
        {
          hospi->XferCount = (data_size >> 1);
        }
      }
      else if (hospi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_WORD)
      {
        if (((data_size % 4U) != 0U) || ((hospi->Init.FifoThreshold % 4U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on word
          => no transfer possible with DMA peripheral access configured as word */
          hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;
        }
        else
        {
          hospi->XferCount = (data_size >> 2);
        }
      }
      else
      {
        /* Nothing to do */
      }

      if (status == HAL_OK)
      {
        hospi->XferSize  = hospi->XferCount;
        hospi->pBuffPtr  = pData;

        /* Configure CR register with functional mode as indirect read */
        MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_READ);

        /* Clear flags related to interrupt */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

        /* Update the state */
        hospi->State = HAL_OSPI_STATE_BUSY_RX;

        /* Set the DMA transfer complete callback */
        hospi->hdma->XferCpltCallback = OSPI_DMACplt;

        /* Set the DMA Half transfer complete callback */
        hospi->hdma->XferHalfCpltCallback = OSPI_DMAHalfCplt;

        /* Set the DMA error callback */
        hospi->hdma->XferErrorCallback = OSPI_DMAError;

        /* Clear the DMA abort callback */
        hospi->hdma->XferAbortCallback = NULL;

        /* Configure the direction of the DMA */
        hospi->hdma->Init.Direction = DMA_PERIPH_TO_MEMORY;
        MODIFY_REG(hospi->hdma->Instance->CCR, DMA_CCR_DIR, hospi->hdma->Init.Direction);

        /* Enable the transmit DMA Channel */
        if (HAL_DMA_Start_IT(hospi->hdma, (uint32_t)&hospi->Instance->DR, (uint32_t)pData, hospi->XferSize) == HAL_OK)
        {
          /* Enable the transfer error interrupt */
          __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TE);

          /* Trig the transfer by re-writing address or instruction register */
          if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
          {
            WRITE_REG(hospi->Instance->AR, addr_reg);
          }
          else
          {
            if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
            {
              WRITE_REG(hospi->Instance->AR, addr_reg);
            }
            else
            {
              WRITE_REG(hospi->Instance->IR, ir_reg);
            }
          }

          /* Enable the DMA transfer by setting the DMAEN bit  */
          SET_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);
        }
        else
        {
          status = HAL_ERROR;
          hospi->ErrorCode = HAL_OSPI_ERROR_DMA;
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the OSPI Automatic Polling Mode in blocking mode.
  * @param  hospi   : OSPI handle
  * @param  cfg     : structure that contains the polling configuration information.
  * @param  Timeout : Timeout duration
  * @note   This function is used only in Automatic Polling Mode
  * @note   This function should not be used when the memory is in octal mode (see Errata Sheet)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_AutoPolling(OSPI_HandleTypeDef *hospi, OSPI_AutoPollingTypeDef *cfg, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;
#ifdef USE_FULL_ASSERT
  uint32_t dlr_reg = hospi->Instance->DLR;
#endif

  /* Check the parameters of the autopolling configuration structure */
  assert_param(IS_OSPI_MATCH_MODE       (cfg->MatchMode));
  assert_param(IS_OSPI_AUTOMATIC_STOP   (cfg->AutomaticStop));
  assert_param(IS_OSPI_INTERVAL         (cfg->Interval));
  assert_param(IS_OSPI_STATUS_BYTES_SIZE(dlr_reg+1U));

  /* Check the state */
  if ((hospi->State == HAL_OSPI_STATE_CMD_CFG) && (cfg->AutomaticStop == HAL_OSPI_AUTOMATIC_STOP_ENABLE))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Configure registers */
      WRITE_REG (hospi->Instance->PSMAR, cfg->Match);
      WRITE_REG (hospi->Instance->PSMKR, cfg->Mask);
      WRITE_REG (hospi->Instance->PIR,   cfg->Interval);
      MODIFY_REG(hospi->Instance->CR,    (OCTOSPI_CR_PMM | OCTOSPI_CR_APMS | OCTOSPI_CR_FMODE),
                 (cfg->MatchMode | cfg->AutomaticStop | OSPI_FUNCTIONAL_MODE_AUTO_POLLING));

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }

      /* Wait till status match flag is set to go back in idle state */
      status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_SM, SET, tickstart, Timeout);

      if (status == HAL_OK)
      {
        /* Clear status match flag */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_SM);

        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the OSPI Automatic Polling Mode in non-blocking mode.
  * @param  hospi : OSPI handle
  * @param  cfg   : structure that contains the polling configuration information.
  * @note   This function is used only in Automatic Polling Mode
  * @note   This function should not be used when the memory is in octal mode (see Errata Sheet)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_AutoPolling_IT(OSPI_HandleTypeDef *hospi, OSPI_AutoPollingTypeDef *cfg)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;
#ifdef USE_FULL_ASSERT
  uint32_t dlr_reg = hospi->Instance->DLR;
#endif

  /* Check the parameters of the autopolling configuration structure */
  assert_param(IS_OSPI_MATCH_MODE       (cfg->MatchMode));
  assert_param(IS_OSPI_AUTOMATIC_STOP   (cfg->AutomaticStop));
  assert_param(IS_OSPI_INTERVAL         (cfg->Interval));
  assert_param(IS_OSPI_STATUS_BYTES_SIZE(dlr_reg+1U));

  /* Check the state */
  if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

    if (status == HAL_OK)
    {
      /* Configure registers */
      WRITE_REG (hospi->Instance->PSMAR, cfg->Match);
      WRITE_REG (hospi->Instance->PSMKR, cfg->Mask);
      WRITE_REG (hospi->Instance->PIR,   cfg->Interval);
      MODIFY_REG(hospi->Instance->CR,    (OCTOSPI_CR_PMM | OCTOSPI_CR_APMS | OCTOSPI_CR_FMODE),
                 (cfg->MatchMode | cfg->AutomaticStop | OSPI_FUNCTIONAL_MODE_AUTO_POLLING));

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_SM);

      /* Update state */
      hospi->State = HAL_OSPI_STATE_BUSY_AUTO_POLLING;

      /* Enable the status match and transfer error interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_SM | HAL_OSPI_IT_TE);

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the Memory Mapped mode.
  * @param  hospi : OSPI handle
  * @param  cfg   : structure that contains the memory mapped configuration information.
  * @note   This function is used only in Memory mapped Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_MemoryMapped(OSPI_HandleTypeDef *hospi, OSPI_MemoryMappedTypeDef *cfg)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the parameters of the memory-mapped configuration structure */
  assert_param(IS_OSPI_TIMEOUT_ACTIVATION(cfg->TimeOutActivation));

  /* Check the state */
  if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

    if (status == HAL_OK)
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_BUSY_MEM_MAPPED;

      if (cfg->TimeOutActivation == HAL_OSPI_TIMEOUT_COUNTER_ENABLE)
      {
        assert_param(IS_OSPI_TIMEOUT_PERIOD(cfg->TimeOutPeriod));

        /* Configure register */
        WRITE_REG(hospi->Instance->LPTR, cfg->TimeOutPeriod);

        /* Clear flags related to interrupt */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TO);

        /* Enable the timeout interrupt */
        __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TO);
      }

      /* Configure CR register with functional mode as memory-mapped */
      MODIFY_REG(hospi->Instance->CR, (OCTOSPI_CR_TCEN | OCTOSPI_CR_FMODE),
                 (cfg->TimeOutActivation | OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED));
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Transfer Error callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_ErrorCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_ErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Abort completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_AbortCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_AbortCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  FIFO Threshold callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_FifoThresholdCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_FIFOThresholdCallback could be implemented in the user file
   */
}

/**
  * @brief  Command completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_CmdCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_CmdCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_RxCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_RxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
 __weak void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_TxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Half Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_RxHalfCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_RxHalfCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Half Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_TxHalfCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_TxHalfCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Status Match callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_StatusMatchCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_StatusMatchCallback could be implemented in the user file
   */
}

/**
  * @brief  Timeout callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_TimeOutCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_TimeOutCallback could be implemented in the user file
   */
}

#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
/**
  * @brief  Register a User OSPI Callback
  *         To be used instead of the weak (surcharged) predefined callback
  * @param hospi : OSPI handle
  * @param CallbackID : ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OSPI_ERROR_CB_ID          OSPI Error Callback ID
  *          @arg @ref HAL_OSPI_ABORT_CB_ID          OSPI Abort Callback ID
  *          @arg @ref HAL_OSPI_FIFO_THRESHOLD_CB_ID OSPI FIFO Threshold Callback ID
  *          @arg @ref HAL_OSPI_CMD_CPLT_CB_ID       OSPI Command Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_CPLT_CB_ID        OSPI Rx Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_CPLT_CB_ID        OSPI Tx Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_HALF_CPLT_CB_ID   OSPI Rx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_HALF_CPLT_CB_ID   OSPI Tx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_STATUS_MATCH_CB_ID   OSPI Status Match Callback ID
  *          @arg @ref HAL_OSPI_TIMEOUT_CB_ID        OSPI Timeout Callback ID
  *          @arg @ref HAL_OSPI_MSP_INIT_CB_ID       OSPI MspInit callback ID
  *          @arg @ref HAL_OSPI_MSP_DEINIT_CB_ID     OSPI MspDeInit callback ID
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_OSPI_RegisterCallback (OSPI_HandleTypeDef *hospi, HAL_OSPI_CallbackIDTypeDef CallbackID, pOSPI_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    /* Update the error code */
    hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }

  if(hospi->State == HAL_OSPI_STATE_READY)
  {
    switch (CallbackID)
    {
    case  HAL_OSPI_ERROR_CB_ID :
      hospi->ErrorCallback = pCallback;
      break;
    case HAL_OSPI_ABORT_CB_ID :
      hospi->AbortCpltCallback = pCallback;
      break;
    case HAL_OSPI_FIFO_THRESHOLD_CB_ID :
      hospi->FifoThresholdCallback = pCallback;
      break;
    case HAL_OSPI_CMD_CPLT_CB_ID :
      hospi->CmdCpltCallback = pCallback;
      break;
    case HAL_OSPI_RX_CPLT_CB_ID :
      hospi->RxCpltCallback = pCallback;
      break;
    case HAL_OSPI_TX_CPLT_CB_ID :
      hospi->TxCpltCallback = pCallback;
      break;
    case HAL_OSPI_RX_HALF_CPLT_CB_ID :
      hospi->RxHalfCpltCallback = pCallback;
      break;
    case HAL_OSPI_TX_HALF_CPLT_CB_ID :
      hospi->TxHalfCpltCallback = pCallback;
      break;
    case HAL_OSPI_STATUS_MATCH_CB_ID :
      hospi->StatusMatchCallback = pCallback;
      break;
    case HAL_OSPI_TIMEOUT_CB_ID :
      hospi->TimeOutCallback = pCallback;
      break;
    case HAL_OSPI_MSP_INIT_CB_ID :
      hospi->MspInitCallback = pCallback;
      break;
    case HAL_OSPI_MSP_DEINIT_CB_ID :
      hospi->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hospi->State == HAL_OSPI_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_OSPI_MSP_INIT_CB_ID :
      hospi->MspInitCallback = pCallback;
      break;
    case HAL_OSPI_MSP_DEINIT_CB_ID :
      hospi->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Unregister a User OSPI Callback
  *         OSPI Callback is redirected to the weak (surcharged) predefined callback
  * @param hospi : OSPI handle
  * @param CallbackID : ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OSPI_ERROR_CB_ID          OSPI Error Callback ID
  *          @arg @ref HAL_OSPI_ABORT_CB_ID          OSPI Abort Callback ID
  *          @arg @ref HAL_OSPI_FIFO_THRESHOLD_CB_ID OSPI FIFO Threshold Callback ID
  *          @arg @ref HAL_OSPI_CMD_CPLT_CB_ID       OSPI Command Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_CPLT_CB_ID        OSPI Rx Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_CPLT_CB_ID        OSPI Tx Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_HALF_CPLT_CB_ID   OSPI Rx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_HALF_CPLT_CB_ID   OSPI Tx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_STATUS_MATCH_CB_ID   OSPI Status Match Callback ID
  *          @arg @ref HAL_OSPI_TIMEOUT_CB_ID        OSPI Timeout Callback ID
  *          @arg @ref HAL_OSPI_MSP_INIT_CB_ID       OSPI MspInit callback ID
  *          @arg @ref HAL_OSPI_MSP_DEINIT_CB_ID     OSPI MspDeInit callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_OSPI_UnRegisterCallback (OSPI_HandleTypeDef *hospi, HAL_OSPI_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(hospi->State == HAL_OSPI_STATE_READY)
  {
    switch (CallbackID)
    {
    case  HAL_OSPI_ERROR_CB_ID :
      hospi->ErrorCallback = HAL_OSPI_ErrorCallback;
      break;
    case HAL_OSPI_ABORT_CB_ID :
      hospi->AbortCpltCallback = HAL_OSPI_AbortCpltCallback;
      break;
    case HAL_OSPI_FIFO_THRESHOLD_CB_ID :
      hospi->FifoThresholdCallback = HAL_OSPI_FifoThresholdCallback;
      break;
    case HAL_OSPI_CMD_CPLT_CB_ID :
      hospi->CmdCpltCallback = HAL_OSPI_CmdCpltCallback;
      break;
    case HAL_OSPI_RX_CPLT_CB_ID :
      hospi->RxCpltCallback = HAL_OSPI_RxCpltCallback;
      break;
    case HAL_OSPI_TX_CPLT_CB_ID :
      hospi->TxCpltCallback = HAL_OSPI_TxCpltCallback;
      break;
    case HAL_OSPI_RX_HALF_CPLT_CB_ID :
      hospi->RxHalfCpltCallback = HAL_OSPI_RxHalfCpltCallback;
      break;
    case HAL_OSPI_TX_HALF_CPLT_CB_ID :
      hospi->TxHalfCpltCallback = HAL_OSPI_TxHalfCpltCallback;
      break;
    case HAL_OSPI_STATUS_MATCH_CB_ID :
      hospi->StatusMatchCallback = HAL_OSPI_StatusMatchCallback;
      break;
    case HAL_OSPI_TIMEOUT_CB_ID :
      hospi->TimeOutCallback = HAL_OSPI_TimeOutCallback;
      break;
    case HAL_OSPI_MSP_INIT_CB_ID :
      hospi->MspInitCallback = HAL_OSPI_MspInit;
      break;
    case HAL_OSPI_MSP_DEINIT_CB_ID :
      hospi->MspDeInitCallback = HAL_OSPI_MspDeInit;
      break;
    default :
      /* Update the error code */
      hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hospi->State == HAL_OSPI_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_OSPI_MSP_INIT_CB_ID :
      hospi->MspInitCallback = HAL_OSPI_MspInit;
      break;
    case HAL_OSPI_MSP_DEINIT_CB_ID :
      hospi->MspDeInitCallback = HAL_OSPI_MspDeInit;
      break;
    default :
      /* Update the error code */
      hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  return status;
}
#endif

/**
  * @}
  */

/** @defgroup OSPI_Exported_Functions_Group3 Peripheral Control and State functions
  *  @brief   OSPI control and State functions
  *
@verbatim
 ===============================================================================
                  ##### Peripheral Control and State functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to :
      (+) Check in run-time the state of the driver.
      (+) Check the error code set during last operation.
      (+) Abort any operation.
      (+) Manage the Fifo threshold.
      (+) Configure the timeout duration used in the driver.

@endverbatim
  * @{
  */

/**
* @brief  Abort the current transmission.
* @param  hospi : OSPI handle
* @retval HAL status
*/
HAL_StatusTypeDef HAL_OSPI_Abort(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t state;
  uint32_t tickstart = HAL_GetTick();

  /* Check if the state is in one of the busy or configured states */
  state = hospi->State;
  if (((state & OSPI_BUSY_STATE_MASK) != 0U) || ((state & OSPI_CFG_STATE_MASK) != 0U))
  {
    /* Check if the DMA is enabled */
    if ((hospi->Instance->CR & OCTOSPI_CR_DMAEN) != 0U)
    {
      /* Disable the DMA transfer on the OctoSPI side */
      CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

      /* Disable the DMA transfer on the DMA side */
      status = HAL_DMA_Abort(hospi->hdma);
      if (status != HAL_OK)
      {
        hospi->ErrorCode = HAL_OSPI_ERROR_DMA;
      }
    }

    if (__HAL_OSPI_GET_FLAG(hospi, HAL_OSPI_FLAG_BUSY) != RESET)
    {
      /* Perform an abort of the OctoSPI */
      SET_BIT(hospi->Instance->CR, OCTOSPI_CR_ABORT);

      /* Wait until the transfer complete flag is set to go back in idle state */
      status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, hospi->Timeout);

      if (status == HAL_OK)
      {
        /* Clear transfer complete flag */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

        /* Wait until the busy flag is reset to go back in idle state */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

        if (status == HAL_OK)
        {
          /* Update state */
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
* @brief  Abort the current transmission (non-blocking function)
* @param  hospi : OSPI handle
* @retval HAL status
*/
HAL_StatusTypeDef HAL_OSPI_Abort_IT(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t state;

  /* Check if the state is in one of the busy or configured states */
  state = hospi->State;
  if (((state & OSPI_BUSY_STATE_MASK) != 0U) || ((state & OSPI_CFG_STATE_MASK) != 0U))
  {
    /* Disable all interrupts */
    __HAL_OSPI_DISABLE_IT(hospi, (HAL_OSPI_IT_TO | HAL_OSPI_IT_SM | HAL_OSPI_IT_FT | HAL_OSPI_IT_TC | HAL_OSPI_IT_TE));

    /* Update state */
    hospi->State = HAL_OSPI_STATE_ABORT;

    /* Check if the DMA is enabled */
    if ((hospi->Instance->CR & OCTOSPI_CR_DMAEN) != 0U)
    {
      /* Disable the DMA transfer on the OctoSPI side */
      CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

      /* Disable the DMA transfer on the DMA side */
      hospi->hdma->XferAbortCallback = OSPI_DMAAbortCplt;
      if (HAL_DMA_Abort_IT(hospi->hdma) != HAL_OK)
      {
        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* Abort callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->AbortCpltCallback(hospi);
#else
        HAL_OSPI_AbortCpltCallback(hospi);
#endif
      }
    }
    else
    {
      if (__HAL_OSPI_GET_FLAG(hospi, HAL_OSPI_FLAG_BUSY) != RESET)
      {
        /* Clear transfer complete flag */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

        /* Enable the transfer complete interrupts */
        __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC);

        /* Perform an abort of the OctoSPI */
        SET_BIT(hospi->Instance->CR, OCTOSPI_CR_ABORT);
      }
      else
      {
        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* Abort callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->AbortCpltCallback(hospi);
#else
        HAL_OSPI_AbortCpltCallback(hospi);
#endif
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/** @brief  Set OSPI Fifo threshold.
  * @param  hospi     : OSPI handle.
  * @param  Threshold : Threshold of the Fifo.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_SetFifoThreshold(OSPI_HandleTypeDef *hospi, uint32_t Threshold)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the state */
  if ((hospi->State & OSPI_BUSY_STATE_MASK) == 0U)
  {
    /* Synchronize initialization structure with the new fifo threshold value */
    hospi->Init.FifoThreshold = Threshold;

    /* Configure new fifo threshold */
    MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FTHRES, ((hospi->Init.FifoThreshold-1U) << OCTOSPI_CR_FTHRES_Pos));

  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/** @brief  Get OSPI Fifo threshold.
  * @param  hospi : OSPI handle.
  * @retval Fifo threshold
  */
uint32_t HAL_OSPI_GetFifoThreshold(OSPI_HandleTypeDef *hospi)
{
  return ((READ_BIT(hospi->Instance->CR, OCTOSPI_CR_FTHRES) >> OCTOSPI_CR_FTHRES_Pos) + 1U);
}

/** @brief Set OSPI timeout.
  * @param  hospi   : OSPI handle.
  * @param  Timeout : Timeout for the memory access.
  * @retval None
  */
HAL_StatusTypeDef HAL_OSPI_SetTimeout(OSPI_HandleTypeDef *hospi, uint32_t Timeout)
{
  hospi->Timeout = Timeout;
  return HAL_OK;
}

/**
* @brief  Return the OSPI error code.
* @param  hospi : OSPI handle
* @retval OSPI Error Code
*/
uint32_t HAL_OSPI_GetError(OSPI_HandleTypeDef *hospi)
{
  return hospi->ErrorCode;
}

/**
  * @brief  Return the OSPI handle state.
  * @param  hospi : OSPI handle
  * @retval HAL state
  */
uint32_t HAL_OSPI_GetState(OSPI_HandleTypeDef *hospi)
{
  /* Return OSPI handle state */
  return hospi->State;
}

/**
  * @}
  */

/** @defgroup OSPI_Exported_Functions_Group4 IO Manager configuration function
  *  @brief   OSPI IO Manager configuration function
  *
@verbatim
 ===============================================================================
                  ##### IO Manager configuration function #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to :
      (+) Configure the IO manager.

@endverbatim
  * @{
  */

/**
  * @brief  Configure the OctoSPI IO manager.
  * @param  hospi   : OSPI handle
  * @param  cfg     : Configuration of the IO Manager for the instance
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPIM_Config(OSPI_HandleTypeDef *hospi, OSPIM_CfgTypeDef *cfg, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t instance;
  uint8_t index, ospi_enabled = 0U, other_instance;
  OSPIM_CfgTypeDef IOM_cfg[OSPI_NB_INSTANCE];

  /* Prevent unused argument(s) compilation warning */
  UNUSED(Timeout);

  /* Check the parameters of the OctoSPI IO Manager configuration structure */
  assert_param(IS_OSPIM_PORT(cfg->ClkPort));
  assert_param(IS_OSPIM_PORT(cfg->DQSPort));
  assert_param(IS_OSPIM_PORT(cfg->NCSPort));
  assert_param(IS_OSPIM_IO_PORT(cfg->IOLowPort));
  assert_param(IS_OSPIM_IO_PORT(cfg->IOHighPort));

  if (hospi->Instance == OCTOSPI1)
  {
    instance = 0U;
    other_instance = 1U;
  }
  else
  {
    instance = 1U;
    other_instance = 0U;
  }

  /**************** Get current configuration of the instances ****************/
  for (index = 0U; index < OSPI_NB_INSTANCE; index++)
  {
    if (OSPIM_GetConfig(index+1U, &(IOM_cfg[index])) != HAL_OK)
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
    }
  }

  if (status == HAL_OK)
  {
    /********** Disable both OctoSPI to configure OctoSPI IO Manager **********/
    if ((OCTOSPI1->CR & OCTOSPI_CR_EN) != 0U)
    {
      CLEAR_BIT(OCTOSPI1->CR, OCTOSPI_CR_EN);
      ospi_enabled |= 0x1U;
    }
    if ((OCTOSPI2->CR & OCTOSPI_CR_EN) != 0U)
    {
      CLEAR_BIT(OCTOSPI2->CR, OCTOSPI_CR_EN);
      ospi_enabled |= 0x2U;
    }

    /***************** Deactivation of previous configuration *****************/
    if (IOM_cfg[instance].ClkPort != 0U)
    {
      CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[instance].ClkPort-1U)],                          OCTOSPIM_PCR_CLKEN);
      CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[instance].DQSPort-1U)],                          OCTOSPIM_PCR_DQSEN);
      CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[instance].NCSPort-1U)],                          OCTOSPIM_PCR_NCSEN);
      CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[instance].IOLowPort-1U)& OSPI_IOM_PORT_MASK)],  OCTOSPIM_PCR_IOLEN);
      CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[instance].IOHighPort-1U)& OSPI_IOM_PORT_MASK)], OCTOSPIM_PCR_IOHEN);
    }

    /********************* Deactivation of other instance *********************/
    if ((cfg->ClkPort == IOM_cfg[other_instance].ClkPort) || (cfg->DQSPort == IOM_cfg[other_instance].DQSPort)     ||
        (cfg->NCSPort == IOM_cfg[other_instance].NCSPort) || (cfg->IOLowPort == IOM_cfg[other_instance].IOLowPort) ||
        (cfg->IOHighPort == IOM_cfg[other_instance].IOHighPort))
    {
      CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].ClkPort-1U)],                          OCTOSPIM_PCR_CLKEN);
      CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].DQSPort-1U)],                          OCTOSPIM_PCR_DQSEN);
      CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].NCSPort-1U)],                          OCTOSPIM_PCR_NCSEN);
      CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[other_instance].IOLowPort-1U)& OSPI_IOM_PORT_MASK)],  OCTOSPIM_PCR_IOLEN);
      CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[other_instance].IOHighPort-1U)& OSPI_IOM_PORT_MASK)], OCTOSPIM_PCR_IOHEN);
    }

    /******************** Activation of new configuration *********************/
    MODIFY_REG(OCTOSPIM->PCR[(cfg->ClkPort-1U)], (OCTOSPIM_PCR_CLKEN | OCTOSPIM_PCR_CLKSRC), (OCTOSPIM_PCR_CLKEN | (instance << OCTOSPIM_PCR_CLKSRC_Pos)));
    MODIFY_REG(OCTOSPIM->PCR[(cfg->DQSPort-1U)], (OCTOSPIM_PCR_DQSEN | OCTOSPIM_PCR_DQSSRC), (OCTOSPIM_PCR_DQSEN | (instance << OCTOSPIM_PCR_DQSSRC_Pos)));
    MODIFY_REG(OCTOSPIM->PCR[(cfg->NCSPort-1U)], (OCTOSPIM_PCR_NCSEN | OCTOSPIM_PCR_NCSSRC), (OCTOSPIM_PCR_NCSEN | (instance << OCTOSPIM_PCR_NCSSRC_Pos)));

    if ((cfg->IOLowPort & OCTOSPIM_PCR_IOLEN) != 0U)
    {
      MODIFY_REG(OCTOSPIM->PCR[((cfg->IOLowPort-1U)& OSPI_IOM_PORT_MASK)], (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC),
                 (OCTOSPIM_PCR_IOLEN | (instance << (OCTOSPIM_PCR_IOLSRC_Pos+1U))));
    }
    else
    {
      MODIFY_REG(OCTOSPIM->PCR[((cfg->IOLowPort-1U)& OSPI_IOM_PORT_MASK)], (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC),
                 (OCTOSPIM_PCR_IOHEN | (instance << (OCTOSPIM_PCR_IOHSRC_Pos+1U))));
    }

    if ((cfg->IOHighPort & OCTOSPIM_PCR_IOLEN) != 0U)
    {
      MODIFY_REG(OCTOSPIM->PCR[((cfg->IOHighPort-1U)& OSPI_IOM_PORT_MASK)], (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC),
                 (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC_0 | (instance << (OCTOSPIM_PCR_IOLSRC_Pos+1U))));
    }
    else
    {
      MODIFY_REG(OCTOSPIM->PCR[((cfg->IOHighPort-1U)& OSPI_IOM_PORT_MASK)], (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC),
                 (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC_0 | (instance << (OCTOSPIM_PCR_IOHSRC_Pos+1U))));
    }

    /******* Re-enable both OctoSPI after configure OctoSPI IO Manager ********/
    if ((ospi_enabled & 0x1U) != 0U)
    {
      SET_BIT(OCTOSPI1->CR, OCTOSPI_CR_EN);
    }
    if ((ospi_enabled & 0x2U) != 0U)
    {
      SET_BIT(OCTOSPI2->CR, OCTOSPI_CR_EN);
    }
  }

  /* Return function status */
  return status;
}

/**
  * @}
  */

/**
  @cond 0
  */
/**
  * @brief  DMA OSPI process complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMACplt(DMA_HandleTypeDef *hdma)
{
  OSPI_HandleTypeDef* hospi = ( OSPI_HandleTypeDef* )(hdma->Parent);
  hospi->XferCount = 0;

  /* Disable the DMA transfer on the OctoSPI side */
  CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

  /* Disable the DMA channel */
  __HAL_DMA_DISABLE(hdma);

  /* Enable the OSPI transfer complete Interrupt */
  __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC);
}

/**
  * @brief  DMA OSPI process half complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMAHalfCplt(DMA_HandleTypeDef *hdma)
{
  OSPI_HandleTypeDef* hospi = ( OSPI_HandleTypeDef* )(hdma->Parent);
  hospi->XferCount = (hospi->XferCount >> 1);

  if (hospi->State == HAL_OSPI_STATE_BUSY_RX)
  {
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->RxHalfCpltCallback(hospi);
#else
    HAL_OSPI_RxHalfCpltCallback(hospi);
#endif
  }
  else
  {
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->TxHalfCpltCallback(hospi);
#else
    HAL_OSPI_TxHalfCpltCallback(hospi);
#endif
  }
}

/**
  * @brief  DMA OSPI communication error callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMAError(DMA_HandleTypeDef *hdma)
{
  OSPI_HandleTypeDef* hospi = ( OSPI_HandleTypeDef* )(hdma->Parent);
  hospi->XferCount = 0;
  hospi->ErrorCode = HAL_OSPI_ERROR_DMA;

  /* Disable the DMA transfer on the OctoSPI side */
  CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

  /* Abort the OctoSPI */
  if (HAL_OSPI_Abort_IT(hospi) != HAL_OK)
  {
    /* Disable the interrupts */
    __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

    /* Update state */
    hospi->State = HAL_OSPI_STATE_READY;

    /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->ErrorCallback(hospi);
#else
    HAL_OSPI_ErrorCallback(hospi);
#endif
  }
}

/**
  * @brief  DMA OSPI abort complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMAAbortCplt(DMA_HandleTypeDef *hdma)
{
  OSPI_HandleTypeDef* hospi = ( OSPI_HandleTypeDef* )(hdma->Parent);
  hospi->XferCount = 0;

  /* Check the state */
  if (hospi->State == HAL_OSPI_STATE_ABORT)
  {
    /* DMA abort called by OctoSPI abort */
    if (__HAL_OSPI_GET_FLAG(hospi, HAL_OSPI_FLAG_BUSY) != RESET)
    {
      /* Clear transfer complete flag */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

      /* Enable the transfer complete interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC);

      /* Perform an abort of the OctoSPI */
      SET_BIT(hospi->Instance->CR, OCTOSPI_CR_ABORT);
    }
    else
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;

      /* Abort callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
      hospi->AbortCpltCallback(hospi);
#else
      HAL_OSPI_AbortCpltCallback(hospi);
#endif
    }
  }
  else
  {
    /* DMA abort called due to a transfer error interrupt */
    /* Update state */
    hospi->State = HAL_OSPI_STATE_READY;

    /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->ErrorCallback(hospi);
#else
    HAL_OSPI_ErrorCallback(hospi);
#endif
  }
}

/**
  * @brief  Wait for a flag state until timeout.
  * @param  hospi     : OSPI handle
  * @param  Flag      : Flag checked
  * @param  State     : Value of the flag expected
  * @param  Timeout   : Duration of the timeout
  * @param  Tickstart : Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef OSPI_WaitFlagStateUntilTimeout(OSPI_HandleTypeDef *hospi, uint32_t Flag,
                                                        FlagStatus State, uint32_t Tickstart, uint32_t Timeout)
{
  /* Wait until flag is in expected state */
  while((__HAL_OSPI_GET_FLAG(hospi, Flag)) != State)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if(((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hospi->State     = HAL_OSPI_STATE_ERROR;
        hospi->ErrorCode |= HAL_OSPI_ERROR_TIMEOUT;

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/**
  * @brief  Configure the registers for the regular command mode.
  * @param  hospi : OSPI handle
  * @param  cmd   : structure that contains the command configuration information
  * @retval HAL status
  */
static HAL_StatusTypeDef OSPI_ConfigCmd(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd)
{
  HAL_StatusTypeDef status = HAL_OK;
  __IO uint32_t *ccr_reg, *tcr_reg, *ir_reg, *abr_reg;

  /* Re-initialize the value of the functional mode */
  MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, 0U);

  /* Configure the flash ID */
  if (hospi->Init.DualQuad == HAL_OSPI_DUALQUAD_DISABLE)
  {
    MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FSEL, cmd->FlashId);
  }

  if (cmd->OperationType == HAL_OSPI_OPTYPE_WRITE_CFG)
  {
    ccr_reg = &(hospi->Instance->WCCR);
    tcr_reg = &(hospi->Instance->WTCR);
    ir_reg  = &(hospi->Instance->WIR);
    abr_reg = &(hospi->Instance->WABR);
  }
  else
  {
    ccr_reg = &(hospi->Instance->CCR);
    tcr_reg = &(hospi->Instance->TCR);
    ir_reg  = &(hospi->Instance->IR);
    abr_reg = &(hospi->Instance->ABR);
  }

  /* Configure the CCR register with DQS and SIOO modes */
  *ccr_reg = (cmd->DQSMode | cmd->SIOOMode);

  if (cmd->AlternateBytesMode != HAL_OSPI_ALTERNATE_BYTES_NONE)
  {
    /* Configure the ABR register with alternate bytes value */
    *abr_reg = cmd->AlternateBytes;

    /* Configure the CCR register with alternate bytes communication parameters */
    MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_ABMODE | OCTOSPI_CCR_ABDTR | OCTOSPI_CCR_ABSIZE),
                           (cmd->AlternateBytesMode | cmd->AlternateBytesDtrMode | cmd->AlternateBytesSize));
  }

  /* Configure the TCR register with the number of dummy cycles */
  MODIFY_REG((*tcr_reg), OCTOSPI_TCR_DCYC, cmd->DummyCycles);

  if (cmd->DataMode != HAL_OSPI_DATA_NONE)
  {
    if (cmd->OperationType == HAL_OSPI_OPTYPE_COMMON_CFG)
    {
      /* Configure the DLR register with the number of data */
      hospi->Instance->DLR = (cmd->NbData - 1U);
    }
  }

  if (cmd->InstructionMode != HAL_OSPI_INSTRUCTION_NONE)
  {
    if (cmd->AddressMode != HAL_OSPI_ADDRESS_NONE)
    {
      if (cmd->DataMode != HAL_OSPI_DATA_NONE)
      {
        /* ---- Command with instruction, address and data ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE  | OCTOSPI_CCR_IDTR  | OCTOSPI_CCR_ISIZE  |
                                OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE |
                                OCTOSPI_CCR_DMODE  | OCTOSPI_CCR_DDTR),
                               (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize |
                                cmd->AddressMode     | cmd->AddressDtrMode     | cmd->AddressSize     |
                                cmd->DataMode        | cmd->DataDtrMode));
      }
      else
      {
        /* ---- Command with instruction and address ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE  | OCTOSPI_CCR_IDTR  | OCTOSPI_CCR_ISIZE  |
                                OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE),
                               (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize |
                                cmd->AddressMode     | cmd->AddressDtrMode     | cmd->AddressSize));

        /* The DHQC bit is linked with DDTR bit which should be activated */
        if ((hospi->Init.DelayHoldQuarterCycle == HAL_OSPI_DHQC_ENABLE) &&
            (cmd->InstructionDtrMode == HAL_OSPI_INSTRUCTION_DTR_ENABLE))
        {
          MODIFY_REG((*ccr_reg), OCTOSPI_CCR_DDTR, HAL_OSPI_DATA_DTR_ENABLE);
        }
      }

      /* Configure the IR register with the instruction value */
      *ir_reg = cmd->Instruction;

      /* Configure the AR register with the address value */
      hospi->Instance->AR = cmd->Address;
    }
    else
    {
      if (cmd->DataMode != HAL_OSPI_DATA_NONE)
      {
        /* ---- Command with instruction and data ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE | OCTOSPI_CCR_IDTR | OCTOSPI_CCR_ISIZE |
                                OCTOSPI_CCR_DMODE | OCTOSPI_CCR_DDTR),
                               (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize |
                                cmd->DataMode        | cmd->DataDtrMode));
      }
      else
      {
        /* ---- Command with only instruction ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE | OCTOSPI_CCR_IDTR | OCTOSPI_CCR_ISIZE),
                               (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize));

        /* The DHQC bit is linked with DDTR bit which should be activated */
        if ((hospi->Init.DelayHoldQuarterCycle == HAL_OSPI_DHQC_ENABLE) &&
            (cmd->InstructionDtrMode == HAL_OSPI_INSTRUCTION_DTR_ENABLE))
        {
          MODIFY_REG((*ccr_reg), OCTOSPI_CCR_DDTR, HAL_OSPI_DATA_DTR_ENABLE);
        }
      }

      /* Configure the IR register with the instruction value */
      *ir_reg = cmd->Instruction;

    }
  }
  else
  {
    if (cmd->AddressMode != HAL_OSPI_ADDRESS_NONE)
    {
      if (cmd->DataMode != HAL_OSPI_DATA_NONE)
      {
        /* ---- Command with address and data ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE |
                                OCTOSPI_CCR_DMODE  | OCTOSPI_CCR_DDTR),
                               (cmd->AddressMode | cmd->AddressDtrMode | cmd->AddressSize     |
                                cmd->DataMode    | cmd->DataDtrMode));
      }
      else
      {
        /* ---- Command with only address ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE),
                               (cmd->AddressMode | cmd->AddressDtrMode | cmd->AddressSize));
      }

      /* Configure the AR register with the instruction value */
      hospi->Instance->AR = cmd->Address;
    }
    else
    {
      /* ---- Invalid command configuration (no instruction, no address) ---- */
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Get the current IOM configuration for an OctoSPI instance.
  * @param  instance_nb : number of the instance
  * @param  cfg         : configuration of the IO Manager for the instance
  * @retval HAL status
  */
static HAL_StatusTypeDef OSPIM_GetConfig(uint8_t instance_nb, OSPIM_CfgTypeDef *cfg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t reg, value = 0U;
  uint32_t index;

  if ((instance_nb == 0U) || (instance_nb > OSPI_NB_INSTANCE) || (cfg == NULL))
  {
    /* Invalid parameter -> error returned */
    status = HAL_ERROR;
  }
  else
  {
    /* Initialize the structure */
    cfg->ClkPort    = 0U;
    cfg->DQSPort    = 0U;
    cfg->NCSPort    = 0U;
    cfg->IOLowPort  = 0U;
    cfg->IOHighPort = 0U;

    if (instance_nb == 2U)
    {
      value = (OCTOSPIM_PCR_CLKSRC | OCTOSPIM_PCR_DQSSRC | OCTOSPIM_PCR_NCSSRC | OCTOSPIM_PCR_IOLSRC_1 | OCTOSPIM_PCR_IOHSRC_1);
    }

    /* Get the information about the instance */
    for (index = 0U; index < OSPI_IOM_NB_PORTS; index ++)
    {
      reg = OCTOSPIM->PCR[index];

      if ((reg & OCTOSPIM_PCR_CLKEN) != 0U)
      {
        /* The clock is enabled on this port */
        if ((reg & OCTOSPIM_PCR_CLKSRC) == (value & OCTOSPIM_PCR_CLKSRC))
        {
          /* The clock correspond to the instance passed as parameter */
          cfg->ClkPort = index+1U;
        }
      }

      if ((reg & OCTOSPIM_PCR_DQSEN) != 0U)
      {
        /* The DQS is enabled on this port */
        if ((reg & OCTOSPIM_PCR_DQSSRC) == (value & OCTOSPIM_PCR_DQSSRC))
        {
          /* The DQS correspond to the instance passed as parameter */
          cfg->DQSPort = index+1U;
        }
      }

      if ((reg & OCTOSPIM_PCR_NCSEN) != 0U)
      {
        /* The nCS is enabled on this port */
        if ((reg & OCTOSPIM_PCR_NCSSRC) == (value & OCTOSPIM_PCR_NCSSRC))
        {
          /* The nCS correspond to the instance passed as parameter */
          cfg->NCSPort = index+1U;
        }
      }

      if ((reg & OCTOSPIM_PCR_IOLEN) != 0U)
      {
        /* The IO Low is enabled on this port */
        if ((reg & OCTOSPIM_PCR_IOLSRC_1) == (value & OCTOSPIM_PCR_IOLSRC_1))
        {
          /* The IO Low correspond to the instance passed as parameter */
          if ((reg & OCTOSPIM_PCR_IOLSRC_0) == 0U)
          {
            cfg->IOLowPort = (OCTOSPIM_PCR_IOLEN | (index+1U));
          }
          else
          {
            cfg->IOLowPort = (OCTOSPIM_PCR_IOHEN | (index+1U));
          }
        }
      }

      if ((reg & OCTOSPIM_PCR_IOHEN) != 0U)
      {
        /* The IO High is enabled on this port */
        if ((reg & OCTOSPIM_PCR_IOHSRC_1) == (value & OCTOSPIM_PCR_IOHSRC_1))
        {
          /* The IO High correspond to the instance passed as parameter */
          if ((reg & OCTOSPIM_PCR_IOHSRC_0) == 0U)
          {
            cfg->IOHighPort = (OCTOSPIM_PCR_IOLEN | (index+1U));
          }
          else
          {
            cfg->IOHighPort = (OCTOSPIM_PCR_IOHEN | (index+1U));
          }
        }
      }
    }
  }

  /* Return function status */
  return status;
}

/**
  @endcond
  */

/**
  * @}
  */

#endif /* HAL_OSPI_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

#endif /* OCTOSPI || OCTOSPI1 || OCTOSPI2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
