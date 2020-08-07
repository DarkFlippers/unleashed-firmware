/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mult_q15.c
 * Description:  Q15 vector multiplication
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
 * @brief           Q15 vector multiplication
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.
 */

void arm_mult_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counters */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t inA1, inA2, inB1, inB2;                  /* temporary input variables */
  q15_t out1, out2, out3, out4;                  /* temporary output variables */
  q31_t mul1, mul2, mul3, mul4;                  /* temporary variables */

  /* loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* read two samples at a time from sourceA */
    inA1 = *__SIMD32(pSrcA)++;
    /* read two samples at a time from sourceB */
    inB1 = *__SIMD32(pSrcB)++;
    /* read two samples at a time from sourceA */
    inA2 = *__SIMD32(pSrcA)++;
    /* read two samples at a time from sourceB */
    inB2 = *__SIMD32(pSrcB)++;

    /* multiply mul = sourceA * sourceB */
    mul1 = (q31_t) ((q15_t) (inA1 >> 16) * (q15_t) (inB1 >> 16));
    mul2 = (q31_t) ((q15_t) inA1 * (q15_t) inB1);
    mul3 = (q31_t) ((q15_t) (inA2 >> 16) * (q15_t) (inB2 >> 16));
    mul4 = (q31_t) ((q15_t) inA2 * (q15_t) inB2);

    /* saturate result to 16 bit */
    out1 = (q15_t) __SSAT(mul1 >> 15, 16);
    out2 = (q15_t) __SSAT(mul2 >> 15, 16);
    out3 = (q15_t) __SSAT(mul3 >> 15, 16);
    out4 = (q15_t) __SSAT(mul4 >> 15, 16);

    /* store the result */
#ifndef ARM_MATH_BIG_ENDIAN

    *__SIMD32(pDst)++ = __PKHBT(out2, out1, 16);
    *__SIMD32(pDst)++ = __PKHBT(out4, out3, 16);

#else

    *__SIMD32(pDst)++ = __PKHBT(out2, out1, 16);
    *__SIMD32(pDst)++ = __PKHBT(out4, out3, 16);

#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

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
    *pDst++ = (q15_t) __SSAT((((q31_t) (*pSrcA++) * (*pSrcB++)) >> 15), 16);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }
}

/**
 * @} end of BasicMult group
 */
