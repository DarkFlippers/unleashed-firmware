/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_dot_prod_f32.c
 * Description:  Floating-point complex dot product
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
 * @defgroup cmplx_dot_prod Complex Dot Product
 *
 * Computes the dot product of two complex vectors.
 * The vectors are multiplied element-by-element and then summed.
 *
 * The <code>pSrcA</code> points to the first complex input vector and
 * <code>pSrcB</code> points to the second complex input vector.
 * <code>numSamples</code> specifies the number of complex samples
 * and the data in each array is stored in an interleaved fashion
 * (real, imag, real, imag, ...).
 * Each array has a total of <code>2*numSamples</code> values.
 *
 * The underlying algorithm is used:
 * <pre>
 * realResult=0;
 * imagResult=0;
 * for(n=0; n<numSamples; n++) {
 *     realResult += pSrcA[(2*n)+0]*pSrcB[(2*n)+0] - pSrcA[(2*n)+1]*pSrcB[(2*n)+1];
 *     imagResult += pSrcA[(2*n)+0]*pSrcB[(2*n)+1] + pSrcA[(2*n)+1]*pSrcB[(2*n)+0];
 * }
 * </pre>
 *
 * There are separate functions for floating-point, Q15, and Q31 data types.
 */

/**
 * @addtogroup cmplx_dot_prod
 * @{
 */

/**
 * @brief  Floating-point complex dot product
 * @param  *pSrcA points to the first input vector
 * @param  *pSrcB points to the second input vector
 * @param  numSamples number of complex samples in each vector
 * @param  *realResult real part of the result returned here
 * @param  *imagResult imaginary part of the result returned here
 * @return none.
 */

void arm_cmplx_dot_prod_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  uint32_t numSamples,
  float32_t * realResult,
  float32_t * imagResult)
{
  float32_t real_sum = 0.0f, imag_sum = 0.0f;    /* Temporary result storage */
  float32_t a0,b0,c0,d0;

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  uint32_t blkCnt;                               /* loop counter */

  /*loop Unrolling */
  blkCnt = numSamples >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      /* Decrement the loop counter */
      blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples & 0x3U;

  while (blkCnt > 0U)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      /* Decrement the loop counter */
      blkCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  while (numSamples > 0U)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      /* Decrement the loop counter */
      numSamples--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

  /* Store the real and imaginary results in the destination buffers */
  *realResult = real_sum;
  *imagResult = imag_sum;
}

/**
 * @} end of cmplx_dot_prod group
 */
