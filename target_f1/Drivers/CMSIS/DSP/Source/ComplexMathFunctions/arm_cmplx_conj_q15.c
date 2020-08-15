/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_conj_q15.c
 * Description:  Q15 complex conjugate
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
 * @ingroup groupCmplxMath
 */

/**
 * @addtogroup cmplx_conj
 * @{
 */

/**
 * @brief  Q15 complex conjugate.
 * @param  *pSrc points to the input vector
 * @param  *pDst points to the output vector
 * @param  numSamples number of complex samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * The Q15 value -1 (0x8000) will be saturated to the maximum allowable positive value 0x7FFF.
 */

void arm_cmplx_conj_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  uint32_t blkCnt;                               /* loop counter */
  q31_t in1, in2, in3, in4;
  q31_t zero = 0;

  /*loop Unrolling */
  blkCnt = numSamples >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C[0]+jC[1] = A[0]+ j (-1) A[1] */
    /* Calculate Complex Conjugate and then store the results in the destination buffer. */
    in1 = *__SIMD32(pSrc)++;
    in2 = *__SIMD32(pSrc)++;
    in3 = *__SIMD32(pSrc)++;
    in4 = *__SIMD32(pSrc)++;

#ifndef ARM_MATH_BIG_ENDIAN

    in1 = __QASX(zero, in1);
    in2 = __QASX(zero, in2);
    in3 = __QASX(zero, in3);
    in4 = __QASX(zero, in4);

#else

    in1 = __QSAX(zero, in1);
    in2 = __QSAX(zero, in2);
    in3 = __QSAX(zero, in3);
    in4 = __QSAX(zero, in4);

#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

    in1 = ((uint32_t) in1 >> 16) | ((uint32_t) in1 << 16);
    in2 = ((uint32_t) in2 >> 16) | ((uint32_t) in2 << 16);
    in3 = ((uint32_t) in3 >> 16) | ((uint32_t) in3 << 16);
    in4 = ((uint32_t) in4 >> 16) | ((uint32_t) in4 << 16);

    *__SIMD32(pDst)++ = in1;
    *__SIMD32(pDst)++ = in2;
    *__SIMD32(pDst)++ = in3;
    *__SIMD32(pDst)++ = in4;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4U;

  while (blkCnt > 0U)
  {
    /* C[0]+jC[1] = A[0]+ j (-1) A[1] */
    /* Calculate Complex Conjugate and then store the results in the destination buffer. */
    *pDst++ = *pSrc++;
    *pDst++ = __SSAT(-*pSrc++, 16);

    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  q15_t in;

  /* Run the below code for Cortex-M0 */

  while (numSamples > 0U)
  {
    /* realOut + j (imagOut) = realIn+ j (-1) imagIn */
    /* Calculate Complex Conjugate and then store the results in the destination buffer. */
    *pDst++ = *pSrc++;
    in = *pSrc++;
    *pDst++ = (in == (q15_t) 0x8000) ? 0x7fff : -in;

    /* Decrement the loop counter */
    numSamples--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of cmplx_conj group
 */
