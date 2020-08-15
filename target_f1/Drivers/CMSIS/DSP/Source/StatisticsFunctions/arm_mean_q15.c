/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mean_q15.c
 * Description:  Mean value of a Q15 vector
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
 * @addtogroup mean
 * @{
 */


/**
 * @brief Mean value of a Q15 vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult mean value returned here
 * @return none.
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using a 32-bit internal accumulator.
 * The input is represented in 1.15 format and is accumulated in a 32-bit
 * accumulator in 17.15 format.
 * There is no risk of internal overflow with this approach, and the
 * full precision of intermediate result is preserved.
 * Finally, the accumulator is saturated and truncated to yield a result of 1.15 format.
 *
 */

void arm_mean_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult)
{
  q31_t sum = 0;                                 /* Temporary result storage */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t in;

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
    in = *__SIMD32(pSrc)++;
    sum += ((in << 16U) >> 16U);
    sum +=  (in >> 16U);
    in = *__SIMD32(pSrc)++;
    sum += ((in << 16U) >> 16U);
    sum +=  (in >> 16U);

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
    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
    sum += *pSrc++;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) / blockSize  */
  /* Store the result to the destination */
  *pResult = (q15_t) (sum / (q31_t)blockSize);
}

/**
 * @} end of mean group
 */
