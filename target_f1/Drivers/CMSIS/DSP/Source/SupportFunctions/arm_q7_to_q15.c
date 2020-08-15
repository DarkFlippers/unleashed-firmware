/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_q7_to_q15.c
 * Description:  Converts the elements of the Q7 vector to Q15 vector
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
 * @addtogroup q7_to_x
 * @{
 */




/**
 * @brief Converts the elements of the Q7 vector to Q15 vector.
 * @param[in]       *pSrc points to the Q7 input vector
 * @param[out]      *pDst points to the Q15 output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 * \par Description:
 *
 * The equation used for the conversion process is:
 *
 * <pre>
 * 	pDst[n] = (q15_t) pSrc[n] << 8;   0 <= n < blockSize.
 * </pre>
 *
 */


void arm_q7_to_q15(
  q7_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q7_t *pIn = pSrc;                              /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)
  q31_t in;
  q31_t in1, in2;
  q31_t out1, out2;

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (q15_t) A << 8 */
    /* convert from q7 to q15 and then store the results in the destination buffer */
    in = *__SIMD32(pIn)++;

    /* rotatate in by 8 and extend two q7_t values to q15_t values */
    in1 = __SXTB16(__ROR(in, 8));

    /* extend remainig two q7_t values to q15_t values */
    in2 = __SXTB16(in);

    in1 = in1 << 8U;
    in2 = in2 << 8U;

    in1 = in1 & 0xFF00FF00;
    in2 = in2 & 0xFF00FF00;

#ifndef ARM_MATH_BIG_ENDIAN

    out2 = __PKHTB(in1, in2, 16);
    out1 = __PKHBT(in2, in1, 16);

#else

    out1 = __PKHTB(in1, in2, 16);
    out2 = __PKHBT(in2, in1, 16);

#endif

    *__SIMD32(pDst)++ = out1;
    *__SIMD32(pDst)++ = out2;

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
    /* C = (q15_t) A << 8 */
    /* convert from q7 to q15 and then store the results in the destination buffer */
    *pDst++ = (q15_t) * pIn++ << 8;

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**
 * @} end of q7_to_x group
 */
