/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_q15_to_q7.c
 * Description:  Converts the elements of the Q15 vector to Q7 vector
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
 * @ingroup groupSupport
 */

/**
 * @addtogroup q15_to_x
 * @{
 */


/**
 * @brief Converts the elements of the Q15 vector to Q7 vector.
 * @param[in]       *pSrc points to the Q15 input vector
 * @param[out]      *pDst points to the Q7 output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 * \par Description:
 *
 * The equation used for the conversion process is:
 *
 * <pre>
 * 	pDst[n] = (q7_t) pSrc[n] >> 8;   0 <= n < blockSize.
 * </pre>
 *
 */


void arm_q15_to_q7(
  q15_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  q15_t *pIn = pSrc;                             /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t in1, in2;
  q31_t out1, out2;

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (q7_t) A >> 8 */
    /* convert from q15 to q7 and then store the results in the destination buffer */
    in1 = *__SIMD32(pIn)++;
    in2 = *__SIMD32(pIn)++;

#ifndef ARM_MATH_BIG_ENDIAN

    out1 = __PKHTB(in2, in1, 16);
    out2 = __PKHBT(in2, in1, 16);

#else

    out1 = __PKHTB(in1, in2, 16);
    out2 = __PKHBT(in1, in2, 16);

#endif //      #ifndef ARM_MATH_BIG_ENDIAN

    /* rotate packed value by 24 */
    out2 = ((uint32_t) out2 << 8) | ((uint32_t) out2 >> 24);

    /* anding with 0xff00ff00 to get two 8 bit values */
    out1 = out1 & 0xFF00FF00;
    /* anding with 0x00ff00ff to get two 8 bit values */
    out2 = out2 & 0x00FF00FF;

    /* oring two values(contains two 8 bit values) to get four packed 8 bit values */
    out1 = out1 | out2;

    /* store 4 samples at a time to destiantion buffer */
    *__SIMD32(pDst)++ = out1;

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
    /* C = (q7_t) A >> 8 */
    /* convert from q15 to q7 and then store the results in the destination buffer */
    *pDst++ = (q7_t) (*pIn++ >> 8);

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**
 * @} end of q15_to_x group
 */
