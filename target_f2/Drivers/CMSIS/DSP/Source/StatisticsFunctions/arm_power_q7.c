/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_power_q7.c
 * Description:  Sum of the squares of the elements of a Q7 vector
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
 * @brief Sum of the squares of the elements of a Q7 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult sum of the squares value returned here
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The function is implemented using a 32-bit internal accumulator.
 * The input is represented in 1.7 format.
 * Intermediate multiplication yields a 2.14 format, and this
 * result is added without saturation to an accumulator in 18.14 format.
 * With 17 guard bits in the accumulator, there is no risk of overflow, and the
 * full precision of the intermediate multiplication is preserved.
 * Finally, the return result is in 18.14 format.
 *
 */

void arm_power_q7(
  q7_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult)
{
  q31_t sum = 0;                                 /* Temporary result storage */
  q7_t in;                                       /* Temporary variable to store input */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t input1;                                  /* Temporary variable to store packed input */
  q31_t in1, in2;                                /* Temporary variables to store input */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* Reading two inputs of pSrc vector and packing */
    input1 = *__SIMD32(pSrc)++;

    in1 = __SXTB16(__ROR(input1, 8));
    in2 = __SXTB16(input1);

    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* calculate power and accumulate to accumulator */
    sum = __SMLAD(in1, in1, sum);
    sum = __SMLAD(in2, in2, sum);

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
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute Power and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += ((q15_t) in * in);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Store the result in 18.14 format  */
  *pResult = sum;
}

/**
 * @} end of power group
 */
