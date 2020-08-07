/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_abs_q15.c
 * Description:  Q15 vector absolute value
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
 * @ingroup groupMath
 */

/**
 * @addtogroup BasicAbs
 * @{
 */

/**
 * @brief Q15 vector absolute value.
 * @param[in]       *pSrc points to the input buffer
 * @param[out]      *pDst points to the output buffer
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * The Q15 value -1 (0x8000) will be saturated to the maximum allowable positive value 0x7FFF.
 */

void arm_abs_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)
  __SIMD32_TYPE *simd;

/* Run the below code for Cortex-M4 and Cortex-M3 */

  q15_t in1;                                     /* Input value1 */
  q15_t in2;                                     /* Input value2 */


  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  simd = __SIMD32_CONST(pDst);
  while (blkCnt > 0U)
  {
    /* C = |A| */
    /* Read two inputs */
    in1 = *pSrc++;
    in2 = *pSrc++;


    /* Store the Absolute result in the destination buffer by packing the two values, in a single cycle */
#ifndef  ARM_MATH_BIG_ENDIAN
    *simd++ =
      __PKHBT(((in1 > 0) ? in1 : (q15_t)__QSUB16(0, in1)),
              ((in2 > 0) ? in2 : (q15_t)__QSUB16(0, in2)), 16);

#else


    *simd++ =
      __PKHBT(((in2 > 0) ? in2 : (q15_t)__QSUB16(0, in2)),
              ((in1 > 0) ? in1 : (q15_t)__QSUB16(0, in1)), 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN    */

    in1 = *pSrc++;
    in2 = *pSrc++;


#ifndef  ARM_MATH_BIG_ENDIAN

    *simd++ =
      __PKHBT(((in1 > 0) ? in1 : (q15_t)__QSUB16(0, in1)),
              ((in2 > 0) ? in2 : (q15_t)__QSUB16(0, in2)), 16);

#else


    *simd++ =
      __PKHBT(((in2 > 0) ? in2 : (q15_t)__QSUB16(0, in2)),
              ((in1 > 0) ? in1 : (q15_t)__QSUB16(0, in1)), 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN    */

    /* Decrement the loop counter */
    blkCnt--;
  }
  pDst = (q15_t *)simd;

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = |A| */
    /* Read the input */
    in1 = *pSrc++;

    /* Calculate absolute value of input and then store the result in the destination buffer. */
    *pDst++ = (in1 > 0) ? in1 : (q15_t)__QSUB16(0, in1);

    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  /* Run the below code for Cortex-M0 */

  q15_t in;                                      /* Temporary input variable */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    /* C = |A| */
    /* Read the input */
    in = *pSrc++;

    /* Calculate absolute value of input and then store the result in the destination buffer. */
    *pDst++ = (in > 0) ? in : ((in == (q15_t) 0x8000) ? 0x7fff : -in);

    /* Decrement the loop counter */
    blkCnt--;
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of BasicAbs group
 */
