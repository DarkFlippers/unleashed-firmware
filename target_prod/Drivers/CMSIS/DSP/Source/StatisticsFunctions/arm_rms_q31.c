/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_rms_q31.c
 * Description:  Root Mean Square of the elements of a Q31 vector
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
 * @addtogroup RMS
 * @{
 */


/**
 * @brief Root Mean Square of the elements of a Q31 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult rms value returned here
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 *\par
 * The function is implemented using an internal 64-bit accumulator.
 * The input is represented in 1.31 format, and intermediate multiplication
 * yields a 2.62 format.
 * The accumulator maintains full precision of the intermediate multiplication results,
 * but provides only a single guard bit.
 * There is no saturation on intermediate additions.
 * If the accumulator overflows, it wraps around and distorts the result.
 * In order to avoid overflows completely, the input signal must be scaled down by
 * log2(blockSize) bits, as a total of blockSize additions are performed internally.
 * Finally, the 2.62 accumulator is right shifted by 31 bits to yield a 1.31 format value.
 *
 */

void arm_rms_q31(
  q31_t * pSrc,
  uint32_t blockSize,
  q31_t * pResult)
{
  q63_t sum = 0;                                 /* accumulator */
  q31_t in;                                      /* Temporary variable to store the input */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t in1, in2, in3, in4;                      /* Temporary input variables */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 8 outputs at a time.
   ** a second loop below computes the remaining 1 to 7 samples. */
  while (blkCnt > 0U)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute sum of the squares and then store the result in a temporary variable, sum */
    /* read two samples from source buffer */
    in1 = pSrc[0];
    in2 = pSrc[1];

    /* calculate power and accumulate to accumulator */
    sum += (q63_t) in1 *in1;
    sum += (q63_t) in2 *in2;

    /* read two samples from source buffer */
    in3 = pSrc[2];
    in4 = pSrc[3];

    /* calculate power and accumulate to accumulator */
    sum += (q63_t) in3 *in3;
    sum += (q63_t) in4 *in4;


    /* update source buffer to process next samples */
    pSrc += 4U;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 8, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else
  /* Run the below code for Cortex-M0 */

  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */

  while (blkCnt > 0U)
  {
    /* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
    /* Compute sum of the squares and then store the results in a temporary variable, sum */
    in = *pSrc++;
    sum += (q63_t) in *in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Convert data in 2.62 to 1.31 by 31 right shifts and saturate */
  /* Compute Rms and store the result in the destination vector */
  arm_sqrt_q31(clip_q63_to_q31((sum / (q63_t) blockSize) >> 31), pResult);
}

/**
 * @} end of RMS group
 */
