/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_negate_q7.c
 * Description:  Negates Q7 vectors
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
 * @addtogroup negate
 * @{
 */

/**
 * @brief  Negates the elements of a Q7 vector.
 * @param[in]  *pSrc points to the input vector
 * @param[out]  *pDst points to the output vector
 * @param[in]  blockSize number of samples in the vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * The Q7 value -1 (0x80) will be saturated to the maximum allowable positive value 0x7F.
 */

void arm_negate_q7(
  q7_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q7_t in;

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t input;                                   /* Input values1-4 */
  q31_t zero = 0x00000000;


  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = -A */
    /* Read four inputs */
    input = *__SIMD32(pSrc)++;

    /* Store the Negated results in the destination buffer in a single cycle by packing the results */
    *__SIMD32(pDst)++ = __QSUB8(zero, input);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */

  while (blkCnt > 0U)
  {
    /* C = -A */
    /* Negate and then store the results in the destination buffer. */ \
      in = *pSrc++;
    *pDst++ = (in == (q7_t) 0x80) ? 0x7f : -in;

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of negate group
 */
