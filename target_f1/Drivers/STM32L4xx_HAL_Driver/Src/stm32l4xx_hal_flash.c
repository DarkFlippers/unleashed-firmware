/**
  ******************************************************************************
  * @file    stm32l4xx_hal_flash.c
  * @author  MCD Application Team
  * @brief   FLASH HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the internal FLASH memory:
  *           + Program operations functions
  *           + Memory Control functions
  *           + Peripheral Errors functions
  *
 @verbatim
  ==============================================================================
                        ##### FLASH peripheral features #####
  ==============================================================================

  [..] The Flash memory interface manages CPU AHB I-Code and D-Code accesses
       to the Flash memory. It implements the erase and program Flash memory operations
       and the read and write protection mechanisms.

  [..] The Flash memory interface accelerates code execution with a system of instruction
       prefetch and cache lines.

  [..] The FLASH main features are:
      (+) Flash memory read operations
      (+) Flash memory program/erase operations
      (+) Read / write protections
      (+) Option bytes programming
      (+) Prefetch on I-Code
      (+) 32 cache lines of 4*64 bits on I-Code
      (+) 8 cache lines of 4*64 bits on D-Code
      (+) Error code correction (ECC) : Data in flash are 72-bits word
          (8 bits added per double word)


                        ##### How to use this driver #####
 ==============================================================================
    [..]
      This driver provides functions and macros to configure and program the FLASH
      memory of all STM32L4xx devices.

      (#) Flash Memory IO Programming functions:
           (++) Lock and Unlock the FLASH interface using HAL_FLASH_Unlock() and
                HAL_FLASH_Lock() functions
           (++) Program functions: double word and fast program (full row programming)
           (++) There Two modes of programming :
            (+++) Polling mode using HAL_FLASH_Program() function
            (+++) Interrupt mode using HAL_FLASH_Program_IT() function

      (#) Interrupts and flags management functions :
           (++) Handle FLASH interrupts by calling HAL_FLASH_IRQHandler()
           (++) Callback functions are called when the flash operations are finished :
                HAL_FLASH_EndOfOperationCallback() when everything is ok, otherwise
                HAL_FLASH_OperationErrorCallback()
           (++) Get error flag status by calling HAL_GetError()

      (#) Option bytes management functions :
           (++) Lock and Unlock the option bytes using HAL_FLASH_OB_Unlock() and
                HAL_FLASH_OB_Lock() functions
           (++) Launch the reload of the option bytes using HAL_FLASH_Launch() function.
                In this case, a reset is generated

    [..]
      In addition to these functions, this driver includes a set of macros allowing
      to handle the following operations:
       (+) Set the latency
       (+) Enable/Disable the prefetch buffer
       (+) Enable/Disable the Instruction cache and the Data cache
       (+) Reset the Instruction cache and the Data cache
       (+) Enable/Disable the Flash power-down during low-power run and sleep modes
       (+) Enable/Disable the Flash interrupts
       (+) Monitor the Flash flags status

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

/** @defgroup FLASH FLASH
  * @brief FLASH HAL module driver
  * @{
  */

