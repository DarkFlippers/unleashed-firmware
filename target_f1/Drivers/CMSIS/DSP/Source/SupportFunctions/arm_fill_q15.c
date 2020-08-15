/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fill_q15.c
 * Description:  Fills a constant value into a Q15 vector
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

/**
 * @ingroup groupSupport
 */

/**
 * @addtogroup Fill
 * @{
 */

/**
 * @brief Fills a constant value into a Q15 vector.
 * @param[in]       value input value to be filled
 * @param[out]      *pDst points to output vector
 * @param[in]       blockSize length of the output vector
 * @return none.
 *
 */

void arm_fill_q15(
  q15_t value,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t packedValue;                             /* value packed to 32 bits */


  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* Packing two 16 bit values to 32 bit value in order to use SIMD */
  packedValue = __PKHBT(value, value, 16U);

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = value */
    /* Fill the value in the destination buffer */
    *__SIMD32(pDst)++ = packedValue;
    *__SIMD32(pDst)++ = packedValue;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */

  while (blkCnt > 0U)
  {
    /* C = value */
    /* Fill the value in the destination buffer */
    *pDst++ = value;

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of Fill group
 */
