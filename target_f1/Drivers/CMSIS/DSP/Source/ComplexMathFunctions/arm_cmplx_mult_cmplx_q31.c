/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_mult_cmplx_q31.c
 * Description:  Q31 complex-by-complex multiplication
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
 * @addtogroup CmplxByCmplxMult
 * @{
 */


/**
 * @brief  Q31 complex-by-complex multiplication
 * @param[in]  *pSrcA points to the first input vector
 * @param[in]  *pSrcB points to the second input vector
 * @param[out]  *pDst  points to the output vector
 * @param[in]  numSamples number of complex samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function implements 1.31 by 1.31 multiplications and finally output is converted into 3.29 format.
 * Input down scaling is not required.
 */

void arm_cmplx_mult_cmplx_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  q31_t * pDst,
  uint32_t numSamples)
{
  q31_t a, b, c, d;                              /* Temporary variables to store real and imaginary values */
  uint32_t blkCnt;                               /* loop counters */
  q31_t mul1, mul2, mul3, mul4;
  q31_t out1, out2;

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /* loop Unrolling */
  blkCnt = numSamples >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x4U;

  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  /* loop Unrolling */
  blkCnt = numSamples >> 1U;

  /* First part of the processing with loop unrolling.  Compute 2 outputs at a time.
   ** a second loop below computes the remaining 1 sample. */
  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 2, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x2U;

  while (blkCnt > 0U)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of CmplxByCmplxMult group
 */