#ifdef HAL_FLASH_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#if defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_NB_DOUBLE_WORDS_IN_ROW  64
#else
#define FLASH_NB_DOUBLE_WORDS_IN_ROW  32
#endif
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup FLASH_Private_Variables FLASH Private Variables
 * @{
 */
/**
  * @brief  Variable used for Program/Erase sectors under interruption
  */
FLASH_ProcessTypeDef pFlash = {.Lock = HAL_UNLOCKED, \
                               .ErrorCode = HAL_FLASH_ERROR_NONE, \
                               .ProcedureOnGoing = FLASH_PROC_NONE, \
                               .Address = 0U, \
                               .Bank = FLASH_BANK_1, \
                               .Page = 0U, \
                               .NbPagesToErase = 0U, \
                               .CacheToReactivate = FLASH_CACHE_DISABLED};
/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup FLASH_Private_Functions FLASH Private Functions
 * @{
 */
static void          FLASH_Program_DoubleWord(uint32_t Address, uint64_t Data);
static void          FLASH_Program_Fast(uint32_t Address, uint32_t DataAddress);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup FLASH_Exported_Functions FLASH Exported Functions
  * @{
  */

/** @defgroup FLASH_Exported_Functions_Group1 Programming operation functions
 *  @brief   Programming operation functions
 *
@verbatim
 ===============================================================================
                  ##### Programming operation functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to manage the FLASH
    program operations.

@endverbatim
  * @{
  */

/**
  * @brief  Program double word or fast program of a row at a specified address.
  * @param  TypeProgram:  Indicate the way to program at a specified address.
  *                           This parameter can be a value of @ref FLASH_Type_Program
  * @param  Address:  specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed
  *                This parameter is the data for the double word program and the address where
  *                are stored the data for the row fast program
  *
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
  HAL_StatusTypeDef status;
  uint32_t prog_bit = 0;

  /* Process Locked */
  __HAL_LOCK(&pFlash);

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  if(status == HAL_OK)
  {
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

    /* Deactivate the data cache if they are activated to avoid data misbehavior */
    if(READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
    {
      /* Disable data cache  */
      __HAL_FLASH_DATA_CACHE_DISABLE();
      pFlash.CacheToReactivate = FLASH_CACHE_DCACHE_ENABLED;
    }
    else
    {
      pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
    }

    if(TypeProgram == FLASH_TYPEPROGRAM_DOUBLEWORD)
    {
      /* Program double-word (64-bit) at a specified address */
      FLASH_Program_DoubleWord(Address, Data);
      prog_bit = FLASH_CR_PG;
    }
    else if((TypeProgram == FLASH_TYPEPROGRAM_FAST) || (TypeProgram == FLASH_TYPEPROGRAM_FAST_AND_LAST))
    {
      /* Fast program a 32 row double-word (64-bit) at a specified address */
      FLASH_Program_Fast(Address, (uint32_t)Data);

      /* If it is the last row, the bit will be cleared at the end of the operation */
      if(TypeProgram == FLASH_TYPEPROGRAM_FAST_AND_LAST)
      {
        prog_bit = FLASH_CR_FSTPG;
      }
    }
    else
    {
      /* Nothing to do */
    }

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

    /* If the program operation is completed, disable the PG or FSTPG Bit */
    if (prog_bit != 0U)
    {
      CLEAR_BIT(FLASH->CR, prog_bit);
    }

    /* Flush the caches to be sure of the data consistency */
    FLASH_FlushCaches();
  }

  /* Process Unlocked */
  __HAL_UNLOCK(&pFlash);

  return status;
}

/**
  * @brief  Program double word or fast program of a row at a specified address with interrupt enabled.
  * @param  TypeProgram:  Indicate the way to program at a specified address.
  *                           This parameter can be a value of @ref FLASH_Type_Program
  * @param  Address:  specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed
  *                This parameter is the data for the double word program and the address where
  *                are stored the data for the row fast program
  *
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));

  /* Process Locked */
  __HAL_LOCK(&pFlash);

  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

  /* Deactivate the data cache if they are activated to avoid data misbehavior */
  if(READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
  {
    /* Disable data cache  */
    __HAL_FLASH_DATA_CACHE_DISABLE();
    pFlash.CacheToReactivate = FLASH_CACHE_DCACHE_ENABLED;
  }
  else
  {
    pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
  }

  /* Set internal variables used by the IRQ handler */
  if(TypeProgram == FLASH_TYPEPROGRAM_FAST_AND_LAST)
  {
    pFlash.ProcedureOnGoing = FLASH_PROC_PROGRAM_LAST;
  }
  else
  {
    pFlash.ProcedureOnGoing = FLASH_PROC_PROGRAM;
  }
  pFlash.Address = Address;

  /* Enable End of Operation and Error interrupts */
  __HAL_FLASH_ENABLE_IT(FLASH_IT_EOP | FLASH_IT_OPERR);

  if(TypeProgram == FLASH_TYPEPROGRAM_DOUBLEWORD)
  {
    /* Program double-word (64-bit) at a specified address */
    FLASH_Program_DoubleWord(Address, Data);
  }
  else if((TypeProgram == FLASH_TYPEPROGRAM_FAST) || (TypeProgram == FLASH_TYPEPROGRAM_FAST_AND_LAST))
  {
    /* Fast program a 32 row double-word (64-bit) at a specified address */
    FLASH_Program_Fast(Address, (uint32_t)Data);
  }
  else
  {
    /* Nothing to do */
  }

  return status;
}

/**
  * @brief Handle FLASH interrupt request.
  * @retval None
  */
void HAL_FLASH_IRQHandler(void)
{
  uint32_t tmp_page;
  uint32_t error;
  FLASH_ProcedureTypeDef procedure;

  /* If the operation is completed, disable the PG, PNB, MER1, MER2 and PER Bit */
  CLEAR_BIT(FLASH->CR, (FLASH_CR_PG | FLASH_CR_MER1 | FLASH_CR_PER | FLASH_CR_PNB));
#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || \
    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
  CLEAR_BIT(FLASH->CR, FLASH_CR_MER2);
#endif

  /* Disable the FSTPG Bit only if it is the last row programmed */
  if(pFlash.ProcedureOnGoing == FLASH_PROC_PROGRAM_LAST)
  {
    CLEAR_BIT(FLASH->CR, FLASH_CR_FSTPG);
  }

  /* Check FLASH operation error flags */
  error = (FLASH->SR & FLASH_FLAG_SR_ERRORS);
  error |= (FLASH->ECCR & FLASH_FLAG_ECCC);

  if (error !=0U)
  {
    /*Save the error code*/
    pFlash.ErrorCode |= error;

    /* Clear error programming flags */
    __HAL_FLASH_CLEAR_FLAG(error);

    /* Flush the caches to be sure of the data consistency */
    FLASH_FlushCaches() ;

    /* FLASH error interrupt user callback */
    procedure = pFlash.ProcedureOnGoing;
    if(procedure == FLASH_PROC_PAGE_ERASE)
    {
       HAL_FLASH_OperationErrorCallback(pFlash.Page);
    }
    else if(procedure == FLASH_PROC_MASS_ERASE)
    {
        HAL_FLASH_OperationErrorCallback(pFlash.Bank);
    }
    else if((procedure == FLASH_PROC_PROGRAM) ||
            (procedure == FLASH_PROC_PROGRAM_LAST))
    {
       HAL_FLASH_OperationErrorCallback(pFlash.Address);
    }
    else
    {
       HAL_FLASH_OperationErrorCallback(0U);
    }

    /*Stop the procedure ongoing*/
    pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
  }

  /* Check FLASH End of Operation flag  */
  if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP) != 0U)
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);

    if(pFlash.ProcedureOnGoing == FLASH_PROC_PAGE_ERASE)
    {
      /* Nb of pages to erased can be decreased */
      pFlash.NbPagesToErase--;

      /* Check if there are still pages to erase*/
      if(pFlash.NbPagesToErase != 0U)
      {
        /* Indicate user which page has been erased*/
        HAL_FLASH_EndOfOperationCallback(pFlash.Page);

        /* Increment page number */
        pFlash.Page++;
        tmp_page = pFlash.Page;
        FLASH_PageErase(tmp_page, pFlash.Bank);
      }
      else
      {
        /* No more pages to Erase */
        /* Reset Address and stop Erase pages procedure */
        pFlash.Page = 0xFFFFFFFFU;
        pFlash.ProcedureOnGoing = FLASH_PROC_NONE;

        /* Flush the caches to be sure of the data consistency */
        FLASH_FlushCaches() ;

        /* FLASH EOP interrupt user callback */
        HAL_FLASH_EndOfOperationCallback(pFlash.Page);
      }
    }
    else
    {
      /* Flush the caches to be sure of the data consistency */
      FLASH_FlushCaches() ;

      procedure = pFlash.ProcedureOnGoing;
      if(procedure == FLASH_PROC_MASS_ERASE)
      {
        /* MassErase ended. Return the selected bank */
        /* FLASH EOP interrupt user callback */
        HAL_FLASH_EndOfOperationCallback(pFlash.Bank);
      }
      else if((procedure == FLASH_PROC_PROGRAM) ||
              (procedure == FLASH_PROC_PROGRAM_LAST))
      {
        /* Program ended. Return the selected address */
        /* FLASH EOP interrupt user callback */
        HAL_FLASH_EndOfOperationCallback(pFlash.Address);
      }
      else
      {
        /* Nothing to do */
      }

      /*Clear the procedure ongoing*/
      pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
    }
  }

  if(pFlash.ProcedureOnGoing == FLASH_PROC_NONE)
  {
    /* Disable End of Operation and Error interrupts */
    __HAL_FLASH_DISABLE_IT(FLASH_IT_EOP | FLASH_IT_OPERR);

    /* Process Unlocked */
    __HAL_UNLOCK(&pFlash);
  }
}

