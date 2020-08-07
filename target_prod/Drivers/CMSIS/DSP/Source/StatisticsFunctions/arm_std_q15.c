/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_std_q15.c
 * Description:  Standard deviation of an array of Q15 vector
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
 * @addtogroup STD
 * @{
 */

/**
 * @brief Standard deviation of the elements of a Q15 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult standard deviation value returned here
 * @return none.
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
 * Finally, the 34.30 result is truncated to 34.15 format by discarding the lower
 * 15 bits, and then saturated to yield a result in 1.15 format.
 */

void arm_std_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult)
{
  q31_t sum = 0;                                 /* Accumulator */
  q31_t meanOfSquares, squareOfMean;             /* square of mean and mean of square */
  uint32_t blkCnt;                               /* loop counter */
  q63_t sumOfSquares = 0;                        /* Accumulator */
#if defined (ARM_MATH_DSP)
  q31_t in;                                      /* input value */
  q15_t in1;                                     /* input value */
#else
  q15_t in;                                      /* input value */
#endif

  if (blockSize == 1U)
  {
    *pResult = 0;
    return;
  }

#if defined (ARM_MATH_DSP)
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1])  */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sum. */
    in = *__SIMD32(pSrc)++;
    sum += ((in << 16U) >> 16U);
    sum +=  (in >> 16U);
    sumOfSquares = __SMLALD(in, in, sumOfSquares);
    in = *__SIMD32(pSrc)++;
    sum += ((in << 16U) >> 16U);
    sum +=  (in >> 16U);
    sumOfSquares = __SMLALD(in, in, sumOfSquares);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sum. */
    in1 = *pSrc++;
    sumOfSquares = __SMLALD(in1, in1, sumOfSquares);
    sum += in1;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = (q31_t)(sumOfSquares / (q63_t)(blockSize - 1U));

  /* Compute square of mean */
  squareOfMean = (q31_t)((q63_t)sum * sum / (q63_t)(blockSize * (blockSize - 1U)));

  /* mean of the squares minus the square of the mean. */
  /* Compute standard deviation and store the result to the destination */
  arm_sqrt_q15(__SSAT((meanOfSquares - squareOfMean) >> 15U, 16U), pResult);

#else
  /* Run the below code for Cortex-M0 */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sumOfSquares. */
    in = *pSrc++;
    sumOfSquares += (in * in);

    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
    /* Compute sum of all input values and then store the result in a temporary variable, sum. */
    sum += in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = (q31_t)(sumOfSquares / (q63_t)(blockSize - 1U));

  /* Compute square of mean */
  squareOfMean = (q31_t)((q63_t)sum * sum / (q63_t)(blockSize * (blockSize - 1U)));

  /* mean of the squares minus the square of the mean. */
  /* Compute standard deviation and store the result to the destination */
  arm_sqrt_q15(__SSAT((meanOfSquares - squareOfMean) >> 15U, 16U), pResult);

#endif /* #if defined (ARM_MATH_DSP) */
}

/**
 * @} end of STD group
 */
