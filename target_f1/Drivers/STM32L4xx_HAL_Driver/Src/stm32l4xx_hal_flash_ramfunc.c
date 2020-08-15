/**
  ******************************************************************************
  * @file    stm32l4xx_hal_flash_ramfunc.c
  * @author  MCD Application Team
  * @brief   FLASH RAMFUNC driver.
  *          This file provides a Flash firmware functions which should be
  *          executed from internal SRAM
  *            + FLASH HalfPage Programming
  *            + FLASH Power Down in Run mode
  *
  *  @verbatim
  ==============================================================================
                   ##### Flash RAM functions #####
  ==============================================================================

    *** ARM Compiler ***
    --------------------
    [..] RAM functions are defined using the toolchain options.
         Functions that are executed in RAM should reside in a separate
         source module. Using the 'Options for File' dialog you can simply change
         the 'Code / Const' area of a module to a memory space in physical RAM.
         Available memory areas are declared in the 'Target' tab of the
         Options for Target' dialog.

    *** ICCARM Compiler ***
    -----------------------
    [..] RAM functions are defined using a specific toolchain keyword "__ramfunc".

    *** GNU Compiler ***
    --------------------
    [..] RAM functions are defined using a specific toolchain attribute
         "__attribute__((section(".RamFunc")))".

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

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup FLASH_RAMFUNC FLASH_RAMFUNC
  * @brief FLASH functions executed from RAM
  * @{
  */

#ifdef HAL_FLASH_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern FLASH_ProcessTypeDef pFlash;

/* Private function prototypes -----------------------------------------------*/
/* Exported functions -------------------------------------------------------*/

/** @defgroup FLASH_RAMFUNC_Exported_Functions FLASH in RAM function Exported Functions
  * @{
  */

/** @defgroup FLASH_RAMFUNC_Exported_Functions_Group1 Peripheral features functions
 *  @brief   Data transfers functions
 *
@verbatim
 ===============================================================================
                      ##### ramfunc functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions that should be executed from RAM.

@endverbatim
  * @{
  */

/**
  * @brief   Enable the Power down in Run Mode
  * @note    This function should be called and executed from SRAM memory
  * @retval None
  */
__RAM_FUNC HAL_FLASHEx_EnableRunPowerDown(void)
{
  /* Enable the Power Down in Run mode*/
  __HAL_FLASH_POWER_DOWN_ENABLE();

  return HAL_OK;

}

/**
  * @brief   Disable the Power down in Run Mode
  * @note    This function should be called and executed from SRAM memory
  * @retval None
  */
__RAM_FUNC HAL_FLASHEx_DisableRunPowerDown(void)
{
  /* Disable the Power Down in Run mode*/
  __HAL_FLASH_POWER_DOWN_DISABLE();

  return HAL_OK;
}

#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
/**
  * @brief  Program the FLASH DBANK User Option Byte.
  *
  * @note   To configure the user option bytes, the option lock bit OPTLOCK must
  *         be cleared with the call of the HAL_FLASH_OB_Unlock() function.
  * @note   To modify the DBANK option byte, no PCROP region should be defined.
  *         To deactivate PCROP, user should perform RDP changing
  *
  * @param  DBankConfig: The FLASH DBANK User Option Byte value.
  *          This parameter  can be one of the following values:
  *            @arg OB_DBANK_128_BITS: Single-bank with 128-bits data
  *            @arg OB_DBANK_64_BITS: Dual-bank with 64-bits data
  *
  * @retval HAL status
  */
