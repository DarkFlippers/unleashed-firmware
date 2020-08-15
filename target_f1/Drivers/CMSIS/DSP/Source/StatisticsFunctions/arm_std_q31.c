/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_std_q31.c
 * Description:  Standard deviation of an array of Q31 type.
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
 * @brief Standard deviation of the elements of a Q31 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult standard deviation value returned here
 * @return none.
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 *\par
 * The function is implemented using an internal 64-bit accumulator.
 * The input is represented in 1.31 format, which is then downshifted by 8 bits
 * which yields 1.23, and intermediate multiplication yields a 2.46 format.
 * The accumulator maintains full precision of the intermediate multiplication results,
 * but provides only a 16 guard bits.
 * There is no saturation on intermediate additions.
 * If the accumulator overflows it wraps around and distorts the result.
 * In order to avoid overflows completely the input signal must be scaled down by
 * log2(blockSize)-8 bits, as a total of blockSize additions are performed internally.
 * After division, internal variables should be Q18.46
 * Finally, the 18.46 accumulator is right shifted by 15 bits to yield a 1.31 format value.
 *
 */

void arm_std_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult)
{
  q63_t sum = 0;                                 /* Accumulator */
  q63_t meanOfSquares, squareOfMean;             /* square of mean and mean of square */
  q31_t in;                                      /* input value */
  uint32_t blkCnt;                               /* loop counter */
  q63_t sumOfSquares = 0;                        /* Accumulator */

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
    in = *pSrc++ >> 8U;
    sum += in;
    sumOfSquares += ((q63_t) (in) * (in));
    in = *pSrc++ >> 8U;
    sum += in;
    sumOfSquares += ((q63_t) (in) * (in));
    in = *pSrc++ >> 8U;
    sum += in;
    sumOfSquares += ((q63_t) (in) * (in));
    in = *pSrc++ >> 8U;
    sum += in;
    sumOfSquares += ((q63_t) (in) * (in));

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
    in = *pSrc++ >> 8U;
    sum += in;
    sumOfSquares += ((q63_t) (in) * (in));

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = sumOfSquares / (q63_t)(blockSize - 1U);

#else
  /* Run the below code for Cortex-M0 */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sumOfSquares. */
    in = *pSrc++ >> 8U;
    sumOfSquares += ((q63_t) (in) * (in));

    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
    /* Compute sum of all input values and then store the result in a temporary variable, sum. */
    sum += in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = sumOfSquares / (q63_t)(blockSize - 1U);

#endif /* #if defined (ARM_MATH_DSP) */

  /* Compute square of mean */
  squareOfMean = sum * sum / (q63_t)(blockSize * (blockSize - 1U));

  /* Compute standard deviation and then store the result to the destination */
  arm_sqrt_q31((meanOfSquares - squareOfMean) >> 15U, pResult);
}

/**
 * @} end of STD group
 */