/**
  * @brief  FLASH end of operation interrupt callback.
  * @param  ReturnValue: The value saved in this parameter depends on the ongoing procedure
  *                  Mass Erase: Bank number which has been requested to erase
  *                  Page Erase: Page which has been erased
  *                    (if 0xFFFFFFFF, it means that all the selected pages have been erased)
  *                  Program: Address which was selected for data program
  * @retval None
  */
__weak void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ReturnValue);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_FLASH_EndOfOperationCallback could be implemented in the user file
   */
}

/**
  * @brief  FLASH operation error interrupt callback.
  * @param  ReturnValue: The value saved in this parameter depends on the ongoing procedure
  *                 Mass Erase: Bank number which has been requested to erase
  *                 Page Erase: Page number which returned an error
  *                 Program: Address which was selected for data program
  * @retval None
  */
__weak void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ReturnValue);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_FLASH_OperationErrorCallback could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup FLASH_Exported_Functions_Group2 Peripheral Control functions
 *  @brief   Management functions
 *
@verbatim
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to control the FLASH
    memory operations.

@endverbatim
  * @{
  */

/**
  * @brief  Unlock the FLASH control register access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
  {
    /* Authorize the FLASH Registers access */
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

    /* Verify Flash is unlocked */
    if(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
    {
      status = HAL_ERROR;
    }
  }

  return status;
}

