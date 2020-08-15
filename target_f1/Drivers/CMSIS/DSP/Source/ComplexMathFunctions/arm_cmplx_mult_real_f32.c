/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_mult_real_f32.c
 * Description:  Floating-point complex by real multiplication
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
 * @defgroup CmplxByRealMult Complex-by-Real Multiplication
 *
 * Multiplies a complex vector by a real vector and generates a complex result.
 * The data in the complex arrays is stored in an interleaved fashion
 * (real, imag, real, imag, ...).
 * The parameter <code>numSamples</code> represents the number of complex
 * samples processed.  The complex arrays have a total of <code>2*numSamples</code>
 * real values while the real array has a total of <code>numSamples</code>
 * real values.
 *
 * The underlying algorithm is used:
 *
 * <pre>
 * for(n=0; n<numSamples; n++) {
 *     pCmplxDst[(2*n)+0] = pSrcCmplx[(2*n)+0] * pSrcReal[n];
 *     pCmplxDst[(2*n)+1] = pSrcCmplx[(2*n)+1] * pSrcReal[n];
 * }
 * </pre>
 *
 * There are separate functions for floating-point, Q15, and Q31 data types.
 */

/**
 * @addtogroup CmplxByRealMult
 * @{
 */


/**
 * @brief  Floating-point complex-by-real multiplication
 * @param[in]  *pSrcCmplx points to the complex input vector
 * @param[in]  *pSrcReal points to the real input vector
 * @param[out]  *pCmplxDst points to the complex output vector
 * @param[in]  numSamples number of samples in each vector
 * @return none.
 */

void arm_cmplx_mult_real_f32(
  float32_t * pSrcCmplx,
  float32_t * pSrcReal,
  float32_t * pCmplxDst,
  uint32_t numSamples)
{
  float32_t in;                                  /* Temporary variable to store input value */
  uint32_t blkCnt;                               /* loop counters */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  float32_t inA1, inA2, inA3, inA4;              /* Temporary variables to hold input data */
  float32_t inA5, inA6, inA7, inA8;              /* Temporary variables to hold input data */
  float32_t inB1, inB2, inB3, inB4;              /* Temporary variables to hold input data */
  float32_t out1, out2, out3, out4;              /* Temporary variables to hold output data */
  float32_t out5, out6, out7, out8;              /* Temporary variables to hold output data */

  /* loop Unrolling */
  blkCnt = numSamples >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    /* read input from complex input buffer */
    inA1 = pSrcCmplx[0];
    inA2 = pSrcCmplx[1];
    /* read input from real input buffer */
    inB1 = pSrcReal[0];

    /* read input from complex input buffer */
    inA3 = pSrcCmplx[2];

    /* multiply complex buffer real input with real buffer input */
    out1 = inA1 * inB1;

    /* read input from complex input buffer */
    inA4 = pSrcCmplx[3];

    /* multiply complex buffer imaginary input with real buffer input */
    out2 = inA2 * inB1;

    /* read input from real input buffer */
    inB2 = pSrcReal[1];
    /* read input from complex input buffer */
    inA5 = pSrcCmplx[4];

    /* multiply complex buffer real input with real buffer input */
    out3 = inA3 * inB2;

    /* read input from complex input buffer */
    inA6 = pSrcCmplx[5];
    /* read input from real input buffer */
    inB3 = pSrcReal[2];

    /* multiply complex buffer imaginary input with real buffer input */
    out4 = inA4 * inB2;

    /* read input from complex input buffer */
    inA7 = pSrcCmplx[6];

    /* multiply complex buffer real input with real buffer input */
    out5 = inA5 * inB3;

    /* read input from complex input buffer */
    inA8 = pSrcCmplx[7];

    /* multiply complex buffer imaginary input with real buffer input */
    out6 = inA6 * inB3;

    /* read input from real input buffer */
    inB4 = pSrcReal[3];

    /* store result to destination bufer */
    pCmplxDst[0] = out1;

    /* multiply complex buffer real input with real buffer input */
    out7 = inA7 * inB4;

    /* store result to destination bufer */
    pCmplxDst[1] = out2;

    /* multiply complex buffer imaginary input with real buffer input */
    out8 = inA8 * inB4;

    /* store result to destination bufer */
    pCmplxDst[2] = out3;
    pCmplxDst[3] = out4;
    pCmplxDst[4] = out5;

    /* incremnet complex input buffer by 8 to process next samples */
    pSrcCmplx += 8U;

    /* store result to destination bufer */
    pCmplxDst[5] = out6;

    /* increment real input buffer by 4 to process next samples */
    pSrcReal += 4U;

    /* store result to destination bufer */
    pCmplxDst[6] = out7;
    pCmplxDst[7] = out8;

    /* increment destination buffer by 8 to process next sampels */
    pCmplxDst += 8U;

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4U;

#else

  /* Run the below code for Cortex-M0 */
  blkCnt = numSamples;

#endif /* #if defined (ARM_MATH_DSP) */

  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    in = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ = (*pSrcCmplx++) * (in);
    *pCmplxDst++ = (*pSrcCmplx++) * (in);

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }
}

/**
 * @} end of CmplxByRealMult group
 */