__RAM_FUNC HAL_FLASHEx_OB_DBankConfig(uint32_t DBankConfig)
{
  register uint32_t count, reg;
  HAL_StatusTypeDef status = HAL_ERROR;

  /* Process Locked */
  __HAL_LOCK(&pFlash);

  /* Check if the PCROP is disabled */
  reg = FLASH->PCROP1SR;
  if (reg > FLASH->PCROP1ER)
  {
    reg = FLASH->PCROP2SR;
    if (reg > FLASH->PCROP2ER)
    {
      /* Disable Flash prefetch */
      __HAL_FLASH_PREFETCH_BUFFER_DISABLE();

      if (READ_BIT(FLASH->ACR, FLASH_ACR_ICEN) != 0U)
      {
        /* Disable Flash instruction cache */
        __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

        /* Flush Flash instruction cache */
        __HAL_FLASH_INSTRUCTION_CACHE_RESET();
      }

      if (READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
      {
        /* Disable Flash data cache */
        __HAL_FLASH_DATA_CACHE_DISABLE();

        /* Flush Flash data cache */
        __HAL_FLASH_DATA_CACHE_RESET();
      }

      /* Disable WRP zone 1 of 1st bank if needed */
      reg = FLASH->WRP1AR;
      if (((reg & FLASH_WRP1AR_WRP1A_STRT) >> FLASH_WRP1AR_WRP1A_STRT_Pos) <=
          ((reg & FLASH_WRP1AR_WRP1A_END) >> FLASH_WRP1AR_WRP1A_END_Pos))
      {
        MODIFY_REG(FLASH->WRP1AR, (FLASH_WRP1AR_WRP1A_STRT | FLASH_WRP1AR_WRP1A_END), FLASH_WRP1AR_WRP1A_STRT);
      }

      /* Disable WRP zone 2 of 1st bank if needed */
      reg = FLASH->WRP1BR;
      if (((reg & FLASH_WRP1BR_WRP1B_STRT) >> FLASH_WRP1BR_WRP1B_STRT_Pos) <=
          ((reg & FLASH_WRP1BR_WRP1B_END) >> FLASH_WRP1BR_WRP1B_END_Pos))
      {
        MODIFY_REG(FLASH->WRP1BR, (FLASH_WRP1BR_WRP1B_STRT | FLASH_WRP1BR_WRP1B_END), FLASH_WRP1BR_WRP1B_STRT);
      }

      /* Disable WRP zone 1 of 2nd bank if needed */
      reg = FLASH->WRP2AR;
      if (((reg & FLASH_WRP2AR_WRP2A_STRT) >> FLASH_WRP2AR_WRP2A_STRT_Pos) <=
          ((reg & FLASH_WRP2AR_WRP2A_END) >> FLASH_WRP2AR_WRP2A_END_Pos))
      {
        MODIFY_REG(FLASH->WRP2AR, (FLASH_WRP2AR_WRP2A_STRT | FLASH_WRP2AR_WRP2A_END), FLASH_WRP2AR_WRP2A_STRT);
      }

      /* Disable WRP zone 2 of 2nd bank if needed */
      reg = FLASH->WRP2BR;
      if (((reg & FLASH_WRP2BR_WRP2B_STRT) >> FLASH_WRP2BR_WRP2B_STRT_Pos) <=
          ((reg & FLASH_WRP2BR_WRP2B_END) >> FLASH_WRP2BR_WRP2B_END_Pos))
      {
        MODIFY_REG(FLASH->WRP2BR, (FLASH_WRP2BR_WRP2B_STRT | FLASH_WRP2BR_WRP2B_END), FLASH_WRP2BR_WRP2B_STRT);
      }

      /* Modify the DBANK user option byte */
      MODIFY_REG(FLASH->OPTR, FLASH_OPTR_DBANK, DBankConfig);

      /* Set OPTSTRT Bit */
      SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

      /* Wait for last operation to be completed */
      /* 8 is the number of required instruction cycles for the below loop statement (timeout expressed in ms) */
      count = FLASH_TIMEOUT_VALUE * (SystemCoreClock / 8U / 1000U);
      do
      {
        if (count == 0U)
        {
          break;
        }
        count--;
      } while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);

      /* If the option byte program operation is completed, disable the OPTSTRT Bit */
      CLEAR_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

      /* Set the bit to force the option byte reloading */
      SET_BIT(FLASH->CR, FLASH_CR_OBL_LAUNCH);
    }
  }

  /* Process Unlocked */
  __HAL_UNLOCK(&pFlash);

  return status;
}
#endif

/**
  * @}
  */

/**
  * @}
  */
#endif /* HAL_FLASH_MODULE_ENABLED */



/**
  * @}
  */

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