/**
  * @brief  Lock the FLASH control register access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
  /* Set the LOCK Bit to lock the FLASH Registers access */
  SET_BIT(FLASH->CR, FLASH_CR_LOCK);

  return HAL_OK;
}

/**
  * @brief  Unlock the FLASH Option Bytes Registers access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Unlock(void)
{
  if(READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != 0U)
  {
    /* Authorizes the Option Byte register programming */
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY1);
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY2);
  }
  else
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Lock the FLASH Option Bytes Registers access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Lock(void)
{
  /* Set the OPTLOCK Bit to lock the FLASH Option Byte Registers access */
  SET_BIT(FLASH->CR, FLASH_CR_OPTLOCK);

  return HAL_OK;
}

/**
  * @brief  Launch the option byte loading.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Launch(void)
{
  /* Set the bit to force the option byte reloading */
  SET_BIT(FLASH->CR, FLASH_CR_OBL_LAUNCH);

  /* Wait for last operation to be completed */
  return(FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE));
}

/**
  * @}
  */

/** @defgroup FLASH_Exported_Functions_Group3 Peripheral State and Errors functions
 *  @brief   Peripheral Errors functions
 *
@verbatim
 ===============================================================================
                ##### Peripheral Errors functions #####
 ===============================================================================
    [..]
    This subsection permits to get in run-time Errors of the FLASH peripheral.

@endverbatim
  * @{
  */

