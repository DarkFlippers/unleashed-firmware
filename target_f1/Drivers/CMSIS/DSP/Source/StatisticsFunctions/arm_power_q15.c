/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_power_q15.c
 * Description:  Sum of the squares of the elements of a Q15 vector
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
 * @ingroup groupStats
 */

/**
 * @addtogroup power
 * @{
 */

/**
 * @brief Sum of the squares of the elements of a Q15 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult sum of the squares value returned here
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The function is implemented using a 64-bit internal accumulator.
 * The input is represented in 1.15 format.
 * Intermediate multiplication yields a 2.30 format, and this
 * result is added without saturation to a 64-bit accumulator in 34.30 format.
 * With 33 guard bits in the accumulator, there is no risk of overflow, and the
 * full precision of the intermediate multiplication is preserved.
 * Finally, the return result is in 34.30 format.
 *
 */

void arm_power_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q63_t * pResult)
{
  q63_t sum = 0;                                 /* Temporary result storage */

#if defined (ARM_MATH_DSP)
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t in32;                                    /* Temporary variable to store input value */
  q15_t in16;                                    /* Temporary variable to store input value */
  uint32_t blkCnt;                               /* loop counter */


  /* loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute Power and then store the result in a temporary variable, sum. */
    in32 = *__SIMD32(pSrc)++;
    sum = __SMLALD(in32, in32, sum);
    in32 = *__SIMD32(pSrc)++;
    sum = __SMLALD(in32, in32, sum);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute Power and then store the result in a temporary variable, sum. */
    in16 = *pSrc++;
    sum = __SMLALD(in16, in16, sum);

    /* Decrement the loop counter */
    blkCnt--;
  }

#else
  /* Run the below code for Cortex-M0 */

  q15_t in;                                      /* Temporary variable to store input value */
  uint32_t blkCnt;                               /* loop counter */


  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute Power and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += ((q31_t) in * in);

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

  /* Store the results in 34.30 format  */
  *pResult = sum;
}

/**
 * @} end of power group
 */
