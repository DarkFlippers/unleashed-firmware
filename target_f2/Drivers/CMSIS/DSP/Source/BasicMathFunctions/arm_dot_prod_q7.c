/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_dot_prod_q7.c
 * Description:  Q7 dot product
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
 * @brief Dot product of Q7 vectors.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[in]       blockSize number of samples in each vector
 * @param[out]      *result output result returned here
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The intermediate multiplications are in 1.7 x 1.7 = 2.14 format and these
 * results are added to an accumulator in 18.14 format.
 * Nonsaturating additions are used and there is no danger of wrap around as long as
 * the vectors are less than 2^18 elements long.
 * The return result is in 18.14 format.
 */

void arm_dot_prod_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  uint32_t blockSize,
  q31_t * result)
{
  uint32_t blkCnt;                               /* loop counter */

  q31_t sum = 0;                                 /* Temporary variables to store output */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */

  q31_t input1, input2;                          /* Temporary variables to store input */
  q31_t inA1, inA2, inB1, inB2;                  /* Temporary variables to store input */



  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* read 4 samples at a time from sourceA */
    input1 = *__SIMD32(pSrcA)++;
    /* read 4 samples at a time from sourceB */
    input2 = *__SIMD32(pSrcB)++;

    /* extract two q7_t samples to q15_t samples */
    inA1 = __SXTB16(__ROR(input1, 8));
    /* extract reminaing two samples */
    inA2 = __SXTB16(input1);
    /* extract two q7_t samples to q15_t samples */
    inB1 = __SXTB16(__ROR(input2, 8));
    /* extract reminaing two samples */
    inB2 = __SXTB16(input2);

    /* multiply and accumulate two samples at a time */
    sum = __SMLAD(inA1, inB1, sum);
    sum = __SMLAD(inA2, inB2, sum);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Dot product and then store the results in a temporary buffer. */
    sum = __SMLAD(*pSrcA++, *pSrcB++, sum);

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
    /* Dot product and then store the results in a temporary buffer. */
    sum += (q31_t) ((q15_t) * pSrcA++ * *pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */


  /* Store the result in the destination buffer in 18.14 format */
  *result = sum;
}

/**
 * @} end of dot_prod group
 */
