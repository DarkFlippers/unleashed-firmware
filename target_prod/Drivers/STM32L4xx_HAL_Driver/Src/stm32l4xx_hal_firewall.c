/**
  ******************************************************************************
  * @file    stm32l4xx_hal_firewall.c
  * @author  MCD Application Team
  * @brief   FIREWALL HAL module driver.
  *          This file provides firmware functions to manage the Firewall
  *          Peripheral initialization and enabling.
  *
  *
  @verbatim
 ===============================================================================
                        ##### How to use this driver #####
 ===============================================================================
  [..]
    The FIREWALL HAL driver can be used as follows:

    (#) Declare a FIREWALL_InitTypeDef initialization structure.

    (#) Resort to HAL_FIREWALL_Config() API to initialize the Firewall

    (#) Enable the FIREWALL in calling HAL_FIREWALL_EnableFirewall() API

    (#) To ensure that any code executed outside the protected segment closes the
        FIREWALL, the user must set the flag FIREWALL_PRE_ARM_SET in calling
        __HAL_FIREWALL_PREARM_ENABLE() macro if called within a protected code segment
        or
        HAL_FIREWALL_EnablePreArmFlag() API if called outside of protected code segment
        after HAL_FIREWALL_Config() call.


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

/** @defgroup FIREWALL FIREWALL
  * @brief HAL FIREWALL module driver
  * @{
  */
#ifdef HAL_FIREWALL_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/** @defgroup FIREWALL_Exported_Functions FIREWALL Exported Functions
  * @{
  */

/** @defgroup FIREWALL_Exported_Functions_Group1 Initialization Functions
  * @brief    Initialization and Configuration Functions
  *
@verbatim
===============================================================================
            ##### Initialization and Configuration functions #####
 ===============================================================================
    [..]
    This subsection provides the functions allowing to initialize the Firewall.
    Initialization is done by HAL_FIREWALL_Config():

      (+) Enable the Firewall clock thru __HAL_RCC_FIREWALL_CLK_ENABLE() macro.

      (+) Set the protected code segment address start and length.

      (+) Set the protected non-volatile and/or volatile data segments
          address starts and lengths if applicable.

      (+) Set the volatile data segment execution and sharing status.

      (+) Length must be set to 0 for an unprotected segment.

@endverbatim
  * @{
  */

/**
  * @brief Initialize the Firewall according to the FIREWALL_InitTypeDef structure parameters.
  * @param fw_init: Firewall initialization structure
  * @note  The API returns HAL_ERROR if the Firewall is already enabled.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FIREWALL_Config(FIREWALL_InitTypeDef * fw_init)
{
  /* Check the Firewall initialization structure allocation */
  if(fw_init == NULL)
  {
    return HAL_ERROR;
  }

  /* Enable Firewall clock */
  __HAL_RCC_FIREWALL_CLK_ENABLE();

  /* Make sure that Firewall is not enabled already */
  if (__HAL_FIREWALL_IS_ENABLED() != RESET)
  {
    return HAL_ERROR;
  }

  /* Check Firewall configuration addresses and lengths when segment is protected */
  /* Code segment */
  if (fw_init->CodeSegmentLength != 0U)
  {
    assert_param(IS_FIREWALL_CODE_SEGMENT_ADDRESS(fw_init->CodeSegmentStartAddress));
    assert_param(IS_FIREWALL_CODE_SEGMENT_LENGTH(fw_init->CodeSegmentStartAddress, fw_init->CodeSegmentLength));
    /* Make sure that NonVDataSegmentLength is properly set to prevent code segment access */
    if (fw_init->NonVDataSegmentLength < 0x100U)
    {
      return HAL_ERROR;
    }
  }
  /* Non volatile data segment */
  if (fw_init->NonVDataSegmentLength != 0U)
  {
    assert_param(IS_FIREWALL_NONVOLATILEDATA_SEGMENT_ADDRESS(fw_init->NonVDataSegmentStartAddress));
    assert_param(IS_FIREWALL_NONVOLATILEDATA_SEGMENT_LENGTH(fw_init->NonVDataSegmentStartAddress, fw_init->NonVDataSegmentLength));
  }
  /* Volatile data segment */
  if (fw_init->VDataSegmentLength != 0U)
  {
    assert_param(IS_FIREWALL_VOLATILEDATA_SEGMENT_ADDRESS(fw_init->VDataSegmentStartAddress));
    assert_param(IS_FIREWALL_VOLATILEDATA_SEGMENT_LENGTH(fw_init->VDataSegmentStartAddress, fw_init->VDataSegmentLength));
  }

  /* Check Firewall Configuration Register parameters */
  assert_param(IS_FIREWALL_VOLATILEDATA_EXECUTE(fw_init->VolatileDataExecution));
  assert_param(IS_FIREWALL_VOLATILEDATA_SHARE(fw_init->VolatileDataShared));


   /* Configuration */

  /* Protected code segment start address configuration */
  WRITE_REG(FIREWALL->CSSA, (FW_CSSA_ADD & fw_init->CodeSegmentStartAddress));
	/* Protected code segment length configuration */
  WRITE_REG(FIREWALL->CSL, (FW_CSL_LENG & fw_init->CodeSegmentLength));

  /* Protected non volatile data segment start address configuration */
  WRITE_REG(FIREWALL->NVDSSA, (FW_NVDSSA_ADD & fw_init->NonVDataSegmentStartAddress));
	/* Protected non volatile data segment length configuration */
  WRITE_REG(FIREWALL->NVDSL, (FW_NVDSL_LENG & fw_init->NonVDataSegmentLength));

  /* Protected volatile data segment start address configuration */
  WRITE_REG(FIREWALL->VDSSA, (FW_VDSSA_ADD & fw_init->VDataSegmentStartAddress));
	/* Protected volatile data segment length configuration */
  WRITE_REG(FIREWALL->VDSL, (FW_VDSL_LENG & fw_init->VDataSegmentLength));

  /* Set Firewall Configuration Register VDE and VDS bits
     (volatile data execution and shared configuration) */
  MODIFY_REG(FIREWALL->CR, FW_CR_VDS|FW_CR_VDE, fw_init->VolatileDataExecution|fw_init->VolatileDataShared);

  return HAL_OK;
}

