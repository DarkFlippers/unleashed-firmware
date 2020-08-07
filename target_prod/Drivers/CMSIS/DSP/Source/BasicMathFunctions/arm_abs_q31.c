/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_abs_q31.c
 * Description:  Q31 vector absolute value
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
 * @brief Q31 vector absolute value.
 * @param[in]       *pSrc points to the input buffer
 * @param[out]      *pDst points to the output buffer
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * The Q31 value -1 (0x80000000) will be saturated to the maximum allowable positive value 0x7FFFFFFF.
 */

void arm_abs_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q31_t in;                                      /* Input value */

#if defined (ARM_MATH_DSP)

  /* Run the below code for Cortex-M4 and Cortex-M3 */
  q31_t in1, in2, in3, in4;

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = |A| */
    /* Calculate absolute of input (if -1 then saturated to 0x7fffffff) and then store the results in the destination buffer. */
    in1 = *pSrc++;
    in2 = *pSrc++;
    in3 = *pSrc++;
    in4 = *pSrc++;

    *pDst++ = (in1 > 0) ? in1 : (q31_t)__QSUB(0, in1);
    *pDst++ = (in2 > 0) ? in2 : (q31_t)__QSUB(0, in2);
    *pDst++ = (in3 > 0) ? in3 : (q31_t)__QSUB(0, in3);
    *pDst++ = (in4 > 0) ? in4 : (q31_t)__QSUB(0, in4);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else

  /* Run the below code for Cortex-M0 */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /*   #if defined (ARM_MATH_DSP)   */

  while (blkCnt > 0U)
  {
    /* C = |A| */
    /* Calculate absolute value of the input (if -1 then saturated to 0x7fffffff) and then store the results in the destination buffer. */
    in = *pSrc++;
    *pDst++ = (in > 0) ? in : ((in == INT32_MIN) ? INT32_MAX : -in);

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**
 * @} end of BasicAbs group
 */
