/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_shift_q7.c
 * Description:  Processing function for the Q7 Shifting
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
 * @brief  Shifts the elements of a Q7 vector a specified number of bits.
 * @param[in]  *pSrc points to the input vector
 * @param[in]  shiftBits number of bits to shift.  A positive value shifts left; a negative value shifts right.
 * @param[out]  *pDst points to the output vector
 * @param[in]  blockSize number of samples in the vector
 * @return none.
 *
 * \par Conditions for optimum performance
 *  Input and output buffers should be aligned by 32-bit
 *
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q7 range [0x8 0x7F] will be saturated.
 */

void arm_shift_q7(
  q7_t * pSrc,
  int8_t shiftBits,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign;                                  /* Sign of shiftBits */

#if defined (ARM_MATH_DSP)

/* Run the below code for Cortex-M4 and Cortex-M3 */
  q7_t in1;                                      /* Input value1 */
  q7_t in2;                                      /* Input value2 */
  q7_t in3;                                      /* Input value3 */
  q7_t in4;                                      /* Input value4 */


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
      /* C = A << shiftBits */
      /* Read 4 inputs */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      in3 = *(pSrc + 2);
      in4 = *(pSrc + 3);

      /* Store the Shifted result in the destination buffer in single cycle by packing the outputs */
      *__SIMD32(pDst)++ = __PACKq7(__SSAT((in1 << shiftBits), 8),
                                   __SSAT((in2 << shiftBits), 8),
                                   __SSAT((in3 << shiftBits), 8),
                                   __SSAT((in4 << shiftBits), 8));
      /* Update source pointer to process next sampels */
      pSrc += 4U;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (q7_t) __SSAT((*pSrc++ << shiftBits), 8);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    shiftBits = -shiftBits;
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* C = A >> shiftBits */
      /* Read 4 inputs */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      in3 = *(pSrc + 2);
      in4 = *(pSrc + 3);

      /* Store the Shifted result in the destination buffer in single cycle by packing the outputs */
      *__SIMD32(pDst)++ = __PACKq7((in1 >> shiftBits), (in2 >> shiftBits),
                                   (in3 >> shiftBits), (in4 >> shiftBits));


      pSrc += 4U;

      /* Decrement the loop counter */
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      in1 = *pSrc++;
      *pDst++ = (in1 >> shiftBits);

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
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (q7_t) __SSAT(((q15_t) * pSrc++ << shiftBits), 8);

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
      /* Shift the input and then store the result in the destination buffer. */
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
