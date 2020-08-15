/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_dot_prod_q15.c
 * Description:  Q15 dot product
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
 * @ingroup groupMath
 */

/**
 * @addtogroup dot_prod
 * @{
 */

/**
 * @brief Dot product of Q15 vectors.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[in]       blockSize number of samples in each vector
 * @param[out]      *result output result returned here
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The intermediate multiplications are in 1.15 x 1.15 = 2.30 format and these
 * results are added to a 64-bit accumulator in 34.30 format.
 * Nonsaturating additions are used and given that there are 33 guard bits in the accumulator
 * there is no risk of overflow.
 * The return result is in 34.30 format.
 */

void arm_dot_prod_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  uint32_t blockSize,
  q63_t * result)
{
  q63_t sum = 0;                                 /* Temporary result storage */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */


  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer. */
    sum = __SMLALD(*__SIMD32(pSrcA)++, *__SIMD32(pSrcB)++, sum);
    sum = __SMLALD(*__SIMD32(pSrcA)++, *__SIMD32(pSrcB)++, sum);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the results in a temporary buffer. */
    sum = __SMLALD(*pSrcA++, *pSrcB++, sum);

    /* Decrement the loop counter */
    blkCnt--;
  }


#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the results in a temporary buffer. */
    sum += (q63_t) ((q31_t) * pSrcA++ * *pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

  /* Store the result in the destination buffer in 34.30 format */
  *result = sum;

}

/**
 * @} end of dot_prod group
 */
