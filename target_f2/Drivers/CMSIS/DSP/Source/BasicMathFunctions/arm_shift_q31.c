/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_shift_q31.c
 * Description:  Shifts the elements of a Q31 vector by a specified number of bits
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
 * @defgroup shift Vector Shift
 *
 * Shifts the elements of a fixed-point vector by a specified number of bits.
 * There are separate functions for Q7, Q15, and Q31 data types.
 * The underlying algorithm used is:
 *
 * <pre>
 *     pDst[n] = pSrc[n] << shift,   0 <= n < blockSize.
 * </pre>
 *
 * If <code>shift</code> is positive then the elements of the vector are shifted to the left.
 * If <code>shift</code> is negative then the elements of the vector are shifted to the right.
 *
 * The functions support in-place computation allowing the source and destination
 * pointers to reference the same memory buffer.
 */

/**
 * @addtogroup shift
 * @{
 */

/**
 * @brief  Shifts the elements of a Q31 vector a specified number of bits.
 * @param[in]  *pSrc points to the input vector
 * @param[in]  shiftBits number of bits to shift.  A positive value shifts left; a negative value shifts right.
 * @param[out]  *pDst points to the output vector
 * @param[in]  blockSize number of samples in the vector
 * @return none.
 *
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q31 range [0x80000000 0x7FFFFFFF] will be saturated.
 */

void arm_shift_q31(
  q31_t * pSrc,
  int8_t shiftBits,
  q31_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign = (shiftBits & 0x80);             /* Sign of shiftBits */

#if defined (ARM_MATH_DSP)

  q31_t in1, in2, in3, in4;                      /* Temporary input variables */
  q31_t out1, out2, out3, out4;                  /* Temporary output variables */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;


  if (sign == 0U)
  {
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
      /* C = A  << shiftBits */
      /* Shift the input and then store the results in the destination buffer. */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      out1 = in1 << shiftBits;
      in3 = *(pSrc + 2);
      out2 = in2 << shiftBits;
      in4 = *(pSrc + 3);
      if (in1 != (out1 >> shiftBits))
        out1 = 0x7FFFFFFF ^ (in1 >> 31);

      if (in2 != (out2 >> shiftBits))
        out2 = 0x7FFFFFFF ^ (in2 >> 31);

      *pDst = out1;
      out3 = in3 << shiftBits;
      *(pDst + 1) = out2;
      out4 = in4 << shiftBits;

      if (in3 != (out3 >> shiftBits))
        out3 = 0x7FFFFFFF ^ (in3 >> 31);

      if (in4 != (out4 >> shiftBits))
        out4 = 0x7FFFFFFF ^ (in4 >> 31);

      *(pDst + 2) = out3;
      *(pDst + 3) = out4;

      /* Update destination pointer to process next sampels */
      pSrc += 4U;
      pDst += 4U;

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
      /* C = A >>  shiftBits */
      /* Shift the input and then store the results in the destination buffer. */
      in1 = *pSrc;
      in2 = *(pSrc + 1);
      in3 = *(pSrc + 2);
      in4 = *(pSrc + 3);

      *pDst = (in1 >> -shiftBits);
      *(pDst + 1) = (in2 >> -shiftBits);
      *(pDst + 2) = (in3 >> -shiftBits);
      *(pDst + 3) = (in4 >> -shiftBits);


      pSrc += 4U;
      pDst += 4U;

      blkCnt--;
    }

  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */


  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */


  while (blkCnt > 0U)
  {
    /* C = A (>> or <<) shiftBits */
    /* Shift the input and then store the result in the destination buffer. */
    *pDst++ = (sign == 0U) ? clip_q63_to_q31((q63_t) * pSrc++ << shiftBits) :
      (*pSrc++ >> -shiftBits);

    /* Decrement the loop counter */
    blkCnt--;
  }


}

/**
 * @} end of shift group
 */