/**
  * @brief  Get the specific FLASH error flag.
  * @retval FLASH_ErrorCode: The returned value can be:
  *            @arg HAL_FLASH_ERROR_RD: FLASH Read Protection error flag (PCROP)
  *            @arg HAL_FLASH_ERROR_PGS: FLASH Programming Sequence error flag
  *            @arg HAL_FLASH_ERROR_PGP: FLASH Programming Parallelism error flag
  *            @arg HAL_FLASH_ERROR_PGA: FLASH Programming Alignment error flag
  *            @arg HAL_FLASH_ERROR_WRP: FLASH Write protected error flag
  *            @arg HAL_FLASH_ERROR_OPERATION: FLASH operation Error flag
  *            @arg HAL_FLASH_ERROR_NONE: No error set
  *            @arg HAL_FLASH_ERROR_OP: FLASH Operation error
  *            @arg HAL_FLASH_ERROR_PROG: FLASH Programming error
  *            @arg HAL_FLASH_ERROR_WRP: FLASH Write protection error
  *            @arg HAL_FLASH_ERROR_PGA: FLASH Programming alignment error
  *            @arg HAL_FLASH_ERROR_SIZ: FLASH Size error
  *            @arg HAL_FLASH_ERROR_PGS: FLASH Programming sequence error
  *            @arg HAL_FLASH_ERROR_MIS: FLASH Fast programming data miss error
  *            @arg HAL_FLASH_ERROR_FAST: FLASH Fast programming error
  *            @arg HAL_FLASH_ERROR_RD: FLASH PCROP read error
  *            @arg HAL_FLASH_ERROR_OPTV: FLASH Option validity error
  *            @arg FLASH_FLAG_PEMPTY : FLASH Boot from not programmed flash (apply only for STM32L43x/STM32L44x devices)
  *            @arg HAL_FLASH_ERROR_ECCD: FLASH two ECC errors have been detected
  */
uint32_t HAL_FLASH_GetError(void)
{
   return pFlash.ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/** @addtogroup FLASH_Private_Functions
  * @{
  */

/**
  * @brief  Wait for a FLASH operation to complete.
  * @param  Timeout: maximum flash operation timeout
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout)
{
  /* Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
     Even if the FLASH operation fails, the BUSY flag will be reset and an error
     flag will be set */

  uint32_t tickstart = HAL_GetTick();
  uint32_t error;

  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY))
  {
    if(Timeout != HAL_MAX_DELAY)
    {
      if((HAL_GetTick() - tickstart) >= Timeout)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  error = (FLASH->SR & FLASH_FLAG_SR_ERRORS);
  error |= (FLASH->ECCR & FLASH_FLAG_ECCD);

  if(error != 0u)
  {
    /*Save the error code*/
    pFlash.ErrorCode |= error;

    /* Clear error programming flags */
    __HAL_FLASH_CLEAR_FLAG(error);

    return HAL_ERROR;
  }

  /* Check FLASH End of Operation flag  */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
  }

  /* If there is an error flag set */
  return HAL_OK;
}

/**
  * @brief  Program double-word (64-bit) at a specified address.
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval None
  */
static void FLASH_Program_DoubleWord(uint32_t Address, uint64_t Data)
{
  /* Check the parameters */
  assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));

  /* Set PG bit */
  SET_BIT(FLASH->CR, FLASH_CR_PG);

  /* Program first word */
  *(__IO uint32_t*)Address = (uint32_t)Data;

  /* Barrier to ensure programming is performed in 2 steps, in right order
    (independently of compiler optimization behavior) */
  __ISB();

  /* Program second word */
  *(__IO uint32_t*)(Address+4U) = (uint32_t)(Data >> 32);
}

/**
  * @brief  Fast program a row double-word (64-bit) at a specified address.
  * @param  Address: specifies the address to be programmed.
  * @param  DataAddress: specifies the address where the data are stored.
  * @retval None
  */
static void FLASH_Program_Fast(uint32_t Address, uint32_t DataAddress)
{
  uint32_t primask_bit;
  uint8_t row_index = (2*FLASH_NB_DOUBLE_WORDS_IN_ROW);
  __IO uint32_t *dest_addr = (__IO uint32_t*)Address;
  __IO uint32_t *src_addr = (__IO uint32_t*)DataAddress;

  /* Check the parameters */
  assert_param(IS_FLASH_MAIN_MEM_ADDRESS(Address));

  /* Set FSTPG bit */
  SET_BIT(FLASH->CR, FLASH_CR_FSTPG);

  /* Disable interrupts to avoid any interruption during the loop */
  primask_bit = __get_PRIMASK();
  __disable_irq();

  /* Program the double word of the row */
  do
  {
    *dest_addr = *src_addr;
    dest_addr++;
    src_addr++;
    row_index--;
  } while (row_index != 0U);

  /* Re-enable the interrupts */
  __set_PRIMASK(primask_bit);
}

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
