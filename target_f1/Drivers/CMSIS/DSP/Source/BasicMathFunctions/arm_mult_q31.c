/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mult_q31.c
 * Description:  Q31 vector multiplication
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
 * @brief Q31 vector multiplication.
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q31 range[0x80000000 0x7FFFFFFF] will be saturated.
 */

void arm_mult_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  q31_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counters */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t inA1, inA2, inA3, inA4;                  /* temporary input variables */
  q31_t inB1, inB2, inB3, inB4;                  /* temporary input variables */
  q31_t out1, out2, out3, out4;                  /* temporary output variables */

  /* loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = A * B */
    /* Multiply the inputs and then store the results in the destination buffer. */
    inA1 = *pSrcA++;
    inA2 = *pSrcA++;
    inA3 = *pSrcA++;
    inA4 = *pSrcA++;
    inB1 = *pSrcB++;
    inB2 = *pSrcB++;
    inB3 = *pSrcB++;
    inB4 = *pSrcB++;

    out1 = ((q63_t) inA1 * inB1) >> 32;
    out2 = ((q63_t) inA2 * inB2) >> 32;
    out3 = ((q63_t) inA3 * inB3) >> 32;
    out4 = ((q63_t) inA4 * inB4) >> 32;

    out1 = __SSAT(out1, 31);
    out2 = __SSAT(out2, 31);
    out3 = __SSAT(out3, 31);
    out4 = __SSAT(out4, 31);

    *pDst++ = out1 << 1U;
    *pDst++ = out2 << 1U;
    *pDst++ = out3 << 1U;
    *pDst++ = out4 << 1U;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = A * B */
    /* Multiply the inputs and then store the results in the destination buffer. */
    inA1 = *pSrcA++;
    inB1 = *pSrcB++;
    out1 = ((q63_t) inA1 * inB1) >> 32;
    out1 = __SSAT(out1, 31);
    *pDst++ = out1 << 1U;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;


  while (blkCnt > 0U)
  {
    /* C = A * B */
    /* Multiply the inputs and then store the results in the destination buffer. */
    *pDst++ =
      (q31_t) clip_q63_to_q31(((q63_t) (*pSrcA++) * (*pSrcB++)) >> 31);

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */
}

/**
 * @} end of BasicMult group
 */