/**
  * @brief Retrieve the Firewall configuration.
  * @param fw_config: Firewall configuration, type is same as initialization structure
  * @note This API can't be executed inside a code area protected by the Firewall
  *       when the Firewall is enabled
  * @note If NVDSL register is different from 0, that is, if the non volatile data segment
  *       is defined, this API can't be executed when the Firewall is enabled.
  * @note User should resort to __HAL_FIREWALL_GET_PREARM() macro to retrieve FPA bit status
  * @retval None
  */
void HAL_FIREWALL_GetConfig(FIREWALL_InitTypeDef * fw_config)
{

  /* Enable Firewall clock, in case no Firewall configuration has been carried
     out up to this point */
  __HAL_RCC_FIREWALL_CLK_ENABLE();

  /* Retrieve code segment protection setting */
  fw_config->CodeSegmentStartAddress = (READ_REG(FIREWALL->CSSA) & FW_CSSA_ADD);
  fw_config->CodeSegmentLength = (READ_REG(FIREWALL->CSL) & FW_CSL_LENG);

  /* Retrieve non volatile data segment protection setting */
  fw_config->NonVDataSegmentStartAddress = (READ_REG(FIREWALL->NVDSSA) & FW_NVDSSA_ADD);
  fw_config->NonVDataSegmentLength = (READ_REG(FIREWALL->NVDSL) & FW_NVDSL_LENG);

  /* Retrieve volatile data segment protection setting */
  fw_config->VDataSegmentStartAddress = (READ_REG(FIREWALL->VDSSA) & FW_VDSSA_ADD);
  fw_config->VDataSegmentLength = (READ_REG(FIREWALL->VDSL) & FW_VDSL_LENG);

  /* Retrieve volatile data execution setting */
  fw_config->VolatileDataExecution = (READ_REG(FIREWALL->CR) & FW_CR_VDE);

  /* Retrieve volatile data shared setting */
  fw_config->VolatileDataShared = (READ_REG(FIREWALL->CR) & FW_CR_VDS);

  return;
}



/**
  * @brief Enable FIREWALL.
  * @note Firewall is enabled in clearing FWDIS bit of SYSCFG CFGR1 register.
  *       Once enabled, the Firewall cannot be disabled by software. Only a
  *       system reset can set again FWDIS bit.
  * @retval None
  */
void HAL_FIREWALL_EnableFirewall(void)
{
  /* Clears FWDIS bit of SYSCFG CFGR1 register */
  CLEAR_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_FWDIS);

}

/**
  * @brief Enable FIREWALL pre arm.
  * @note When FPA bit is set, any code executed outside the protected segment
  *       will close the Firewall.
  * @note This API provides the same service as __HAL_FIREWALL_PREARM_ENABLE() macro
  *       but can't be executed inside a code area protected by the Firewall.
  * @note When the Firewall is disabled, user can resort to HAL_FIREWALL_EnablePreArmFlag() API any time.
  * @note When the Firewall is enabled and NVDSL register is equal to 0 (that is,
  *       when the non volatile data segment is not defined),
  *        **  this API can be executed when the Firewall is closed
  *        **  when the Firewall is opened, user should resort to
  *            __HAL_FIREWALL_PREARM_ENABLE() macro instead
  * @note When the Firewall is enabled and  NVDSL register is different from 0
  *       (that is, when the non volatile data segment is defined)
  *       **  FW_CR register can be accessed only when the Firewall is opened:
  *           user should resort to  __HAL_FIREWALL_PREARM_ENABLE() macro instead.
  * @retval None
  */
void HAL_FIREWALL_EnablePreArmFlag(void)
{
  /* Set FPA bit */
  SET_BIT(FIREWALL->CR, FW_CR_FPA);
}


/**
  * @brief Disable FIREWALL pre arm.
  * @note When FPA bit is reset, any code executed outside the protected segment
  *       when the Firewall is opened will generate a system reset.
  * @note This API provides the same service as __HAL_FIREWALL_PREARM_DISABLE() macro
  *       but can't be executed inside a code area protected by the Firewall.
  * @note When the Firewall is disabled, user can resort to HAL_FIREWALL_EnablePreArmFlag() API any time.
  * @note When the Firewall is enabled and NVDSL register is equal to 0 (that is,
  *       when the non volatile data segment is not defined),
  *        **  this API can be executed when the Firewall is closed
  *        **  when the Firewall is opened, user should resort to
  *            __HAL_FIREWALL_PREARM_DISABLE() macro instead
  * @note When the Firewall is enabled and  NVDSL register is different from 0
  *       (that is, when the non volatile data segment is defined)
  *       **  FW_CR register can be accessed only when the Firewall is opened:
  *           user should resort to  __HAL_FIREWALL_PREARM_DISABLE() macro instead.

  * @retval None
  */
void HAL_FIREWALL_DisablePreArmFlag(void)
{
  /* Clear FPA bit */
  CLEAR_BIT(FIREWALL->CR, FW_CR_FPA);
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_FIREWALL_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
