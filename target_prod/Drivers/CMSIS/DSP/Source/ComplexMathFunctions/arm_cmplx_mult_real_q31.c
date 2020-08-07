/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_mult_real_q31.c
 * Description:  Q31 complex by real multiplication
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
 * @addtogroup CmplxByRealMult
 * @{
 */


/**
 * @brief  Q31 complex-by-real multiplication
 * @param[in]  *pSrcCmplx points to the complex input vector
 * @param[in]  *pSrcReal points to the real input vector
 * @param[out]  *pCmplxDst points to the complex output vector
 * @param[in]  numSamples number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q31 range[0x80000000 0x7FFFFFFF] will be saturated.
 */

void arm_cmplx_mult_real_q31(
  q31_t * pSrcCmplx,
  q31_t * pSrcReal,
  q31_t * pCmplxDst,
  uint32_t numSamples)
{
  q31_t inA1;                                    /* Temporary variable to store input value */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  uint32_t blkCnt;                               /* loop counters */
  q31_t inA2, inA3, inA4;                        /* Temporary variables to hold input data */
  q31_t inB1, inB2;                              /* Temporary variabels to hold input data */
  q31_t out1, out2, out3, out4;                  /* Temporary variables to hold output data */

  /* loop Unrolling */
  blkCnt = numSamples >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    /* read real input from complex input buffer */
    inA1 = *pSrcCmplx++;
    inA2 = *pSrcCmplx++;
    /* read input from real input bufer */
    inB1 = *pSrcReal++;
    inB2 = *pSrcReal++;
    /* read imaginary input from complex input buffer */
    inA3 = *pSrcCmplx++;
    inA4 = *pSrcCmplx++;

    /* multiply complex input with real input */
    out1 = ((q63_t) inA1 * inB1) >> 32;
    out2 = ((q63_t) inA2 * inB1) >> 32;
    out3 = ((q63_t) inA3 * inB2) >> 32;
    out4 = ((q63_t) inA4 * inB2) >> 32;

    /* sature the result */
    out1 = __SSAT(out1, 31);
    out2 = __SSAT(out2, 31);
    out3 = __SSAT(out3, 31);
    out4 = __SSAT(out4, 31);

    /* get result in 1.31 format */
    out1 = out1 << 1;
    out2 = out2 << 1;
    out3 = out3 << 1;
    out4 = out4 << 1;

    /* store the result to destination buffer */
    *pCmplxDst++ = out1;
    *pCmplxDst++ = out2;
    *pCmplxDst++ = out3;
    *pCmplxDst++ = out4;

    /* read real input from complex input buffer */
    inA1 = *pSrcCmplx++;
    inA2 = *pSrcCmplx++;
    /* read input from real input bufer */
    inB1 = *pSrcReal++;
    inB2 = *pSrcReal++;
    /* read imaginary input from complex input buffer */
    inA3 = *pSrcCmplx++;
    inA4 = *pSrcCmplx++;

    /* multiply complex input with real input */
    out1 = ((q63_t) inA1 * inB1) >> 32;
    out2 = ((q63_t) inA2 * inB1) >> 32;
    out3 = ((q63_t) inA3 * inB2) >> 32;
    out4 = ((q63_t) inA4 * inB2) >> 32;

    /* sature the result */
    out1 = __SSAT(out1, 31);
    out2 = __SSAT(out2, 31);
    out3 = __SSAT(out3, 31);
    out4 = __SSAT(out4, 31);

    /* get result in 1.31 format */
    out1 = out1 << 1;
    out2 = out2 << 1;
    out3 = out3 << 1;
    out4 = out4 << 1;

    /* store the result to destination buffer */
    *pCmplxDst++ = out1;
    *pCmplxDst++ = out2;
    *pCmplxDst++ = out3;
    *pCmplxDst++ = out4;

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4U;

  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    /* read real input from complex input buffer */
    inA1 = *pSrcCmplx++;
    inA2 = *pSrcCmplx++;
    /* read input from real input bufer */
    inB1 = *pSrcReal++;

    /* multiply complex input with real input */
    out1 = ((q63_t) inA1 * inB1) >> 32;
    out2 = ((q63_t) inA2 * inB1) >> 32;

    /* sature the result */
    out1 = __SSAT(out1, 31);
    out2 = __SSAT(out2, 31);

    /* get result in 1.31 format */
    out1 = out1 << 1;
    out2 = out2 << 1;

    /* store the result to destination buffer */
    *pCmplxDst++ = out1;
    *pCmplxDst++ = out2;

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  while (numSamples > 0U)
  {
    /* realOut = realA * realB.            */
    /* imagReal = imagA * realB.               */
    inA1 = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ =
      (q31_t) clip_q63_to_q31(((q63_t) * pSrcCmplx++ * inA1) >> 31);
    *pCmplxDst++ =
      (q31_t) clip_q63_to_q31(((q63_t) * pSrcCmplx++ * inA1) >> 31);

    /* Decrement the numSamples loop counter */
    numSamples--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of CmplxByRealMult group
 */
