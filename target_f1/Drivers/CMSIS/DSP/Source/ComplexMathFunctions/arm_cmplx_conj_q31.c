/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_conj_q31.c
 * Description:  Q31 complex conjugate
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
 * @brief  Q31 complex conjugate.
 * @param  *pSrc points to the input vector
 * @param  *pDst points to the output vector
 * @param  numSamples number of complex samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * The Q31 value -1 (0x80000000) will be saturated to the maximum allowable positive value 0x7FFFFFFF.
 */

void arm_cmplx_conj_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t numSamples)
{
  uint32_t blkCnt;                               /* loop counter */
  q31_t in;                                      /* Input value */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t inR1, inR2, inR3, inR4;                  /* Temporary real variables */
  q31_t inI1, inI2, inI3, inI4;                  /* Temporary imaginary variables */

  /*loop Unrolling */
  blkCnt = numSamples >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C[0]+jC[1] = A[0]+ j (-1) A[1] */
    /* Calculate Complex Conjugate and then store the results in the destination buffer. */
    /* Saturated to 0x7fffffff if the input is -1(0x80000000) */
    /* read real input sample */
    inR1 = pSrc[0];
    /* store real input sample */
    pDst[0] = inR1;

    /* read imaginary input sample */
    inI1 = pSrc[1];

    /* read real input sample */
    inR2 = pSrc[2];
    /* store real input sample */
    pDst[2] = inR2;

    /* read imaginary input sample */
    inI2 = pSrc[3];

    /* negate imaginary input sample */
    inI1 = __QSUB(0, inI1);

    /* read real input sample */
    inR3 = pSrc[4];
    /* store real input sample */
    pDst[4] = inR3;

    /* read imaginary input sample */
    inI3 = pSrc[5];

    /* negate imaginary input sample */
    inI2 = __QSUB(0, inI2);

    /* read real input sample */
    inR4 = pSrc[6];
    /* store real input sample */
    pDst[6] = inR4;

    /* negate imaginary input sample */
    inI3 = __QSUB(0, inI3);

    /* store imaginary input sample */
    inI4 = pSrc[7];

    /* store imaginary input samples */
    pDst[1] = inI1;

    /* negate imaginary input sample */
    inI4 = __QSUB(0, inI4);

    /* store imaginary input samples */
    pDst[3] = inI2;

    /* increment source pointer by 8 to proecess next samples */
    pSrc += 8U;

    /* store imaginary input samples */
    pDst[5] = inI3;
    pDst[7] = inI4;

    /* increment destination pointer by 8 to process next samples */
    pDst += 8U;

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
    /* C[0]+jC[1] = A[0]+ j (-1) A[1] */
    /* Calculate Complex Conjugate and then store the results in the destination buffer. */
    /* Saturated to 0x7fffffff if the input is -1(0x80000000) */
    *pDst++ = *pSrc++;
    in = *pSrc++;
    *pDst++ = (in == INT32_MIN) ? INT32_MAX : -in;

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**
 * @} end of cmplx_conj group
 */
