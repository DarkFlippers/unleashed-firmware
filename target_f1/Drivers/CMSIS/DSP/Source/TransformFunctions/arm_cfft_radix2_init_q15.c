/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cfft_radix2_init_q15.c
 * Description:  Radix-2 Decimation in Frequency Q15 FFT & IFFT initialization function
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arm_math.h"
#include "arm_common_tables.h"

/**
 * @ingroup groupTransforms
 */


/**
 * @addtogroup ComplexFFT
 * @{
 */

/**
* @brief Initialization function for the Q15 CFFT/CIFFT.
* @deprecated Do not use this function.  It has been superseded by \ref arm_cfft_q15 and will be removed
* @param[in,out] *S             points to an instance of the Q15 CFFT/CIFFT structure.
* @param[in]     fftLen         length of the FFT.
* @param[in]     ifftFlag       flag that selects forward (ifftFlag=0) or inverse (ifftFlag=1) transform.
* @param[in]     bitReverseFlag flag that enables (bitReverseFlag=1) or disables (bitReverseFlag=0) bit reversal of output.
* @return        The function returns ARM_MATH_SUCCESS if initialization is successful or ARM_MATH_ARGUMENT_ERROR if <code>fftLen</code> is not a supported value.
*
* \par Description:
* \par
* The parameter <code>ifftFlag</code> controls whether a forward or inverse transform is computed.
* Set(=1) ifftFlag for calculation of CIFFT otherwise  CFFT is calculated
* \par
* The parameter <code>bitReverseFlag</code> controls whether output is in normal order or bit reversed order.
* Set(=1) bitReverseFlag for output to be in normal order otherwise output is in bit reversed order.
* \par
* The parameter <code>fftLen</code>	Specifies length of CFFT/CIFFT process. Supported FFT Lengths are 16, 64, 256, 1024.
* \par
* This Function also initializes Twiddle factor table pointer and Bit reversal table pointer.
*/

arm_status arm_cfft_radix2_init_q15(
  arm_cfft_radix2_instance_q15 * S,
  uint16_t fftLen,
  uint8_t ifftFlag,
  uint8_t bitReverseFlag)
{
  /*  Initialise the default arm status */
  arm_status status = ARM_MATH_SUCCESS;

  /*  Initialise the FFT length */
  S->fftLen = fftLen;

  /*  Initialise the Twiddle coefficient pointer */
  S->pTwiddle = (q15_t *) twiddleCoef_4096_q15;
  /*  Initialise the Flag for selection of CFFT or CIFFT */
  S->ifftFlag = ifftFlag;
  /*  Initialise the Flag for calculation Bit reversal or not */
  S->bitReverseFlag = bitReverseFlag;

  /*  Initializations of structure parameters depending on the FFT length */
  switch (S->fftLen)
  {
  case 4096U:
    /*  Initializations of structure parameters for 4096 point FFT */

    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 1U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 1U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) armBitRevTable;

    break;

  case 2048U:
    /*  Initializations of structure parameters for 2048 point FFT */

    /*  Initialise the twiddle coef modifier value */
    S->twidCoefModifier = 2U;
    /*  Initialise the bit reversal table modifier */
    S->bitRevFactor = 2U;
    /*  Initialise the bit reversal table pointer */
    S->pBitRevTable = (uint16_t *) & armBitRevTable[1];

    break;

  case 1024U:
    /*  Initializations of structure parameters for 1024 point FFT */
    S->twidCoefModifier = 4U;
    S->bitRevFactor = 4U;
    S->pBitRevTable = (uint16_t *) & armBitRevTable[3];

    break;

  case 512U:
    /*  Initializations of structure parameters for 512 point FFT */
    S->twidCoefModifier = 8U;
    S->bitRevFactor = 8U;
    S->pBitRevTable = (uint16_t *) & armBitRevTable[7];

    break;

  case 256U:
    /*  Initializations of structure parameters for 256 point FFT */
    S->twidCoefModifier = 16U;
    S->bitRevFactor = 16U;
    S->pBitRevTable = (uint16_t *) & armBitRevTable[15];

    break;

  case 128U:
    /*  Initializations of structure parameters for 128 point FFT */
    S->twidCoefModifier = 32U;
    S->bitRevFactor = 32U;
    S->pBitRevTable = (uint16_t *) & armBitRevTable[31];

    break;

  case 64U:
    /*  Initializations of structure parameters for 64 point FFT */
    S->twidCoefModifier = 64U;
    S->bitRevFactor = 64U;
    S->pBitRevTable = (uint16_t *) & armBitRevTable[63];

    break;

  case 32U:
    /*  Initializations of structure parameters for 32 point FFT */
    S->twidCoefModifier = 128U;
    S->bitRevFactor = 128U;
    S->pBitRevTable = (uint16_t *) & armBitRevTable[127];

    break;

  case 16U:
    /*  Initializations of structure parameters for 16 point FFT */
    S->twidCoefModifier = 256U;
    S->bitRevFactor = 256U;
    S->pBitRevTable = (uint16_t *) & armBitRevTable[255];

    break;

  default:
    /*  Reporting argument error if fftSize is not valid value */
    status = ARM_MATH_ARGUMENT_ERROR;
    break;
  }

  return (status);
}

/**
 * @} end of ComplexFFT group
 */
