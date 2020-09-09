/**
  ******************************************************************************
  * @file    stm32l4xx_hal_crc_ex.c
  * @author  MCD Application Team
  * @brief   Extended CRC HAL module driver.
  *          This file provides firmware functions to manage the extended
  *          functionalities of the CRC peripheral.
  *
  @verbatim
================================================================================
            ##### How to use this driver #####
================================================================================
    [..]
         (+) Set user-defined generating polynomial thru HAL_CRCEx_Polynomial_Set()
         (+) Configure Input or Output data inversion

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

/** @defgroup CRCEx CRCEx
  * @brief CRC Extended HAL module driver
  * @{
  */

#ifdef HAL_CRC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup CRCEx_Exported_Functions CRC Extended Exported Functions
  * @{
  */

/** @defgroup CRCEx_Exported_Functions_Group1 Extended Initialization/de-initialization functions
  * @brief    Extended Initialization and Configuration functions.
  *
@verbatim
 ===============================================================================
            ##### Extended configuration functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Configure the generating polynomial
      (+) Configure the input data inversion
      (+) Configure the output data inversion

@endverbatim
  * @{
  */


/**
  * @brief  Initialize the CRC polynomial if different from default one.
  * @param  hcrc CRC handle
  * @param  Pol CRC generating polynomial (7, 8, 16 or 32-bit long).
  *         This parameter is written in normal representation, e.g.
  *         @arg for a polynomial of degree 7, X^7 + X^6 + X^5 + X^2 + 1 is written 0x65
  *         @arg for a polynomial of degree 16, X^16 + X^12 + X^5 + 1 is written 0x1021
  * @param  PolyLength CRC polynomial length.
  *         This parameter can be one of the following values:
  *          @arg @ref CRC_POLYLENGTH_7B  7-bit long CRC (generating polynomial of degree 7)
  *          @arg @ref CRC_POLYLENGTH_8B  8-bit long CRC (generating polynomial of degree 8)
  *          @arg @ref CRC_POLYLENGTH_16B 16-bit long CRC (generating polynomial of degree 16)
  *          @arg @ref CRC_POLYLENGTH_32B 32-bit long CRC (generating polynomial of degree 32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRCEx_Polynomial_Set(CRC_HandleTypeDef *hcrc, uint32_t Pol, uint32_t PolyLength)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t msb = 31U; /* polynomial degree is 32 at most, so msb is initialized to max value */

  /* Check the parameters */
  assert_param(IS_CRC_POL_LENGTH(PolyLength));

  /* check polynomial definition vs polynomial size:
   * polynomial length must be aligned with polynomial
   * definition. HAL_ERROR is reported if Pol degree is
   * larger than that indicated by PolyLength.
   * Look for MSB position: msb will contain the degree of
   *  the second to the largest polynomial member. E.g., for
   *  X^7 + X^6 + X^5 + X^2 + 1, msb = 6. */
  while ((msb-- > 0U) && ((Pol & ((uint32_t)(0x1U) << (msb & 0x1FU))) == 0U))
  {
  }

  switch (PolyLength)
  {
    case CRC_POLYLENGTH_7B:
      if (msb >= HAL_CRC_LENGTH_7B)
      {
        status =   HAL_ERROR;
      }
      break;
    case CRC_POLYLENGTH_8B:
      if (msb >= HAL_CRC_LENGTH_8B)
      {
        status =   HAL_ERROR;
      }
      break;
    case CRC_POLYLENGTH_16B:
      if (msb >= HAL_CRC_LENGTH_16B)
      {
        status =   HAL_ERROR;
      }
      break;

    case CRC_POLYLENGTH_32B:
      /* no polynomial definition vs. polynomial length issue possible */
      break;
    default:
      status =  HAL_ERROR;
      break;
  }
  if (status == HAL_OK)
  {
    /* set generating polynomial */
    WRITE_REG(hcrc->Instance->POL, Pol);

    /* set generating polynomial size */
    MODIFY_REG(hcrc->Instance->CR, CRC_CR_POLYSIZE, PolyLength);
  }
  /* Return function status */
  return status;
}

/**
  * @brief  Set the Reverse Input data mode.
  * @param  hcrc CRC handle
  * @param  InputReverseMode Input Data inversion mode.
  *         This parameter can be one of the following values:
  *          @arg @ref CRC_INPUTDATA_INVERSION_NONE     no change in bit order (default value)
  *          @arg @ref CRC_INPUTDATA_INVERSION_BYTE     Byte-wise bit reversal
  *          @arg @ref CRC_INPUTDATA_INVERSION_HALFWORD HalfWord-wise bit reversal
  *          @arg @ref CRC_INPUTDATA_INVERSION_WORD     Word-wise bit reversal
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRCEx_Input_Data_Reverse(CRC_HandleTypeDef *hcrc, uint32_t InputReverseMode)
{
  /* Check the parameters */
  assert_param(IS_CRC_INPUTDATA_INVERSION_MODE(InputReverseMode));

  /* Change CRC peripheral state */
  hcrc->State = HAL_CRC_STATE_BUSY;

  /* set input data inversion mode */
  MODIFY_REG(hcrc->Instance->CR, CRC_CR_REV_IN, InputReverseMode);
  /* Change CRC peripheral state */
  hcrc->State = HAL_CRC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Set the Reverse Output data mode.
  * @param  hcrc CRC handle
  * @param  OutputReverseMode Output Data inversion mode.
  *         This parameter can be one of the following values:
  *          @arg @ref CRC_OUTPUTDATA_INVERSION_DISABLE no CRC inversion (default value)
  *          @arg @ref CRC_OUTPUTDATA_INVERSION_ENABLE  bit-level inversion (e.g. for a 8-bit CRC: 0xB5 becomes 0xAD)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRCEx_Output_Data_Reverse(CRC_HandleTypeDef *hcrc, uint32_t OutputReverseMode)
{
  /* Check the parameters */
  assert_param(IS_CRC_OUTPUTDATA_INVERSION_MODE(OutputReverseMode));

  /* Change CRC peripheral state */
  hcrc->State = HAL_CRC_STATE_BUSY;

  /* set output data inversion mode */
  MODIFY_REG(hcrc->Instance->CR, CRC_CR_REV_OUT, OutputReverseMode);

  /* Change CRC peripheral state */
  hcrc->State = HAL_CRC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}




/**
  * @}
  */


/**
  * @}
  */


#endif /* HAL_CRC_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
