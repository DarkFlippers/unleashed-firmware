/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_shift_q15.c
 * Description:  Shifts the elements of a Q15 vector by a specified number of bits
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
 * @addtogroup shift
 * @{
 */

/**
 * @brief  Shifts the elements of a Q15 vector a specified number of bits.
 * @param[in]  *pSrc points to the input vector
 * @param[in]  shiftBits number of bits to shift.  A positive value shifts left; a negative value shifts right.
 * @param[out]  *pDst points to the output vector
 * @param[in]  blockSize number of samples in the vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.
 */

void arm_shift_q15(
  q15_t * pSrc,
  int8_t shiftBits,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign;                                  /* Sign of shiftBits */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */

  q15_t in1, in2;                                /* Temporary variables */


  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

  /* If the shift value is positive then do right shift else left shift */
  if (sign == 0U)
  {
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* Read 2 inputs */
      in1 = *pSrc++;
      in2 = *pSrc++;
      /* C = A << shiftBits */
      /* Shift the inputs and then store the results in the destination buffer. */
#ifndef  ARM_MATH_BIG_ENDIAN

      *__SIMD32(pDst)++ = __PKHBT(__SSAT((in1 << shiftBits), 16),
                                  __SSAT((in2 << shiftBits), 16), 16);

#else

      *__SIMD32(pDst)++ = __PKHBT(__SSAT((in2 << shiftBits), 16),
                                  __SSAT((in1 << shiftBits), 16), 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN    */

      in1 = *pSrc++;
      in2 = *pSrc++;

#ifndef  ARM_MATH_BIG_ENDIAN

      *__SIMD32(pDst)++ = __PKHBT(__SSAT((in1 << shiftBits), 16),
                                  __SSAT((in2 << shiftBits), 16), 16);

#else

      *__SIMD32(pDst)++ = __PKHBT(__SSAT((in2 << shiftBits), 16),
                                  __SSAT((in1 << shiftBits), 16), 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN    */

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U)
    {
      /* C = A << shiftBits */
      /* Shift and then store the results in the destination buffer. */
      *pDst++ = __SSAT((*pSrc++ << shiftBits), 16);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* Read 2 inputs */
      in1 = *pSrc++;
      in2 = *pSrc++;

      /* C = A >> shiftBits */
      /* Shift the inputs and then store the results in the destination buffer. */
#ifndef  ARM_MATH_BIG_ENDIAN

      *__SIMD32(pDst)++ = __PKHBT((in1 >> -shiftBits),
                                  (in2 >> -shiftBits), 16);

#else

      *__SIMD32(pDst)++ = __PKHBT((in2 >> -shiftBits),
                                  (in1 >> -shiftBits), 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN    */

      in1 = *pSrc++;
      in2 = *pSrc++;

#ifndef  ARM_MATH_BIG_ENDIAN

      *__SIMD32(pDst)++ = __PKHBT((in1 >> -shiftBits),
                                  (in2 >> -shiftBits), 16);

#else

      *__SIMD32(pDst)++ = __PKHBT((in2 >> -shiftBits),
                                  (in1 >> -shiftBits), 16);

#endif /* #ifndef  ARM_MATH_BIG_ENDIAN    */

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U)
    {
      /* C = A >> shiftBits */
      /* Shift the inputs and then store the results in the destination buffer. */
      *pDst++ = (*pSrc++ >> -shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }

#else

  /* Run the below code for Cortex-M0 */

  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

  /* If the shift value is positive then do right shift else left shift */
  if (sign == 0U)
  {
    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
      /* C = A << shiftBits */
      /* Shift and then store the results in the destination buffer. */
      *pDst++ = __SSAT(((q31_t) * pSrc++ << shiftBits), 16);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
      /* C = A >> shiftBits */
      /* Shift the inputs and then store the results in the destination buffer. */
      *pDst++ = (*pSrc++ >> -shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }

#endif /* #if defined (ARM_MATH_DSP) */

}

/**
 * @} end of shift group
 */
