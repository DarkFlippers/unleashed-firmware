/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_mag_q31.c
 * Description:  Q31 complex magnitude
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
 * @addtogroup cmplx_mag
 * @{
 */

/**
 * @brief  Q31 complex magnitude
 * @param  *pSrc points to the complex input vector
 * @param  *pDst points to the real output vector
 * @param  numSamples number of complex samples in the input vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function implements 1.31 by 1.31 multiplications and finally output is converted into 2.30 format.
 * Input down scaling is not required.
 */

void arm_cmplx_mag_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t numSamples)
{
  q31_t real, imag;                              /* Temporary variables to hold input values */
  q31_t acc0, acc1;                              /* Accumulators */
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t real1, real2, imag1, imag2;              /* Temporary variables to hold input values */
  q31_t out1, out2, out3, out4;                  /* Accumulators */
  q63_t mul1, mul2, mul3, mul4;                  /* Temporary variables */


  /*loop Unrolling */
  blkCnt = numSamples >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* read complex input from source buffer */
    real1 = pSrc[0];
    imag1 = pSrc[1];
    real2 = pSrc[2];
    imag2 = pSrc[3];

    /* calculate power of input values */
    mul1 = (q63_t) real1 *real1;
    mul2 = (q63_t) imag1 *imag1;
    mul3 = (q63_t) real2 *real2;
    mul4 = (q63_t) imag2 *imag2;

    /* get the result to 3.29 format */
    out1 = (q31_t) (mul1 >> 33);
    out2 = (q31_t) (mul2 >> 33);
    out3 = (q31_t) (mul3 >> 33);
    out4 = (q31_t) (mul4 >> 33);

    /* add real and imaginary accumulators */
    out1 = out1 + out2;
    out3 = out3 + out4;

    /* read complex input from source buffer */
    real1 = pSrc[4];
    imag1 = pSrc[5];
    real2 = pSrc[6];
    imag2 = pSrc[7];

    /* calculate square root */
    arm_sqrt_q31(out1, &pDst[0]);

    /* calculate power of input values */
    mul1 = (q63_t) real1 *real1;

    /* calculate square root */
    arm_sqrt_q31(out3, &pDst[1]);

    /* calculate power of input values */
    mul2 = (q63_t) imag1 *imag1;
    mul3 = (q63_t) real2 *real2;
    mul4 = (q63_t) imag2 *imag2;

    /* get the result to 3.29 format */
    out1 = (q31_t) (mul1 >> 33);
    out2 = (q31_t) (mul2 >> 33);
    out3 = (q31_t) (mul3 >> 33);
    out4 = (q31_t) (mul4 >> 33);

    /* add real and imaginary accumulators */
    out1 = out1 + out2;
    out3 = out3 + out4;

    /* calculate square root */
    arm_sqrt_q31(out1, &pDst[2]);

    /* increment destination by 8 to process next samples */
    pSrc += 8U;

    /* calculate square root */
    arm_sqrt_q31(out3, &pDst[3]);

    /* increment destination by 4 to process next samples */
    pDst += 4U;

    /* Decrement the loop counter */
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
    /* C[0] = sqrt(A[0] * A[0] + A[1] * A[1]) */
    real = *pSrc++;
    imag = *pSrc++;
    acc0 = (q31_t) (((q63_t) real * real) >> 33);
    acc1 = (q31_t) (((q63_t) imag * imag) >> 33);
    /* store the result in 2.30 format in the destination buffer. */
    arm_sqrt_q31(acc0 + acc1, pDst++);

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of cmplx_mag group
 */
