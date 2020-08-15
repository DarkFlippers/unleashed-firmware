/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mult_q7.c
 * Description:  Q7 vector multiplication
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
 * @addtogroup BasicMult
 * @{
 */

/**
 * @brief           Q7 vector multiplication
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q7 range [0x80 0x7F] will be saturated.
 */

void arm_mult_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counters */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  q7_t out1, out2, out3, out4;                   /* Temporary variables to store the product */

  /* loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = A * B */
    /* Multiply the inputs and store the results in temporary variables */
    out1 = (q7_t) __SSAT((((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7), 8);
    out2 = (q7_t) __SSAT((((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7), 8);
    out3 = (q7_t) __SSAT((((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7), 8);
    out4 = (q7_t) __SSAT((((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7), 8);

    /* Store the results of 4 inputs in the destination buffer in single cycle by packing */
    *__SIMD32(pDst)++ = __PACKq7(out1, out2, out3, out4);

    /* Decrement the blockSize loop counter */
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
    /* C = A * B */
    /* Multiply the inputs and store the result in the destination buffer */
    *pDst++ = (q7_t) __SSAT((((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7), 8);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }
}

/**
 * @} end of BasicMult group
 */
