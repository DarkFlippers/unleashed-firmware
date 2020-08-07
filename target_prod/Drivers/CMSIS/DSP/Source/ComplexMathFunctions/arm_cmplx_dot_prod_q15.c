/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_dot_prod_q15.c
 * Description:  Processing function for the Q15 Complex Dot product
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
 * @addtogroup cmplx_dot_prod
 * @{
 */

/**
 * @brief  Q15 complex dot product
 * @param  *pSrcA points to the first input vector
 * @param  *pSrcB points to the second input vector
 * @param  numSamples number of complex samples in each vector
 * @param  *realResult real part of the result returned here
 * @param  *imagResult imaginary part of the result returned here
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function is implemented using an internal 64-bit accumulator.
 * The intermediate 1.15 by 1.15 multiplications are performed with full precision and yield a 2.30 result.
 * These are accumulated in a 64-bit accumulator with 34.30 precision.
 * As a final step, the accumulators are converted to 8.24 format.
 * The return results <code>realResult</code> and <code>imagResult</code> are in 8.24 format.
 */

void arm_cmplx_dot_prod_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  uint32_t numSamples,
  q31_t * realResult,
  q31_t * imagResult)
{
  q63_t real_sum = 0, imag_sum = 0;              /* Temporary result storage */
  q15_t a0,b0,c0,d0;

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

      real_sum += (q31_t)a0 * c0;
      imag_sum += (q31_t)a0 * d0;
      real_sum -= (q31_t)b0 * d0;
      imag_sum += (q31_t)b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += (q31_t)a0 * c0;
      imag_sum += (q31_t)a0 * d0;
      real_sum -= (q31_t)b0 * d0;
      imag_sum += (q31_t)b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += (q31_t)a0 * c0;
      imag_sum += (q31_t)a0 * d0;
      real_sum -= (q31_t)b0 * d0;
      imag_sum += (q31_t)b0 * c0;

      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += (q31_t)a0 * c0;
      imag_sum += (q31_t)a0 * d0;
      real_sum -= (q31_t)b0 * d0;
      imag_sum += (q31_t)b0 * c0;

      /* Decrement the loop counter */
      blkCnt--;
  }

  /* If the numSamples is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4U;

  while (blkCnt > 0U)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;

      real_sum += (q31_t)a0 * c0;
      imag_sum += (q31_t)a0 * d0;
      real_sum -= (q31_t)b0 * d0;
      imag_sum += (q31_t)b0 * c0;

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

  /* Store the real and imaginary results in 8.24 format  */
  /* Convert real data in 34.30 to 8.24 by 6 right shifts */
  *realResult = (q31_t) (real_sum >> 6);
  /* Convert imaginary data in 34.30 to 8.24 by 6 right shifts */
  *imagResult = (q31_t) (imag_sum >> 6);
}

/**
 * @} end of cmplx_dot_prod group
 */
